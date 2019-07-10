#include "queryacsunify.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <vector>
#include <tuple>
#include <queue>
#include <functional>
#include "factories.h"
#include "variable.h"
#include "varterm.h"
#include "funterm.h"
#include "ldecompalg.h"
#include "ldelexalg.h"
#include "ldegraphalg.h"
#include "ldeslopesalg.h"

using namespace std;

QueryACSUnify::QueryACSUnify() {}
  
Query *QueryACSUnify::create() {
  return new QueryACSUnify();
}
  
void QueryACSUnify::parse(string &s, int &w) {
  matchString(s, w, "acs-unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
  t1->vars(); t2->vars();
}

void QueryACSUnify::execute() {
  Function *f = getFunction("f");
  Term *unityElement = getFunTerm(f->unityElement, {});
  function<void(Term*, map<Term*, int>&)> getCoefs = [&](Term *t, map<Term*, int> &M) {
    if(t->isVarTerm) {
      ++M[t];
      return;
    }
    for(auto term : t->getAsFunTerm()->arguments) {
      getCoefs(term, M);
    }
  };
  auto delSameCoefs = [&](map<Term*, int> &l, map<Term*, int> &r) {
    if (l.size() > r.size()) {
      l.swap(r);
    }
    vector<Term*> toDel;
    for (auto it : l) {
      if (r.count(it.first)) {
        toDel.push_back(it.first);
      }
    }
    for (Term *it : toDel) {
      int minVal = min(l[it], r[it]);
      l[it] -= minVal;
      r[it] -= minVal;
      if (!l[it]) {
        l.erase(it);
      }
      if (!r[it]) {
        r.erase(it);
      }
    }
  };
  auto fromMapToVector = [&](map<Term*, int> &M) {
    vector<int> ans;
    for (auto it : M) {
      ans.push_back(it.second);
    }
    return ans;
  };
  auto createFuncWithSameVar = [&](int cnt, Term *var) {
    if (!cnt) {
      return getFunTerm(getFunction("e"), {});
    }
    --cnt;
    if (!cnt) {
      return var;
    }
    Term *ans = var;
    while (cnt > 0) {
      --cnt;
      ans = getFunTerm(f, {ans, var});
    }
    return ans;
  };

  cout << "AC-Simple-Unifying " << t1->toString() << " and " << t2->toString() << endl;
  map<Term*, int> l, r;
  getCoefs(t1, l);
  getCoefs(t2, r);
  delSameCoefs(l, r);
  if (l.size() + r.size() == 0) {
    cout << Substitution().toString() << endl;
    return;
  }
  vector<int> a = fromMapToVector(l);
  vector<int> b = fromMapToVector(r);
  LDEGraphAlg graphAlg(a, b);
  vector<pair<vector<int>, vector<int>>> result = graphAlg.solve();
  if (!result.size()) {
    cout << "No unification" << endl;
    return;
  }

  vector<Substitution> sigma;
  for (const auto &sol : result) {
    int index = 0;
    sigma.push_back(Substitution());
    Term *z = getVarTerm(createFreshVariable((Sort*) getSort("State")));
    for (auto it : l) {
      sigma.back().add(it.first->getAsVarTerm()->variable, createFuncWithSameVar(sol.first[index], z));
      ++index;
    }
    index = 0;
    for (auto it : r) {
      sigma.back().add(it.first->getAsVarTerm()->variable, createFuncWithSameVar(sol.second[index], z));
      ++index;
    }
  }

  auto checkMask = [&](const vector<bool> &mask) -> bool {
    for (auto it : l) {
      bool flag = false;
      for (int i = 0; i < (int)sigma.size(); ++i) {
        if (mask[i]) {
          continue;
        }
        Term *aux = sigma[i].image(it.first->getAsVarTerm()->variable);
        if (aux->isVarTerm || aux != unityElement) {
          flag = true;
          break;
        }
      }
      if(!flag) {
        return false;
      }
    }
    for (auto it : r) {
      bool flag = false;
      for (int i = 0; i < (int)sigma.size(); ++i) {
        if (mask[i]) {
          continue;
        }
        Term *aux = sigma[i].image(it.first->getAsVarTerm()->variable);
        if (aux->isVarTerm || aux != unityElement) {
          flag = true;
          break;
        }
      }
      if (!flag) {
        return false;
      }
    }
    return true;
  };
  auto getSubstFromMask = [&](const vector<bool> &mask, Substitution &subst) {
    for (auto it : l) {
      Term *ans = nullptr;
      for (int i = 0; i < (int)sigma.size(); ++i) {
        if (mask[i]) {
          continue;
        }
        Term *aux = sigma[i].image(it.first->getAsVarTerm()->variable);
        if (aux->isVarTerm || aux != unityElement) {
          ans = ans ? getFunTerm(f, {ans, aux}) : aux;
        }
      }
      subst.add(it.first->getAsVarTerm()->variable, ans);
    }
    for (auto it : r) {
      Term *ans = nullptr;
      for (int i = 0; i < (int)sigma.size(); ++i) {
        if (mask[i]) {
          continue;
        }
        Term *aux = sigma[i].image(it.first->getAsVarTerm()->variable);
        if (aux->isVarTerm || aux != unityElement) {
          ans = ans ? getFunTerm(f, {ans, aux}) : aux;
        }
      }
      subst.add(it.first->getAsVarTerm()->variable, ans);
    }
  };

  vector<Substitution> minSubstSet;
  minSubstSet.push_back(Substitution());
  vector<bool> initMask(result.size());
  if (!checkMask(initMask)) {
    cout << "No unification" << endl;
    return;
  } else {
    getSubstFromMask(initMask, minSubstSet.back());
  }

  queue<vector<bool>> q;
  for (q.push(initMask); !q.empty(); q.pop()) {
    vector<bool> &now = q.front();
    vector<bool> nextMask = now;
    for (int i = 0; i < (int)sigma.size(); ++i) {
      if (now[i]) {
        break;
      }
      nextMask[i] = true;
      if (checkMask(nextMask)) {
        Substitution subst;
        getSubstFromMask(nextMask, subst);
        minSubstSet.push_back(subst);
        q.push(nextMask);
      }
      nextMask[i] = false;
    }
  }

  for (auto &subst : minSubstSet) {
    cout << subst.toString() << endl;
  }
}
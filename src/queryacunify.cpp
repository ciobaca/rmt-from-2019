#include <iostream>
#include <queue>
#include "queryacunify.h"
#include "parse.h"
#include "varterm.h"
#include "funterm.h"
#include "factories.h"
#include "unifeq.h"
#include "unifeqsystem.h"
#include "ldegraphalg.h"

using namespace std;

QueryACUnify::QueryACUnify() {}
  
Query *QueryACUnify::create() {
  return new QueryACUnify();
}
  
void QueryACUnify::parse(string &s, int &w) {
  matchString(s, w, "ac-unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
  t1->vars(); t2->vars();
  if (t1->isFunTerm && t2->isVarTerm) {
    swap(t1, t2);
  }
}

void QueryACUnify::delSameCoeffs(map<Term*, int> &l, map<Term*, int> &r) {
  if (l.size() > r.size()) {
    l.swap(r);
  }
  vector<Term*> toDel;
  for (auto it : l) {
    if (r.count(it.first)) {
      toDel.push_back(it.first);
    }
  }
  for (auto it : toDel) {
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
}

vector<int> QueryACUnify::fromMapToVector(const map<Term*, int> &M) {
  vector<int> ans;
  ans.reserve(M.size());
  for (auto it : M) {
    ans.push_back(it.second);
  }
  return ans;
}

Term* QueryACUnify::createFuncWithSameVar(int cnt, Term *var, Function *f, Term *unityElement) {
  if (!cnt) {
    return unityElement;
  }
  Term *ans = var;
  while (cnt > 1) {
    ans = getFunTerm(f, {ans, var});
    --cnt;
  }
  return ans;
}

vector<Substitution> QueryACUnify::solveAC(UnifEq ueq) {
  Function *f = ueq.t1->getAsFunTerm()->function;
  Term *unityElement = getFunTerm(f->unityElement, {});
  function<void(Term*, map<Term*, int>&, map<Term*, Term*>&)> getCoeffs;
  getCoeffs = [&](Term *t, map<Term*, int> &M, map<Term*, Term*> &constToVar) {
    if(t->isVarTerm) {
      ++M[t];
      return;
    }
    FunTerm *funTerm = t->getAsFunTerm();
    if(!funTerm->arguments.size() || funTerm->function != f) {
      if(!constToVar.count(funTerm)) {
        constToVar[funTerm] = getVarTerm(createFreshVariable((Sort*) getSort("State")));
      }
      ++M[constToVar[funTerm]];
      return;
    }
    for(auto term : funTerm->arguments) {
      getCoeffs(term, M, constToVar);
    }
  };
  map<Term*, int> l, r;
  map<Term*, Term*> constToVar;
  getCoeffs(ueq.t1, l, constToVar);
  getCoeffs(ueq.t2, r, constToVar);
  delSameCoeffs(l, r);
  if (!l.size() || !r.size()) {
    if (l.size() != r.size()) {
      if (f->unityElement == nullptr) {
        return {};
      }
      Substitution subst;
      for (auto it : l) {
        subst.add(it.first->getAsVarTerm()->variable, unityElement);
      }
      for (auto it : r) {
        subst.add(it.first->getAsVarTerm()->variable, unityElement);
      }
      return vector<Substitution>{subst};
    }
    return vector<Substitution>{Substitution()};
  }
  vector<int> a = fromMapToVector(l);
  vector<int> b = fromMapToVector(r);
  LDEGraphAlg graphAlg(a, b);
  vector<pair<vector<int>, vector<int>>> result = graphAlg.solve();
  if (!result.size()) {
    return {};
  }
  vector<Substitution> sigma;
  for (const auto &sol : result) {
    int index = 0;
    sigma.push_back(Substitution());
    Term *z = getVarTerm(createFreshVariable((Sort*) getSort("State")));
    for (auto it : l) {
      sigma.back().add(it.first->getAsVarTerm()->variable, createFuncWithSameVar(sol.first[index], z,f, unityElement));
      ++index;
    }
    index = 0;
    for (auto it : r) {
      sigma.back().add(it.first->getAsVarTerm()->variable, createFuncWithSameVar(sol.second[index], z, f, unityElement));
      ++index;
    }
  }
  vector<vector<Term*>> sigmaImage(sigma.size(), vector<Term*>(l.size() + r.size()));
  for(int i = 0; i < (int)sigma.size(); ++i) {
    int index = 0;
    for(auto it : l) {
      sigmaImage[i][index] = sigma[i].image(it.first->getAsVarTerm()->variable);
      ++index;
    }
    for(auto it : r) {
      sigmaImage[i][index] = sigma[i].image(it.first->getAsVarTerm()->variable);
      ++index;
    }
  }
  auto checkConstConstraints = [&](Substitution &subst) -> bool {
    map<Term*, Term*> constSubst;
    for (const auto &it : constToVar) {
      Variable *var = it.second->getAsVarTerm()->variable;
      if (subst.inDomain(var)) {
        Term *t = subst.image(var);
        if(t->isFunTerm && t != it.second) {
          return false;
        }
        if(t->isVarTerm) {
          if(constSubst.count(t) && constSubst[t] != it.first) {
            return false;
          }
          constSubst[t] = it.first;
        }
      }
    }
    for (const auto &it : constSubst) {
      subst.force(it.first->getAsVarTerm()->variable, it.second);
    }
    return true;
  };
  auto checkMask = [&](int mask) -> bool {
    int n = sigma.size();
    int m = sigmaImage[0].size();
    int used = 0;
    const int allBits = (1 << m) - 1;
    for (int i = 0; i < n; ++i) {
      if (mask & (1 << i)) {
        continue;
      }
      for (int j = 0; j < m; ++j) {
        if (used & (1 << j)) {
          continue;
        }
        Term *aux = sigmaImage[i][j];
        if (aux->isVarTerm || aux != unityElement) {
          used |= 1 << j;
          if (used == allBits) {
            return true;
          }
        }
      }
    }
    return false;
  };
  auto getSubstFromMask = [&](const int &mask, Substitution &subst) -> bool {
    int n = sigma.size();
    int m = sigmaImage[0].size();
    vector<Term*> ans(m, nullptr);
    for (int i = 0; i < n; ++i) {
      if (mask & (1 << i)) {
        continue;
      }
      for (int j = 0; j < m; ++j) {
        Term *aux = sigmaImage[i][j];
        if (aux->isVarTerm || aux != unityElement) {
          ans[j] = ans[j] ? getFunTerm(f, {ans[j], aux}) : aux;
        }
      }
    }
    if (f->unityElement != nullptr) {
      for (auto &it : ans) {
        if (it == nullptr) {
          it = unityElement;
        }
      }
    }
    int index = 0;
    for (auto it : l) {
      subst.add(it.first->getAsVarTerm()->variable, ans[index]);
      ++index;
    }
    for (auto it : r) {
      subst.add(it.first->getAsVarTerm()->variable, ans[index]);
      ++index;
    }
    return checkConstConstraints(subst);
  };
  vector<Substitution> minSubstSet;
  const int upperMask = 1 << sigma.size();
  for (int mask = 0; mask < upperMask; ++mask) {
    if (f->unityElement != nullptr || checkMask(mask)) {
      Substitution subst;
      if (getSubstFromMask(mask, subst)) {
        minSubstSet.push_back(subst);
      }
    }
  }
  if (!minSubstSet.size()) {
    return {};
  }
  map<Term*, Term*> invConstToVar;
  for(const auto &it : constToVar) {
    invConstToVar[it.second] = it.first;
  }
  for (auto &subst : minSubstSet) {
    Substitution normalizedSubst;
    for (const auto &it : subst) {
      Term *aux = getVarTerm(it.first);
      if (!invConstToVar.count(aux) && l.count(aux) + r.count(aux) > 0) {
        normalizedSubst.force(it.first, it.second);
      }
    }
    subst = move(normalizedSubst);
  }
  return minSubstSet;
}

vector<Substitution> QueryACUnify::solve(UnifEqSystem ues) {
  vector<Substitution> minSubstSet;
  queue<pair<UnifEqSystem, Substitution>> q;
  for (q.push(make_pair(ues, Substitution())); !q.empty(); q.pop()) {
    ues = move(q.front().first);
    Substitution subst = move(q.front().second);
    bool toAdd = true;
    while (ues.size() && toAdd) {
      UnifEq eq = ues.back();
      if (eq.t1 == eq.t2) {
        ues.pop_back();
        continue;
      }
      if (eq.t1->isVarTerm) {
        if (eq.t2->isFunTerm && eq.t2->hasVariable(eq.t1->getAsVarTerm()->variable)) {
          Term *uElem = getFunTerm(eq.t2->getAsFunTerm()->function->unityElement, {});
          if (uElem != nullptr) {
            ues.pop_back();
            Term *arg1 = eq.t2->getAsFunTerm()->arguments[0];
            Term *arg2 = eq.t2->getAsFunTerm()->arguments[1];
            UnifEqSystem nues(ues);
            nues.addEq(UnifEq(arg1, uElem));
            nues.addEq(UnifEq(arg2, eq.t1), true);
            q.push(make_pair(nues, subst));
            if (arg1 != arg2) {
              nues = UnifEqSystem(ues);
              nues.addEq(UnifEq(arg1, eq.t1));
              nues.addEq(UnifEq(arg2, uElem), true);
              q.push(make_pair(nues, subst));
            }
          }
          toAdd = false;
          break;
        }
        subst.force(eq.t1->getAsVarTerm()->variable, eq.t2);
        Substitution aux(eq.t1->getAsVarTerm()->variable, eq.t2);
        ues.pop_back();
        for (auto &it : ues) {
          it.t1 = it.t1->substitute(aux);
          it.t2 = it.t2->substitute(aux);
        }
        continue;
      }
      if (eq.t1->isFunTerm && eq.t2->isFunTerm) {
        auto func1 = eq.t1->getAsFunTerm()->function;
        auto func2 = eq.t2->getAsFunTerm()->function;
        if (func1 != func2) {
          ues.pop_back();
          if (func1->unityElement != nullptr) {
            Term *uElem = getFunTerm(func1->unityElement, {});
            Term *arg1 = eq.t1->getAsFunTerm()->arguments[0];
            Term *arg2 = eq.t1->getAsFunTerm()->arguments[1];
            UnifEqSystem nues(ues);
            nues.addEq(UnifEq(arg1, uElem));
            nues.addEq(UnifEq(arg2, eq.t2), true);
            q.push(make_pair(nues, subst));
            nues = UnifEqSystem(ues);
            nues.addEq(UnifEq(arg1, eq.t2));
            nues.addEq(UnifEq(arg2, uElem), true);
            q.push(make_pair(nues, subst));
          }
          if (func2->unityElement != nullptr) {
            Term *uElem = getFunTerm(func2->unityElement, {});
            Term *arg1 = eq.t2->getAsFunTerm()->arguments[0];
            Term *arg2 = eq.t2->getAsFunTerm()->arguments[1];
            UnifEqSystem nues(ues);
            nues.addEq(UnifEq(arg1, uElem));
            nues.addEq(UnifEq(arg2, eq.t1), true);
            q.push(make_pair(nues, subst));
            nues = UnifEqSystem(ues);
            nues.addEq(UnifEq(arg1, eq.t1));
            nues.addEq(UnifEq(arg2, uElem), true);
            q.push(make_pair(nues, subst));
          }
          toAdd = false;
          break;
        }
        if (func1->isAssociative && func1->isCommutative) {
          vector<Substitution> sols = solveAC(eq);
          ues.pop_back();
          for (auto &sol : sols) {
            q.push(make_pair(UnifEqSystem(sol, ues), subst));
          }
          toAdd = false;
          break;
        }
        UnifEq toDecomp = move(eq);
        ues.pop_back();
        ues.decomp(toDecomp.t1->getAsFunTerm(), toDecomp.t2->getAsFunTerm());
      }
    }
    if (toAdd) {
      minSubstSet.push_back(move(subst));
    }
  }
  return minSubstSet;
}

void QueryACUnify::execute() {
  cout << "AC-Unifying " << t1->toString() << " and " << t2->toString() << endl;
  vector<Substitution> minSubstSet = solve(UnifEqSystem(t1, t2));
  if (!minSubstSet.size()) {
    cout << "No unification." << endl;
    return;
  }
  for (auto &subst : minSubstSet) {
    cout << subst.toString() << endl;
  }
}
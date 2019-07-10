#include <string>
#include <map>
#include <iostream>
#include "queryunifypatweg.h"
#include "parse.h"
#include "factories.h"
#include "factories.h"
#include "triangularsubstitution.h"
#include "funterm.h"
#include "varterm.h"

using namespace std;

QueryUnifyPatWeg::QueryUnifyPatWeg() {
}
  
Query *QueryUnifyPatWeg::create() {
  return new QueryUnifyPatWeg();
}
  
void QueryUnifyPatWeg::parse(string &s, int &w) {
  matchString(s, w, "pat-weg-unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryUnifyPatWeg::execute() {
  cout << "Unifying " << t1->toString() << " and " << t2->toString() << endl;
  getParents(t1, nullptr);
  getParents(t2, nullptr);
  sort(funcNodes.begin(), funcNodes.end());
  sort(varNodes.begin(), varNodes.end());
  funcNodes.erase(unique(funcNodes.begin(), funcNodes.end()), funcNodes.end());
  varNodes.erase(unique(varNodes.begin(), varNodes.end()), varNodes.end());
  links[t1].push_back(t2);
  links[t2].push_back(t1);
  for (const auto &it : funcNodes) {
    if (!finish(it)) {
      cout << "No unification." << endl;
      return;
    }
  }
  for (const auto &it : varNodes) {
    if (!finish(it)) {
      cout << "No unification." << endl;
      return;
    }
  }
  for (const auto &it : sigma) {
    if (it.second->hasVariable(it.first->getAsVarTerm()->variable)) {
      cout << "No unification." << endl;
      return;
    }
  }
  for (const auto &it : sigma) {   
    Term *t = exploreVar(it.first);
    if (t != it.first) {
      subst.add(it.first->getAsVarTerm()->variable, t);
    }
  }
  if (t1->substitute(subst) != t2->substitute(subst)) {
    cout << "No unification." << endl;
  } else {
    cout << "Substitution: " << subst.toString() << endl;
  }
}

Term* QueryUnifyPatWeg::exploreVar(Term *s) {
  if (ready.find(s) != ready.end()) {
    return ready[s];
  }
  if (sigma.find(s) == sigma.end()) {
    return ready[s] = s;
  }
  auto aux = descend(sigma[s]);
  return ready[s] = move(aux);
}

Term* QueryUnifyPatWeg::descend(Term *s) {
  if (ready.find(s) != ready.end()) {
    return ready[s];
  }
  if (!s || (s->isFunTerm && !s->getAsFunTerm()->arguments.size())) {
    return ready[s] = s;
  }
  if (s->isVarTerm) {
    auto aux = exploreVar(s);
    return ready[s] = move(aux);
  }
  auto aux = getFunTerm(s->getAsFunTerm()->function, exploreArgs(s->getAsFunTerm()->arguments));
  return ready[s] = move(aux);
}

vector<Term*> QueryUnifyPatWeg::exploreArgs(vector<Term*> v) {
  for (auto &it : v) {
    it = descend(it);
  }
  return v;
}

bool QueryUnifyPatWeg::finish(Term *r) {
  if (complete.find(r) != complete.end()) {
    return true;
  }
  if (ptr.find(r) != ptr.end()) {
    return false;
  }
  ptr[r] = r;
  vector<Term*> stk;
  for (stk.push_back(r); stk.size();) {
    Term *s = stk.back();
    stk.pop_back();
    if (s->isFunTerm && r->isFunTerm && s->getAsFunTerm()->function != r->getAsFunTerm()->function) {
      return false;
    }
    for (const auto &it : parents[s]) {
      if (!finish(it)) {
        return false;
      }
    }
    for (const auto &it : links[s]) {
      if (it == r || complete.find(it) != complete.end()) {
        continue;
      }
      if (ptr.find(it) == ptr.end()) {
        ptr[it] = r;
        stk.push_back(it);
      } else if (ptr[it] != r) {
        return false;
      }
    }
    if (s != r) {
      if (s->isVarTerm) {
        sigma[s] = r;
      } else {
        FunTerm *fr = r->getAsFunTerm();
        FunTerm *fs = s->getAsFunTerm();
        int n = fr->arguments.size();
        if ((int)fs->arguments.size() != n) {
          return false;
        }
        for (int i = 0; i < n; ++i) {
          links[fr->arguments[i]].push_back(fs->arguments[i]);
          links[fs->arguments[i]].push_back(fr->arguments[i]);
        }
      }
    }
    complete.insert(s);
  }
  return true;
}

void QueryUnifyPatWeg::getParents(Term *s, Term *parent) {
  if (parent) {
    parents[s].push_back(parent);
  }
  (s->isVarTerm ? varNodes : funcNodes).push_back(s);
  if (s->isFunTerm) {
    for (const auto &it : s->getAsFunTerm()->arguments) {
      getParents(it, s);
    }
  }
}
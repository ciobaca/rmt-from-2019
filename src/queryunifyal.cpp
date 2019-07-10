#include <string>
#include <map>
#include <iostream>
#include "queryunifyal.h"
#include "parse.h"
#include "factories.h"
#include "factories.h"
#include "triangularsubstitution.h"
#include "funterm.h"
#include "varterm.h"

using namespace std;

QueryUnifyAL::QueryUnifyAL() {
}
  
Query *QueryUnifyAL::create() {
  return new QueryUnifyAL();
}
  
void QueryUnifyAL::parse(string &s, int &w) {
  matchString(s, w, "n*unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryUnifyAL::execute() {
  cout << "Unifying " << t1->toString() << " and " << t2->toString() << endl;
  if (unifClosure(t1, t2) && findSolution(t1)) {
    reverse(subst.begin(), subst.end());
    Substitution sbst = subst.getSubstitution();
    if (t1->substitute(sbst) == t2->substitute(sbst)) {
      cout << "Substitution: " << sbst.toString() << endl;
    } else {
      cout << "No unification." << endl;
    }
  } else {
    cout << "No unification." << endl;
  }
}

Term* QueryUnifyAL::getSchemaTerm(Term* s) {
  auto it = schemaTerm.find(s);
  if (it == schemaTerm.end()) {
    return schemaTerm[s] = s;
  }
  return it->second;
}

Term* QueryUnifyAL::getEqClass(Term* s) {
  auto it = eqClass.find(s);
  if (it == eqClass.end()) {
    return eqClass[s] = s;
  }
  return it->second;
}

vector<Variable*> QueryUnifyAL::getVars(Term* s) {
  auto it = vars.find(s);
  if (it == vars.end()) {
    if (s->isVarTerm) {
      vars[s] = {s->getAsVarTerm()->variable};
    }
    return vars[s];
  }
  return it->second;
}

int QueryUnifyAL::getSize(Term* s) {
  auto it = sz.find(s);
  if (it == sz.end()) {
    return sz[s] = 1;
  }
  return it->second;
}

Term* QueryUnifyAL::findClass(Term *s) {
  return getEqClass(s) == s ? s : eqClass[s] = findClass(getEqClass(s));
}

void QueryUnifyAL::joinClass(Term *s, Term *t) {
  s = findClass(s);
  t = findClass(t);
  if (getSize(s) < getSize(t)) {
    swap(s, t);
  }
  if (getSchemaTerm(getEqClass(s))->isVarTerm) {
    schemaTerm[getEqClass(s)] = getSchemaTerm(getEqClass(t));
  }
  sz[s] += getSize(t);
  vector<Variable*> vs = getVars(s);
  vector<Variable*> ts = getVars(t);
  vs.insert(vs.end(), ts.begin(), ts.end());
  sort(vs.begin(), vs.end());
  vs.erase(unique(vs.begin(), vs.end()), vs.end());
  vars[s] = vs;
  eqClass[t] = s;
}

bool QueryUnifyAL::unifClosure(Term *s, Term *t) {
  s = findClass(s);
  t = findClass(t);
  if (s == t) {
    return true;
  }
  Term *l = getSchemaTerm(getEqClass(s));
  Term *r = getSchemaTerm(getEqClass(t));
  if (l->isFunTerm && r->isFunTerm) {
    FunTerm *fs = l->getAsFunTerm();
    FunTerm *ft = r->getAsFunTerm();
    if (fs->function != ft->function) {
      return false;
    }
    joinClass(s, t);
    int n = fs->arguments.size();
    for (int i = 0; i < n; ++i) {
      if (!unifClosure(fs->arguments[i], ft->arguments[i])) {
        return false;
      }
    }
    return true;
  }
  joinClass(s, t);
  return true;
}

bool QueryUnifyAL::findSolution(Term *s) {
  s = getSchemaTerm(findClass(s));
  if (acyclic.find(s) != acyclic.end()) {
    return true;
  }
  if (visited.find(s) != visited.end()) {
    return false;
  }
  if (s->isFunTerm && s->getAsFunTerm()->arguments.size() > 0) {
    visited.insert(s);
    for (auto &it : s->getAsFunTerm()->arguments) {
      if (!findSolution(it)) {
        return false;
      }
    }
    visited.erase(s);
  }
  acyclic.insert(s);
  Variable *vs = s->isVarTerm ? s->getAsVarTerm()->variable : nullptr;
  for (auto &it : getVars(findClass(s))) {
    if (it != vs) {
      subst.add(it, s);
    }
  }
  return true;
}

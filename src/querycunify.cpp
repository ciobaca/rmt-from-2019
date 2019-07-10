#include "querycunify.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include "factories.h"
#include "variable.h"
#include "varterm.h"
#include "funterm.h"
#include <functional>

using namespace std;

QueryCUnify::QueryCUnify() {}
  
Query *QueryCUnify::create() {
  return new QueryCUnify();
}
  
void QueryCUnify::parse(string &s, int &w) {
  matchString(s, w, "c-unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
  t1->vars(); t2->vars();
}

void QueryCUnify::execute() {
  cout << "C-Unifying " << t1->toString() << " and " << t2->toString() << endl;

  function<int(Term*)> countNumberOfCFunTerm = [&](Term *t) {
    if(t->isVarTerm) {
      return 0;
    }
    int cnt = t->getAsFunTerm()->function->isCommutative;
    for(auto term : t->getAsFunTerm()->arguments) {
      cnt += countNumberOfCFunTerm(term);
    }
    return cnt;
  };

  function<void(Term*&)> getIdentityPermutation = [&](Term *&t) {
    if(t->isVarTerm) {
      return;
    }

    vector<Term*> args = t->getAsFunTerm()->arguments;
    if(t->getAsFunTerm()->function->isCommutative) {
      sort(args.begin(), args.end());
    }
    Term *copyT = t;
    for(auto &term : args) {
      getIdentityPermutation(term);
    }

    t = getFunTerm(copyT->getAsFunTerm()->function, args);
  };

  function<bool(Term*&)> applyNextPermutation = [&](Term *&t) {
    if(t->isVarTerm) {
      return false;
    }

    vector<Term*> args = t->getAsFunTerm()->arguments;
    Term *copyT = t;
    for(auto &term : args) {
      if(applyNextPermutation(term)) {
        t = getFunTerm(copyT->getAsFunTerm()->function, args);
        return true;
      }
    }

    if(copyT->getAsFunTerm()->function->isCommutative) {
      if(next_permutation(args.begin(), args.end())) {
        t = getFunTerm(copyT->getAsFunTerm()->function, args);
        return true;
      }
    }

    t = getFunTerm(copyT->getAsFunTerm()->function, args);

    return false;
  };

  if(countNumberOfCFunTerm(t1) > countNumberOfCFunTerm(t2)) {
    swap(t1, t2);
  }
  getIdentityPermutation(t1);
  vector<Substitution> gSubst;
  do {
    Substitution subst;
    if(t1->unifyWith(t2, subst)) {
      gSubst.push_back(subst);
    }
  } while(applyNextPermutation(t1));

  if(!gSubst.size()) {
    cout << "No unification" << endl;
  } else {
    cout << "Substitutions:" << endl;
    for(auto &subst : gSubst) {
      cout << subst.toString() << endl;
    }
  }
}
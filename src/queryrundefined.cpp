#include "queryrundefined.h"
#include "parse.h"
#include "factories.h"
#include "z3driver.h"
#include "log.h"
#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

QueryRunDefined::QueryRunDefined()
{
}

Query *QueryRunDefined::create()
{
  return new QueryRunDefined();
}
  
void QueryRunDefined::parse(std::string &s, int &w)
{
  matchString(s, w, "rundefined");
  skipWhiteSpace(s, w);
  matchString(s, w, "in");
  skipWhiteSpace(s, w);
  rewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ":");
  skipWhiteSpace(s, w);
  term = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryRunDefined::execute()
{
  Log(DEBUG1) << "Trying" << endl;
  ConstrainedRewriteSystem crs;
  if (existsRewriteSystem(rewriteSystemName)) {
    crs = ConstrainedRewriteSystem(getRewriteSystem(rewriteSystemName));
  }
  else if (existsConstrainedRewriteSystem(rewriteSystemName)) {
    crs = getConstrainedRewriteSystem(rewriteSystemName);
  } else {
    Log(ERROR) << "Cannot find (constrained) rewrite system " << rewriteSystemName << endl;
    return;
  }
  Term *oldTerm;
  Term *newTerm = term;
  Substitution subst;
  const int maxSteps = 9999;
  int steps = 0;

  ConstrainedRewriteSystem definedFunctions = getDefinedFunctionsSystem(getDefinedFunctions());

  do {
    oldTerm = simplifyTerm(newTerm);
    cout << "STEP " << steps << ": " << oldTerm->toString() << endl;
    if (steps == maxSteps) {
      break;
    }
    newTerm = oldTerm->rewriteOneStep(crs, subst);
    if (newTerm == oldTerm) {
      vector< pair< ConstrainedSolution, bool > > v;
      Log(DEBUG) << "No successors " << newTerm->toString() << " taking defined functions" << endl;
      addDefinedSuccessors(v, newTerm, bTrue(), false, 0);
      if (!v.empty()) {
        ConstrainedSolution sol = v[0].first;
        ConstrainedTerm afterStep = ConstrainedTerm(sol.term, sol.constraint);
        afterStep = simplifyConstrainedTerm(afterStep.substitute(sol.subst).substitute(sol.simplifyingSubst));
        newTerm = afterStep.term;
      }
    }
    steps++;
  } while (newTerm != oldTerm);
  if (newTerm == oldTerm) {
    cout << "Reached normal form in " << steps << " steps." << endl;
  } else {
    cout << "Reached bound on number of rewrites (" << maxSteps << ")" << endl;
  }
}

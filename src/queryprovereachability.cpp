#include "queryprovereachability.h"
#include "constrainedterm.h"
#include "parse.h"
#include "factories.h"
#include "funterm.h"
#include "z3driver.h"
#include "log.h"
#include "sort.h"
#include "helper.h"
#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include <cassert>

using namespace std;

std::string stringFromReason(Reason reason)
{
  switch (reason) {
  case DepthLimit:
    return "depth limit exceeded";
  case BranchingLimit:
    return "branching limit exceeded";
  case Completeness:
    return "could not prove completeness";
  }
  Log(WARNING) << "Unknown reason" << endl;
  return "unknown reason";
}

QueryProveReachability::QueryProveReachability()
{
}
  
Query *QueryProveReachability::create()
{
  return new QueryProveReachability();
}

void QueryProveReachability::parse(std::string &s, int &w)
{
  matchString(s, w, "prove");
  skipWhiteSpace(s, w);
  if (lookAhead(s, w, "[")) {
    matchString(s, w, "[");
    skipWhiteSpace(s, w);
    maxDepth = getNumber(s, w);
    if (maxDepth < 0 || maxDepth > 99999) {
      Log(ERROR) << "Maximum depth (" << maxDepth << ") must be between 0 and 99999" << endl;
      expected("Legal maximum depth", w, s);
    }
    skipWhiteSpace(s, w);
    matchString(s, w, ",");
    skipWhiteSpace(s, w);
    maxBranchingDepth = getNumber(s, w);
    if (maxBranchingDepth < 0 || maxBranchingDepth > 99999) {
      Log(ERROR) << "Maximum branching depth (" << maxBranchingDepth << ") must be between 0 and 99999" << endl;
      expected("Legal maximum branching depth", w, s);
    }
    skipWhiteSpace(s, w);
    matchString(s, w, "]");
    skipWhiteSpace(s, w);
  } else {
    maxDepth = 100;
    maxBranchingDepth = 2;
  }
  matchString(s, w, "in");
  skipWhiteSpace(s, w);
  rewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ":");
  skipWhiteSpace(s, w);
  circularitiesRewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}
  
void QueryProveReachability::execute()
{
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
  ConstrainedRewriteSystem &circ = getConstrainedRewriteSystem(circularitiesRewriteSystemName);

  for (int i = 0; i < (int)circ.size(); ++i) {
    ConstrainedTerm lhs = circ[i].first;
    Term *rhs = circ[i].second;
    cout << endl;
    cout << "Proving circularity #" << i + 1 << ":" << endl;
    cout << "--------" << endl;
    proveCRS(lhs, rhs, crs, circ, false);
    cout << "--------" << endl;
    cout << "Circularity #" << i + 1 << (unproven.size() ? " not proved. The following proof obligations failed:" : " proved.") << endl;
    for (int i = 0; i < (int)unproven.size(); ++i) {
      cout << "Remaining proof obligation #" << i + 1 << " (reason: " << stringFromReason(unproven[i].reason) << "): " <<
	      unproven[i].lhs.toPrettyString() << " => " << unproven[i].rhs->toPrettyString() << endl;
    }
  }
}

// returns a constraint that describes when
// lhs implies rhs
Term *QueryProveReachability::proveByImplicationCRS(ConstrainedTerm lhs, Term *rhs,
			 ConstrainedRewriteSystem &, ConstrainedRewriteSystem &, int depth)
{
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;
  
  Term *unificationConstraint;
  Substitution subst;

  Log(DEBUG) << spaces(depth + 1) << "STEP 1. Does lhs imply rhs?" << endl;
  if (lhs.term->unifyModuloTheories(rhs, subst, unificationConstraint)) {
    Log(DEBUG4) << spaces(depth + 1) << "lhs unifies with rhs" << endl;
    Log(DEBUG4) << spaces(depth + 1) << "Unification constraint: " << unificationConstraint->toString() << endl;
    Term *constraint = bImplies(lhs.constraint, unificationConstraint);
    constraint = simplifyConstraint(constraint);
    constraint = constraint;
    if (isSatisfiable(bNot(constraint)) == unsat) {
      // the negation of the implication is unsatisfiable,
      // meaning that the implication is valid
      Log(INFO) << spaces(depth + 1) << "RHS is an instance of LHS, this branch is done." << endl;
      return bTrue();
    } else {
      Log(DEBUG2) << spaces(depth + 1) << "Constraint " << constraint->toString() << " is not valid." << endl;
      if (isSatisfiable(constraint) == sat) {
	      Log(INFO) << spaces(depth + 1) << "RHS is an instance of LHS in case " <<
	        constraint->toString() << endl;
	      return constraint;
      } else {
	      Log(INFO) << spaces(depth + 1) << "RHS is not an instance of LHS in any case (constraint not satisfiable) " <<
	        constraint->toString() << endl;
	      return bFalse();
      }
    }
  } else {
    Log(INFO) << spaces(depth + 1) << "RHS is not an instance of LHS in any case (no mgu)." << endl;
    return bFalse();
  }
}

// returns a constraint that describes when
// rhs can be reached from lhs by applying circularities
Term *QueryProveReachability::proveByCircularitiesCRS(ConstrainedTerm lhs, Term *rhs,
			   ConstrainedRewriteSystem &crs, ConstrainedRewriteSystem &circ, int depth, bool hadProgress,
			   int branchingDepth)
{
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;
  
  Log(DEBUG) << spaces(depth + 1) << "STEP 2. Does lhs rewrite using circularities?" << endl;
  Log(DEBUG) << spaces(depth + 1) << "LHS = " << lhs.toString() << endl;

  Term *circularityConstraint = bFalse();
  if (hadProgress) {
    vector<ConstrainedSolution> solutions;

    solutions = lhs.smtNarrowSearch(circ);

    Log(DEBUG) << spaces(depth + 1) << "Narrowing results in " << solutions.size() << " solutions." << endl;

    int newBranchDepth = solutions.size() > 1 ? branchingDepth + 1 : branchingDepth;
    for (int i = 0; i < (int)solutions.size(); ++i) {
      ConstrainedSolution sol = solutions[i];

      circularityConstraint = simplifyConstraint(bOr(
						     introduceExists(sol.constraint->substitute(sol.subst)->substitute(sol.simplifyingSubst),
								     sol.lhsTerm->uniqueVars()),
						     circularityConstraint));

      proveCRS(ConstrainedTerm(sol.term->substitute(sol.subst)->substitute(sol.simplifyingSubst),
			       sol.constraint->substitute(sol.subst)->substitute(sol.simplifyingSubst)),
	       rhs->substitute(sol.subst)->substitute(sol.simplifyingSubst), crs, circ, true, depth + 1, newBranchDepth);
    }
  }
  Log(INFO) << spaces(depth + 1) << "Circularities apply in case: " << circularityConstraint->toString() << endl;
  return circularityConstraint;
}

// returns a constraint that describes when
// rhs can be reached from lhs by applying circularities
Term *QueryProveReachability::proveByRewriteCRS(ConstrainedTerm lhs, Term *rhs,
		     ConstrainedRewriteSystem &crs, ConstrainedRewriteSystem &circ, int depth, bool, int branchingDepth)
{
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;

  Log(DEBUG) << spaces(depth + 1) << "STEP 3. Does lhs rewrite using trusted rewrite rules?" << endl;
  Log(DEBUG) << spaces(depth + 1) << "LHS = " << lhs.toString() << endl;

  // search for all successors in trusted rewrite system
  Term *rewriteConstraint = bFalse();

  vector<ConstrainedSolution> solutions = lhs.smtNarrowSearch(crs);

  Log(DEBUG) << spaces(depth + 1) << "Narrowing results in " << solutions.size() << " solutions." << endl;

  int newBranchDepth = (solutions.size() > 1) ? (branchingDepth + 1) : branchingDepth;
  for (int i = 0; i < (int)solutions.size(); ++i) {
    ConstrainedSolution sol = solutions[i];

    rewriteConstraint = simplifyConstraint(bOr(
					       introduceExists(sol.constraint->substitute(sol.subst)->substitute(sol.simplifyingSubst),
							       sol.lhsTerm->uniqueVars()),
					       rewriteConstraint));
    proveCRS(ConstrainedTerm(sol.term->substitute(sol.subst)->substitute(sol.simplifyingSubst),
			     sol.constraint->substitute(sol.subst)->substitute(sol.simplifyingSubst)),
	     rhs->substitute(sol.subst)->substitute(sol.simplifyingSubst), crs, circ, true, depth + 1, newBranchDepth);
  }

  Log(INFO) << spaces(depth + 1) << "Rewrite rules apply in case: " << rewriteConstraint->toString() << endl;

  return rewriteConstraint;
}

void QueryProveReachability::proveCRS(ConstrainedTerm lhs, Term *rhs,
			  ConstrainedRewriteSystem &crs, ConstrainedRewriteSystem &circ, bool hadProgress, int depth, int branchingDepth
			  )
{
  if (depth > maxDepth) {
    unproven.push_back(ProofObligation(simplifyConstrainedTerm(lhs), rhs, DepthLimit));
    Log(WARNING) << spaces(depth) << "(*****) Reached depth" << maxDepth << ", stopping search." << endl;
    return;
  }
  if (branchingDepth > maxBranchingDepth) {
    unproven.push_back(ProofObligation(simplifyConstrainedTerm(lhs), rhs, BranchingLimit));
    Log(WARNING) << spaces(depth) << "(*****) Reached branch depth " << maxBranchingDepth << ", stopping search." << endl;
    return;
  }

  if (lhs.constraint == 0) {
    lhs.constraint = bTrue();
  }
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;

  lhs = simplifyConstrainedTerm(lhs);
  ConstrainedTerm initialLhs = lhs;

  Log(INFO) << spaces(depth) << "Trying to prove " << lhs.toString() << " => " << rhs->toString() << endl;

  cout << spaces(depth) << "PROVING " << lhs.toString()  << " => " << rhs->toString() << endl;
  Term *implicationConstraint = proveByImplicationCRS(lhs, rhs, crs, circ, depth);
  ConstrainedTerm implLhs = lhs;
  implLhs.constraint = simplifyConstraint(bAnd(implLhs.constraint, implicationConstraint));
  cout << spaces(depth) << "  IMPL if " << implicationConstraint->toPrettyString() << endl;
  lhs.constraint = bAnd(lhs.constraint, bNot(implicationConstraint));
  lhs = simplifyConstrainedTerm(lhs);
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;

  Term *circularityConstraint = proveByCircularitiesCRS(lhs, rhs, crs, circ, depth, hadProgress, branchingDepth);
  ConstrainedTerm circLhs = lhs;
  circLhs.constraint = simplifyConstraint(bAnd(circLhs.constraint, circularityConstraint));
  cout << spaces(depth) << "  CIRC if " << circularityConstraint->toPrettyString() << endl;
  lhs.constraint = bAnd(lhs.constraint, bNot(circularityConstraint));
  lhs = simplifyConstrainedTerm(lhs);
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;

  Term *rewriteConstraint = proveByRewriteCRS(lhs, rhs, crs, circ, depth, hadProgress, branchingDepth);
  ConstrainedTerm rewrLhs = lhs;
  rewrLhs.constraint = simplifyConstraint(bAnd(rewrLhs.constraint, circularityConstraint));
  cout << spaces(depth) << "  REWR if " << rewriteConstraint->toPrettyString() << endl;
  lhs.constraint = bAnd(lhs.constraint, bNot(rewriteConstraint));
  lhs = simplifyConstrainedTerm(lhs);
  lhs.term = lhs.term;
  lhs.constraint = lhs.constraint;
  rhs = rhs;

  if (isSatisfiable(lhs.constraint) != unsat) {
    cout << spaces(depth) << "! Remaining proof obligation:" << lhs.toPrettyString() << " => " << rhs->toPrettyString() << endl;
    unproven.push_back(ProofObligation(lhs, rhs, Completeness));

    cout << spaces(depth) << "* Assuming that " << initialLhs.toPrettyString() << " => " << rhs->toPrettyString() << endl;
  } else {
    cout << spaces(depth) << "OKAY" << endl;
  }
}

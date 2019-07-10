#include "constrainedterm.h"
#include "term.h"
#include "log.h"
#include "factories.h"
#include "z3driver.h"
#include <sstream>
#include <cassert>
#include <set>
#include <algorithm>

using namespace std;

string ConstrainedTerm::toString()
{
  ostringstream oss;
  oss << term->toString();
  if (constraint) {
    oss << " /\\ ";
    oss << constraint->toString();
  }
  return oss.str();
}

string ConstrainedTerm::toPrettyString()
{
  ostringstream oss;
  oss << term->toPrettyString();
  if (constraint) {
    oss << " if ";
    oss << constraint->toPrettyString();
  }
  return oss.str();
}

ConstrainedRewriteSystem ConstrainedTerm::getDefinedFunctionsSystem(int printDepth)
{
  vector<Function *> definedFunctions = this->getDefinedFunctions();

  Log(DEBUG) << spaces(printDepth) << "Constrained term " << this->toString() << " has " << definedFunctions.size() << " defined symbols." << endl;

  return ::getDefinedFunctionsSystem(definedFunctions);
}

vector<ConstrainedTerm> ConstrainedTerm::smtNarrowDefinedSearch(int minDepth, int maxDepth, int printDepth)
{
  ConstrainedRewriteSystem rs = this->getDefinedFunctionsSystem(printDepth);
  return this->smtNarrowSearch(rs, minDepth, maxDepth);
}

vector<ConstrainedSolution> ConstrainedTerm::smtNarrowDefinedSearch(int printDepth)
{
  ConstrainedRewriteSystem rs = this->getDefinedFunctionsSystem(printDepth);
  return this->smtNarrowSearch(rs);
}

vector<ConstrainedSolution> ConstrainedTerm::smtRewriteDefined(int, ConstrainedRewriteSystem rs)
{
  vector<Variable *> vstemp = this->vars();
  vector<Variable *> vs;
  for (const auto &it : vstemp)
    if (it->sort == getIntSort())
      vs.push_back(it);
  sort(vs.begin(), vs.end());
  vs.resize(distance(vs.begin(), unique(vs.begin(), vs.end())));
  vector<Term *> cts;
  Substitution subst;
  for (int i = 0; i < (int)vs.size(); ++i) {
    cts.push_back(getFunTerm(getFreshConstant(vs[i]->sort), vector0()));
    subst.add(vs[i], cts[i]);
  }
  ConstrainedTerm newct = this->substitute(subst);
  Log(DEBUG5) << "Narrowing search for " << newct.toString() << endl;
  vector<ConstrainedSolution> result = newct.smtNarrowSearch(rs);
  for (int i = 0; i < (int)result.size(); ++i) {
    result[i] = result[i].unsubstitute(cts, vs);
  }
  return result;
}

vector<ConstrainedSolution> ConstrainedTerm::smtRewriteDefined(int printDepth) {
  return this->smtRewriteDefined(printDepth, this->getDefinedFunctionsSystem(printDepth));
}

vector<ConstrainedSolution> ConstrainedTerm::smtNarrowSearch(ConstrainedRewriteSystem &crs)
{
  Log(DEBUG1) << "ConstrainedTerm::smtNarrowSearch(ConstrainedRewriteSystem &crs) " << this->toString() << endl;
  Term *startTerm = term;
  Term *startConstraint = constraint;
  Log(DEBUG1) << "ConstrainedTerm::smtNarrowSearch(ConstrainedRewriteSystem &crs) Starting Term = " << startTerm->toString() << endl;
  Log(DEBUG1) << "ConstrainedTerm::smtNarrowSearch(ConstrainedRewriteSystem &crs) Starting Constraint = " << startConstraint->toString() << endl;
  vector<ConstrainedSolution> sols = startTerm->smtNarrowSearchWdf(crs, startConstraint);
  for (int i = 0; i < static_cast<int>(sols.size()); ++i) {
    Log(DEBUG1) << "Final solution " << i << " = " << sols[i].toString() << endl;
  }
  return sols;
}

vector<ConstrainedTerm> solutionsToSuccessors(vector<ConstrainedSolution> &solutions) {
  vector<ConstrainedTerm> successors;
  for (ConstrainedSolution &sol : solutions) {
    ConstrainedTerm ct =
      simplifyConstrainedTerm(ConstrainedTerm(sol.term, sol.constraint));
    successors.push_back(
      simplifyConstrainedTerm(ct.substitute(sol.subst).substitute(sol.simplifyingSubst)));
  }
  return successors;
}

void ConstrainedTerm::smtNarrowSearchHelper(ConstrainedRewriteSystem &crs,
							       int minDepth, int maxDepth, int depth,
							       vector<ConstrainedTerm> &result)
{
  Log(DEBUG2) << "ConstrainedTerm::smtNarrowSearchHelper " << this->toString() << endl;
  if (minDepth <= depth && depth <= maxDepth) {
    result.push_back(*this);
  } 
  if (depth < maxDepth) {
    vector<ConstrainedSolution> sols = this->smtNarrowSearch(crs);
    for (int i = 0; i < (int)sols.size(); ++i) {
      ConstrainedTerm ct(sols[i].term->substitute(sols[i].subst)->substitute(sols[i].simplifyingSubst),
			 sols[i].getFullConstraint()->substitute(sols[i].subst)->substitute(sols[i].simplifyingSubst));
      Log(DEBUG2) << "recurse " << ct.toString() << endl;
      ct.smtNarrowSearchHelper(crs, minDepth, maxDepth, depth + 1, result);
    }
  }
}

vector<ConstrainedTerm> ConstrainedTerm::smtNarrowSearch(ConstrainedRewriteSystem &crs, int minDepth, int maxDepth)
{
  assert(0 <= minDepth);
  assert(minDepth <= maxDepth);
  assert(maxDepth <= 99999999);

  vector<ConstrainedTerm> result;
  smtNarrowSearchHelper(crs, minDepth, maxDepth, 0, result);
  return result;
}

ConstrainedTerm ConstrainedTerm::normalize(RewriteSystem &rs)
{
  ConstrainedTerm ct(this->term->normalize(rs), this->constraint->normalize(rs));
  return ct;
}

vector<Variable *> ConstrainedTerm::vars()
{
  vector<Variable *> result;
  append(result, term->vars());
  append(result, constraint->vars());
  return result;
}

ConstrainedTerm ConstrainedTerm::substitute(Substitution &subst)
{
  ConstrainedTerm ct(term->substitute(subst), constraint->substitute(subst));
  return ct;
}

ConstrainedTerm ConstrainedTerm::fresh()
{
  vector<Variable *> myvars;
  append(myvars, term->vars());
  append(myvars, constraint->vars());

  map<Variable *, Variable *> renaming = freshRenaming(myvars);

  Substitution subst = createSubstitution(renaming);

  return this->substitute(subst);
}


// assume *this = <t1 if c1>
// assume goal = <t2 if c2>
// returns a constraint c such that
//
// forall X, c(X) /\ c1(X) -> exists Y, s.t. c2(X, Y) /\ t1(X) = t2(X,Y)
// 
// X are all variables in <t1 if c1>
// Y are all variables in <t2 if c2>
Term *ConstrainedTerm::whenImplies(ConstrainedTerm goal)
{
  ConstrainedTerm freshGoal = goal.fresh();
  vector<Variable *> vars = freshGoal.vars();
  Substitution subst;
  Term *constraint;
  Log(DEBUG) << "whenImplies " << this->toString() << " " << goal.toString() << endl; 
  if (this->term->unifyModuloTheories(freshGoal.term, subst, constraint)) {
    Log(DEBUG) << "whenImplies unification " << endl;
    Term *lhsConstraint = this->constraint->substitute(subst);
    Term *rhsConstraint = freshGoal.constraint->substitute(subst);
    Term *resultingConstraint = bImplies(lhsConstraint, introduceExists(bAnd(constraint, rhsConstraint), vars));
    if (isSatisfiable(bNot(resultingConstraint)) == unsat) {
      return bTrue();
    }
    if (isSatisfiable(resultingConstraint) != unsat) {
      return resultingConstraint;
    }
  }
  Log(DEBUG) << "whenImplies no unification" << endl;
  return bFalse();
}

// ConstrainedTerm ConstrainedTerm::normalizeFunctions()
// {
//   Term *newTerm = term, *newConstraint = constraint;
//   newTerm = term;
//   // if (term->hasDefinedFunctions) {
//   //   RewriteSystem functionsRS = getRewriteSystem("functions");
//   //   newTerm = term->normalize(functionsRS, false);
//   // }
//   newConstraint = constraint;
//   // if (constraint->hasDefinedFunctions) {
//   //   RewriteSystem functionsRS = getRewriteSystem("functions");
//   //   newConstraint = constraint->normalize(functionsRS, false);
//   // }
//   return ConstrainedTerm(newTerm, newConstraint);
// }

vector<Function *> ConstrainedTerm::getDefinedFunctions()
{
  set<Function *> where;
  term->getDefinedFunctions(where);
  constraint->getDefinedFunctions(where);
  vector<Function *> result;
  std::copy(where.begin(), where.end(), std::back_inserter(result));
  return result;
}

string ConstrainedPair::toString() {
  ostringstream oss;
  oss << lhs->toString() << " " << rhs->toString();
  if (constraint) {
    oss << " /\\ ";
    oss << constraint->toString();
  }
  return oss.str();
}

#include <cassert>
#include <iostream>
#include "term.h"
#include "log.h"
#include "z3driver.h"
#include "factories.h"
#include "varterm.h"
#include "constrainedterm.h"
#include "substitution.h"
#include <ctime>

using namespace std;

vector<Variable *> &Term::vars() 
{
  if (computedVars) {
    return allVars;
  } else {
    computedVars = true;
    return allVars = computeVars();
  }
}

vector<void *> &Term::varsAndFresh()
{
  if (computedVarsAndFresh) {
    return allVarsAndFresh;
  }
  else {
    computedVarsAndFresh = true;
    return allVarsAndFresh = computeVarsAndFresh();
  }
}


vector<Variable *> Term::computeUniqueVars()
{
  vector<Variable *> myv = this->vars();
  sort(myv.begin(), myv.end());
  vector<Variable *>::iterator last = unique(myv.begin(), myv.end());
  myv.erase(last, myv.end());
  return myv;
}

vector<Variable *> &Term::uniqueVars()
{
  if (computedUniqueVars) {
    return allUniqueVars;
  } else {
    computedUniqueVars = true;
    return allUniqueVars = computeUniqueVars();
  }
}

Term *Term::cachedSubstitute(Substitution &subst)
{
  map<Term *, Term *> cache;
  return computeSubstitution(subst, cache);
}

Term *Term::cachedSubstituteSingleton(Variable *v, Term *t)
{
  map<Term *, Term *> cache;
  return computeSingletonSubstitution(v, t, cache);
}

bool Term::isNormalized(RewriteSystem &rewriteSystem)
{
  map<Term *, bool> cache;
  return computeIsNormalized(rewriteSystem, cache);
}

Term *Term::normalize(RewriteSystem &rewriteSystem, bool optimallyReducing)
{
  Log(DEBUG9) << "Normalizing " << this->toString() << endl;
  map<Term *, Term *> cache;
  Term *result = computeNormalize(rewriteSystem, cache, optimallyReducing);
  return result;
}

bool Term::isInstanceOf(Term *t, Substitution &s)
{
  map<pair<Term *, Term *>, bool> cache;
  return computeIsInstanceOf(t, s, cache);
}

int Term::dagSize()
{
  map<Term *, int> cache;
  return computeDagSize(cache);
}

Term *Term::rewriteTopMost(RewriteSystem &rewrite, Substitution &how)
{
  for (int i = 0; i < len(rewrite); ++i) {
    pair<Term *, Term *> rewriteRule = rewrite[i];
    Term *result = rewriteTopMost(rewriteRule, how);
    if (this != result) {
      return result;
    }
  }
  return this;
}

Term *Term::rewriteTopMost(ConstrainedRewriteSystem &crs, Substitution &how)
{
  for (int i = 0; i < len(crs); ++i) {
    pair<ConstrainedTerm, Term *> crewriteRule = crs[i];
    Term *result = rewriteTopMost(crewriteRule, how);
    if (this != result) {
      return result;
    }
  }
  return this;
}

Term *Term::rewriteTopMost(pair<Term *, Term *> rewriteRule, Substitution &how)
{
  Term *l = rewriteRule.first;
  Term *r = rewriteRule.second;
    
  Substitution subst;
  if (this->isInstanceOf(l, subst)) {
    how = subst;
    return r->substitute(subst);
  }
  return this;
}

Term *Term::rewriteTopMost(pair<ConstrainedTerm, Term *> crewriteRule, Substitution &how)
{
  Log(DEBUG8) << "Term *Term::rewriteTopMost(pair<ConstrainedTerm, Term *> crewriteRule, Substitution &how)" << endl;
  Term *l = crewriteRule.first.term;
  Term *r = crewriteRule.second;
  Term *c = crewriteRule.first.constraint;

  assert(!l->hasDefinedFunctions);
  Substitution subst;
  if (this->isInstanceOf(l, subst)) {
    Log(DEBUG7) << "instance of " << l->toString() << endl;
    if (isSatisfiable(c->substitute(subst)) == sat) {
      Log(DEBUG7) << "    and satisfiable" << endl;
      // TODO does not work when constraint has variables not in the lhs of the rewrite rule
      // to solve this issue, should add to "how" the variables occuring in the constraint but not the rule
      how = subst;
      return r->substitute(subst);
    } else {
      Log(DEBUG7) << "    but not satisfiable (" << c->substitute(subst)->toString() << ")" << endl;
    }
  }
  Log(DEBUG8) << "not an instance of " << l->toString() << endl;
  return this;
}

bool unabstractSolution(Substitution &abstractingSubstitution, ConstrainedSolution &solution)
{
  
  // clock_t t0 = clock();

  // Log(DEBUG7) << "unabstractSolution" << endl;
  
  // Log(DEBUG7) << "Term = " << solution.term->toString() << endl;
  // Log(DEBUG7) << "Constraint = " << solution.constraint->toString() << endl;
  // Log(DEBUG7) << "Subst = " << solution.subst.toString() << endl;
  // Log(DEBUG7) << "LHS Term = " << solution.lhsTerm->toString() << endl;
  // Log(DEBUG7) << "Abstracting substitution = " << abstractingSubstitution.toString() << endl;

  Substitution &simplifyingSubst = solution.simplifyingSubst;
  for (Substitution::iterator it = solution.subst.begin(); it != solution.subst.end(); ++it) {
    Term *lhsTerm = getVarTerm(it->first)->substitute(abstractingSubstitution)->substitute(simplifyingSubst);
    Term *rhsTerm = it->second->substitute(abstractingSubstitution)->substitute(simplifyingSubst);
    //    Log(DEBUG9) << "Processing constraint " << lhsTerm->toString() << " = " << rhsTerm->toString() << endl;
    if (lhsTerm == rhsTerm) {
      //      Log(DEBUG9) << "Constraint is trivial, skipping" << endl;
      continue;
    }
    bool simplifiedConstraint = false;
    if (lhsTerm->isVarTerm) {
      //      Log(DEBUG7) << "Left-hand side is a variable." << endl;
      Variable *var = lhsTerm->getAsVarTerm()->variable;
      //      Log(DEBUG7) << "Variable " << var->name << " in domain of simplifyingSubst: " << simplifyingSubst.inDomain(var) << endl;
      //      Log(DEBUG7) << "Term " << rhsTerm->toString() << " has variable " << var->name << ": " << rhsTerm->hasVariable(var) << endl;
      //      Log(DEBUG7) << "Substitution " << abstractingSubstitution.toString() << " has variable " << var->name << " in range: " << abstractingSubstitution.inRange(var) << endl;
      if ((!(simplifyingSubst.inDomain(var)))) {
        if ((!(rhsTerm->hasVariable(var)))) {
          if ((!(abstractingSubstitution.inRange(var)))) {
	    //            Log(DEBUG7) << "Not yet in domain of simplifyingSubst, adding " << var->name << " |-> " << rhsTerm->toString() << "." << endl;
            simplifiedConstraint = true;
            simplifyingSubst.add(var, rhsTerm);
          }
        }
      }
    }
    if (!simplifiedConstraint && rhsTerm->isVarTerm) {
      //      Log(DEBUG7) << "Right-hand side is a variable." << endl;
      Variable *var = rhsTerm->getAsVarTerm()->variable;
      //      Log(DEBUG7) << "Variable " << var->name << " in domain of simplifyingSubst: " << simplifyingSubst.inDomain(var) << endl;
      //      Log(DEBUG7) << "Term " << lhsTerm->toString() << " has variable " << var->name << ": " << lhsTerm->hasVariable(var) << endl;
      //      Log(DEBUG7) << "Substitution " << abstractingSubstitution.toString() << " has variable " << var->name << " in range: " << abstractingSubstitution.inRange(var) << endl;
      if (!simplifyingSubst.inDomain(var) && !lhsTerm->hasVariable(var) && !abstractingSubstitution.inRange(var)) {
	//        Log(DEBUG7) << "Not yet in domain of simplifyingSubst, adding " << var->name << " |-> " << lhsTerm->toString() << "." << endl;
        simplifiedConstraint = true;
        simplifyingSubst.add(var, lhsTerm);
      }
    }
    if (!simplifiedConstraint) {
      //      Log(DEBUG7) << "Could not simplify constraint, sending to SMT solver." << endl;
      solution.constraint = bAnd(solution.constraint, createEqualityConstraint(lhsTerm, rhsTerm));
    }
  }

  for (Substitution::iterator it = abstractingSubstitution.begin(); it != abstractingSubstitution.end(); ++it) {
    solution.subst.force(it->first, it->second);
  }
  solution.simplifyingSubst = simplifyingSubst;

  //  Log(DEBUG7) << "Solution.subst = " << solution.subst.toString() << endl;
  //  Log(DEBUG7) << "Solution.simplifyingSubst = " << solution.simplifyingSubst.toString() << endl;

  Term *toCheck = simplifyConstraint(solution.constraint->substitute(solution.subst)->substitute(solution.simplifyingSubst));
  //  Log(DEBUG7) << "Checking satisfiability of " << toCheck->toString() << "." << endl;
  Z3Theory theory;
  vector<Variable *> &interpretedVariables = getInterpretedVariables();
  for (int i = 0; i < (int)interpretedVariables.size(); ++i) {
    theory.addVariable(interpretedVariables[i]);
  }
  theory.addConstraint(solution.constraint->substitute(solution.subst)->substitute(solution.simplifyingSubst));
  //  Log(DEBUG7) << "Sending to SMT solver." << endl;
  bool retval = false;

  if (theory.isSatisfiable() != unsat) {
    //    Log(DEBUG7) << "Possibly satisfiable." << endl;
    retval = true;
  }
  else {
    //    Log(DEBUG7) << "Surely unsatisfiable." << endl;
    retval = false;
  }

  return retval;
}

bool Term::unifyModuloTheories(Term *other, Substitution &resultSubstitution, Term *&resultConstraint)
{
  Log(DEBUG6) << "unifyModuloTheories " << this->toString() << " and " <<
    other->toString() << endl;
  Substitution abstractingSubstitution;

  Term *abstractTerm = this->abstract(abstractingSubstitution);
  Log(DEBUG6) << "Abstract term: " << abstractTerm->toString() << endl;
  Log(DEBUG6) << "Abstracting substitution: " << abstractingSubstitution.toString() << endl;
  
  Substitution unifyingSubstitution;
  if (abstractTerm->unifyWith(other, unifyingSubstitution)) {
    Log(DEBUG6) << "Syntactic unification succeeded. Unifying substitution: " << endl;
    Log(DEBUG6) << unifyingSubstitution.toString() << endl;
    Term *whatever = bTrue();
    ConstrainedSolution sol(whatever, bTrue(), unifyingSubstitution, whatever);

    if (unabstractSolution(abstractingSubstitution, sol)) {
      resultSubstitution = sol.subst; // TODO: compus cu simplifyingSubst?;
      resultConstraint = sol.constraint->substitute(sol.simplifyingSubst);
      for (Substitution::iterator it = sol.simplifyingSubst.begin(); it != sol.simplifyingSubst.end(); ++it) {
        resultSubstitution.force(it->first, it->second);
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

vector<ConstrainedSolution> Term::smtNarrowSearchBasic(ConstrainedRewriteSystem &crsInit, Term *initialConstraint)
{
  vector<ConstrainedSolution> finalResult;

  Substitution abstractingSubstitution;

  // STEP 1: compute abstracted term (and constraining substitution)
  Log(DEBUG) << "Term::smtNarrowSearchBasic(ConstrainedRewriteSystem &, Term *) " <<
    this->toString() << " /\\ " << initialConstraint->toString() << endl;

  if (initialConstraint == bFalse()) {
    return vector<ConstrainedSolution>();
  }
  Term *abstractTerm = this->abstract(abstractingSubstitution);

  Log(DEBUG) << "Abstract term: " << abstractTerm->toString() << endl;
  Log(DEBUG) << "Abstracting substitution: " << abstractingSubstitution.toString() << endl;

  // STEP 2: perform one-step narrowing from the abstract term
  Log(DEBUG9) << "Conditional system: " << crsInit.toString() << endl;
  ConstrainedRewriteSystem crs = crsInit.fresh();
  Log(DEBUG9) << "Fresh rewrite system: " << crs.toString() << endl;
  vector<ConstrainedSolution> solutions = abstractTerm->narrowSearch(crs);

  Log(DEBUG) << "Narrowing abstract term resulted in " << solutions.size() << " solutions" << endl;
  
  // STEP 3: check that the narrowing constraints are satisfiable
  if (initialConstraint == 0) {
    initialConstraint = bTrue();
  }

  // STEP 3.2: check that the constraints are satisfiable
  for (int i = 0; i < (int)solutions.size(); ++i) {
    Log(DEBUG) << "Initial solution " << i << " = " << solutions[i].toString() << endl;
    ConstrainedSolution sol = solutions[i];

    sol.constraint = bAnd(initialConstraint, sol.constraint);

    if (unabstractSolution(abstractingSubstitution, sol)) {
      Log(DEBUG) << "Final solution " << i << " = " << sol.toString() << endl;
      finalResult.push_back(sol);
    }
  }

  return finalResult;
}

vector<ConstrainedSolution> Term::smtNarrowSearchWdf(ConstrainedRewriteSystem &crsInit, Term *initialConstraint)
{
  Log(DEBUG7) << "Term::smtNarrowSearchWdt" << this->toString() << endl;
  return this->smtNarrowSearchBasic(crsInit, initialConstraint);
}

bool Term::hasVariable(Variable *var)
{
  vector<Variable *> &uvars = uniqueVars();
  for (vector<Variable *>::iterator it = uvars.begin(); it != uvars.end(); ++it) {
    if (*it == var) {
      return true;
    }
  }
  return false;
}

//Term *Term::normalizeFunctions()
//{
//  return this;
  // TODO this needs to be redesigned
  
  //  Log(DEBUG6) << "Term *Term::normalizeFunctions() (" << this->toString() << ")" << endl;
  // if (hasDefinedFunctions) {
  //   RewriteSystem functionsRS = getRewriteSystem("functions");
  //   Term *result = this->normalize(functionsRS, false);
  //   //    Log(DEBUG6) << "result (" << result->toString() << ")" << endl;
  //   assert(!result->hasDefinedFunctions);
  //   return result;
  // } else {
  //   return this;
  // }
//}

string &Term::toString() {
  if (this->stringRepresentation.size() == 0)
    this->computeToString();
  return this->stringRepresentation;
}

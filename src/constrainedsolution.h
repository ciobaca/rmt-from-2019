#ifndef CONSTRAINEDSOLUTION_H__
#define CONSTRAINEDSOLUTION_H__

#include <string>
#include "substitution.h"

struct Term;

/*
  Represents a solution to a narrowing search question.
  
  If we are asked to compute the successors of a term t in R, then a
  constrained solution { term |-> t_i, constraint |-> c_i, lhsTerm |-> l,
  subst |-> pi, simplifyingSubst |-> omega } is sound if:

  t pi_i omega_i => t_i pi_i omega_i if c_i pi_i omega_i in one step,
  
  for all valuations of the variables involved.
 */
struct ConstrainedTerm;

struct ConstrainedSolution
{
  Term *term;
  Term *constraint;
  Term *lhsTerm;
  Substitution subst;
  Substitution simplifyingSubst; // the part of the constraint that can be encoded as substitution

  ConstrainedSolution(Term *term, Term *constraint, Substitution &subst, Term *lhsTerm);
  ConstrainedSolution(Term *term, Substitution &subst, Term *lhsTerm);

  ConstrainedSolution unsubstitute(std::vector<Term *> &, std::vector<Variable *> &);
  
  Term *getFullConstraint(); // returns the full constraint, including the simplifyingSubstitution, assuming search started from the constrained term given as argument

  std::string toString();

  ConstrainedTerm getConstrainedTerm();
};

#endif

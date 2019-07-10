#ifndef TERM_H__
#define TERM_H__

#include <string>
#include <vector>
#include <set>
#include <z3.h>

#include "variable.h"
#include "function.h"
#include "rewritesystem.h"
#include "constrainedrewritesystem.h"
#include "constrainedsolution.h"
#include "substitution.h"
#include "helper.h"

using namespace std;

struct Substitution;
struct FunTerm;
struct VarTerm;
struct KnowledgeBase;
struct ConstrainedTerm;
struct ConstrainedRewriteSystem;

struct Term
{
  virtual Sort *getSort() = 0;

  bool hasDefinedFunctions;
  int countDefinedFunctions;
  bool isVarTerm;
  bool isFunTerm;

  bool computedVars;
  bool computedVarsAndFresh;
  bool computedUniqueVars;
  std::vector<Variable *> allVars;
  std::vector<void *> allVarsAndFresh;
  std::vector<Variable *> allUniqueVars;

  string stringRepresentation;

  Term() {
    computedVars = false;
    computedUniqueVars = false;
    computedVarsAndFresh = false;
    hasDefinedFunctions = false;
    countDefinedFunctions = 0;
  }
  
  // Returns the set of variables that appear in the term.  The result
  // is cached (a second call to vars will be O(1)).
  virtual vector<Variable *> &vars();

  // Returns the set of unique variables and fresh constants that appear in the term
  // in the order in which they appear. The result
  // is cached (a second call to vars will be O(1)).
  virtual vector<void*> &varsAndFresh();

  // Returns the set of variables in a term; no repetitions. Prefer
  // the cached version below to this one.
  virtual vector<Variable *> computeUniqueVars();

  virtual vector<void*> computeVarsAndFresh() = 0;
  
  // Returns the set of variables in a term; no repetitions. Caches
  // the result.
  virtual vector<Variable *> &uniqueVars();

  // Returns whether the term contains the variable given as argument
  virtual bool hasVariable(Variable *);

  // Apply the substitution given as a parameter to this term.
  virtual Term *cachedSubstitute(Substitution &);
  virtual Term *cachedSubstituteSingleton(Variable *v, Term *t);

  virtual Term *substitute(Substitution &) = 0;
  virtual Term *substituteSingleton(Variable *v, Term *t) = 0;

  // Helper function that applies the substitution given as the first
  // parameter.  The second parameter is a cache that records for
  // every (sub)term the result.  The cache allows to save computation
  // time if there are several equal subterms.  For example, when
  // appling the substitution sigma to f(g(x, y), g(x, y)), the cache
  // will contain at the end x |-> sigma(x); y |-> sigma(y); g(x, y)
  // |-> \sigma(g(x, y)) and f(g(x, y), g(x, y)) |-> sigma(f(g(x, y),
  // g(x, y))). In this way, the computation (applying the
  // substitution) is actually performed once for each subterm that is
  // different from previous subterms. If a subterm has been seen
  // before, the result is obtained from the cache.
  virtual Term *computeSubstitution(Substitution &, map<Term *, Term *> &) = 0;
  virtual Term *computeSingletonSubstitution(Variable *v, Term *t, map<Term *, Term *> &) = 0;

  // Compute the set of variable appearing in the term. O(|dag|).
  virtual vector<Variable *> computeVars() = 0;

  // Check if the term is in normal form w.r.t. the given rewrite
  // system.
  virtual bool isNormalized(RewriteSystem &rewriteSystem);

  // Compute a normal form of the term w.r.t. the given rewrite
  // system.  Can loop forever if the rewrite system is not
  // terminating.  The normalization is quite fast, using a cache that
  // remembers for each subterm appearing in the computation the
  // normalized form.
  // The boolean parameter should be set to false if the rs is not optimally reducing.
  virtual Term *normalize(RewriteSystem &rewriteSystem, bool = true);

  // Unifies this term with the parameter. Implements visitor pattern
  // for multiple dispatch.  The substitution is the substitution
  // compute so far.
  virtual bool unifyWith(Term *, Substitution &) = 0;

  // Unifies this term with the parameter. Implements visitor pattern
  // for multiple dispatch.  The substitution is the substitution
  // compute so far.
  virtual bool unifyWithFunTerm(FunTerm *, Substitution &) = 0;

  // Unifies this term with the parameter. Implements visitor pattern
  // for multiple dispatch.  The substitution is the substitution
  // compute so far.
  virtual bool unifyWithVarTerm(VarTerm *, Substitution &) = 0;

  // Checks whethere the current term is an instance of the parameter
  // (i.e. if there exists a substitution sigma such that *this =
  // \sigma(parameter)). If the result is true, then the second
  // parameter will contain the witness substitution.
  virtual bool isInstanceOf(Term *, Substitution &);

  // Converts the term "this" to a VarTerm.
  // Assumes that isVariable() is true.
  virtual VarTerm *getAsVarTerm() = 0;
  
  // Converts the term "this" to a FunTerm.
  // Assumes that isFunTerm() is true.
  virtual FunTerm *getAsFunTerm() = 0;

  virtual ~Term() {}

  // Computes if a term is in normal form w.r.t. the given rewrite
  // system.  The computation is quite fast, using a cache (the second
  // parameter) that remembers for each subterm if it is in normal
  // form or not.  You should probably use "isNormalized()" instead of
  // this function.
  virtual bool computeIsNormalized(RewriteSystem &, map<Term *, bool> &) = 0;

  // Compute a normal form of the term w.r.t. the given rewrite
  // system.  Can loop forever if the rewrite system is not
  // terminating.  The normalization is quite fast, using a cache (the
  // second parameter) that remembers for each subterm appearing in
  // the computation the normalized form. You should probably use
  // "normalize()" instead of this function.
  // The boolean parameter should be false when the rs is not optimally reducing.
  virtual Term *computeNormalize(RewriteSystem &, map<Term *, Term *> &, bool = true) = 0;

  // Checks whethere the current term is an instance of the parameter
  // (i.e. if there exists a substitution sigma such that *this =
  // \sigma(parameter)). If the result is true, then the second
  // parameter will contain the witness substitution. Implements visitor pattern
  // for multiple dispatch.
  virtual bool computeIsInstanceOf(Term *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;

  // Checks whethere the current term is a generalization of the
  // parameter, or equivalently, if the parameter is an instance of
  // the current term.  (i.e. if there exists a substitution sigma
  // such that \sigma(*this) = parameter). If the result is true, then
  // the second parameter will contain the witness substitution. The
  // third parameter is a cache that avoids repeating the same
  // computation. Implements visitor pattern for multiple dispatch,
  // together with computeIsInstanceOf.  virtual bool
  // computeIsGeneralizationOf(NamTerm *, Substitution &,
  // map<pair<Term *, Term *>, bool> &) = 0;
  virtual bool computeIsGeneralizationOf(VarTerm *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;

  // Checks whethere the current term is a generalization of the
  // parameter, or equivalently, if the parameter is an instance of
  // the current term.  (i.e. if there exists a substitution sigma
  // such that \sigma(*this) = parameter). If the result is true, then
  // the second parameter will contain the witness substitution. The
  // third parameter is a cache that avoids repeating the same
  // computation. Implements visitor pattern for multiple dispatch,
  // together with computeIsInstanceOf.  virtual bool
  // computeIsGeneralizationOf(NamTerm *, Substitution &,
  // map<pair<Term *, Term *>, bool> &) = 0;
  virtual bool computeIsGeneralizationOf(FunTerm *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;

  // Performs one top-most rewrite w.r.t. the given rewrite system.
  // Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteTopMost(RewriteSystem &, Substitution &how);

  // Performs one top-most rewrite w.r.t. the given rewrite rule.
  // Returns this if not possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteTopMost(pair<Term *, Term *>, Substitution &how);

  // Performs one top-most rewrite w.r.t. the given constrained rewrite system.
  // Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteTopMost(ConstrainedRewriteSystem &, Substitution &how);

  // Performs one top-most rewrite w.r.t. the given constrained rewrite rule.
  // Returns this if not possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteTopMost(pair<ConstrainedTerm, Term *>, Substitution &how);

  // Performs one rewrite step w.r.t. the given rewrite system (not
  // necessarily top-most).  Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteOneStep(RewriteSystem &, Substitution &how) = 0;

  // Performs one rewrite step w.r.t. the given rewrite rule (not
  // necessarily top-most).  Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteOneStep(pair<Term *, Term *>, Substitution &how) = 0;

  // Performs one rewrite step w.r.t. the given constrained rewrite system (not
  // necessarily top-most).  Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteOneStep(ConstrainedRewriteSystem &, Substitution &how) = 0;

  // Performs one rewrite step w.r.t. the given constrained rewrite rule (not
  // necessarily top-most).  Returns this if no rewrite is possible.
  // Fill in substitution with the witness to the rewrite.
  virtual Term *rewriteOneStep(pair<ConstrainedTerm, Term *>, Substitution &how) = 0;

  // Performs a one-step rewrite search, i.e. finds all terms to which
  // this reduces in one step.
  virtual vector<ConstrainedSolution> rewriteSearch(RewriteSystem &) = 0;

  // Performs a one-step narrowing search, i.e. finds all terms to which
  // this term narrows in one step.
  virtual vector<ConstrainedSolution> narrowSearch(ConstrainedRewriteSystem &) = 0;

  // Performs a one-step narrowing search, offloading interpreted terms
  // to the SMT solver.
  virtual vector<ConstrainedSolution> smtNarrowSearchBasic(ConstrainedRewriteSystem &, Term *initialConstraint);

  // Performs a one-step narrowing search, offloading interpreted terms
  // to the SMT solver. If the term contains defined functions, it
  // solves them using the "functions" rewrite system first.
  virtual vector<ConstrainedSolution> smtNarrowSearchWdf(ConstrainedRewriteSystem &, Term *initialConstraint);

  // Perform a unification modulo theories between *this and *other.
  // Assume t1 = *this and t2 = *other.
  // Builds a substitution sigma = resultSubstitution and a constraint c = resultConstraint
  // such that c implies t1\sigma = t2\sigma.
  // Returns false if c is unsatisfiable.
  bool unifyModuloTheories(Term *other, Substitution &resultSubstitution, Term *&resultConstraint);

  // Returns the number of different subterms (the same subterm is
  // counted only once, even if it appears in different places).  Uses
  // computeDagSize, which caches the results, ensuring O(|dagSize()|)
  // running time.
  virtual int dagSize();

  // Computed the number of different subterms (the same subterm is
  // counted only once, even if it appears in different places). The
  // parameter is a cache that eliminated duplicate computations.
  virtual int computeDagSize(map<Term *, int> &) = 0;

  // Returns the abstraction of a term w.r.t. the subterms that are
  // interpreted. The abstraction is a generalization of the term
  // where all subterms that are interpreted are replaced with
  // variables. Two different occurences of the same subterm must be
  // replaced with different variables.  The function returns the
  // abstracted term and the first parameter contains the witnessing
  // substitution.
  virtual Term *abstract(Substitution &) = 0;

  // return the reprezentation of a term in smt-speak (Z3)
  // virtual string toSmtString() = 0;
  virtual Z3_ast toSmt() = 0;

  // returns an infix representation of the term as a string (cached)
  virtual void computeToString() = 0;
  string &toString();

  // returns a term in which variable names are uniformized
  virtual Term *toUniformTerm(vector<void*> &allVars, map<Variable*, Term*> *subst = NULL) = 0;

  // returns a pretier representation of the term
  virtual string toPrettyString() = 0;

  // returns the term after computing all defined functions it contains
  virtual Term *compute() = 0;

  // return all defined function occuring in term
  virtual void getDefinedFunctions(std::set<Function *> &) = 0;

  // replace all constants by corresponding variables
  virtual Term *unsubstitute(vector<Term *> &cts, vector<Variable *> &vs) = 0;

  // reverse of "toUnifTerm" process
  virtual Term *unsubstituteUnif(map<Variable*, Term*> &subst) = 0;

  //count apparitions of a certain function in term
  virtual int nrFuncInTerm(Function *f) = 0;
};

bool unabstractSolution(Substitution &, ConstrainedSolution &);

#endif

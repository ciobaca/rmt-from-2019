#ifndef FUNTERM_H__
#define FUNTERM_H__

#include "term.h"
#include "constrainedsolution.h"
#include <set>

struct FunTerm : public Term
{
  Function *function;
  vector<Term *> arguments;

  FunTerm(Function *function, vector<Term *> arguments);

  vector<Variable *> computeVars();

  Sort *getSort();

  std::set<void*> seenVarsAndFresh; //auxiliary set to make sure all vars * fresh are unique

  virtual Term *substitute(Substitution &);
  virtual Term *substituteSingleton(Variable *v, Term *t);

  Term *computeSubstitution(Substitution &, map<Term *, Term *> &);
  Term *computeSingletonSubstitution(Variable *v, Term *t, map<Term *, Term *> &);

  bool unifyWith(Term *, Substitution &);
  bool unifyWithFunTerm(FunTerm *, Substitution &);
  bool unifyWithVarTerm(VarTerm *, Substitution &);

  VarTerm *getAsVarTerm();
  FunTerm *getAsFunTerm();

  bool computeIsNormalized(RewriteSystem &, map<Term *, bool> &);
  Term *computeNormalize(RewriteSystem &, map<Term *, Term *> &, bool);

  bool computeIsInstanceOf(Term *, Substitution &, map<pair<Term *, Term *>, bool> &);
  bool computeIsGeneralizationOf(VarTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);
  bool computeIsGeneralizationOf(FunTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);

  int computeDagSize(map<Term *, int> &);

  Term *rewriteOneStep(RewriteSystem &, Substitution &how);
  Term *rewriteOneStep(pair<Term *, Term *>, Substitution &how);
  Term *rewriteOneStep(ConstrainedRewriteSystem &, Substitution &how);
  Term *rewriteOneStep(pair<ConstrainedTerm, Term *>, Substitution &how);

  Term *abstract(Substitution &);

  Z3_ast toSmt();

  vector<void*> computeVarsAndFresh();

  void computeToString();
  string toPrettyString();

  vector<ConstrainedSolution> rewriteSearch(RewriteSystem &);
  vector<ConstrainedSolution> narrowSearch(ConstrainedRewriteSystem &);

  Term *compute();

  void getDefinedFunctions(std::set<Function *> &);
  Term *unsubstitute(std::vector<Term *> &cts, std::vector<Variable *> &vs);

  Term *toUniformTerm(std::vector<void*> &allVars, map<Variable*, Term*> *subst);

  Term *unsubstituteUnif(map<Variable*, Term*> &subst);

  int nrFuncInTerm(Function *f);
};

#endif

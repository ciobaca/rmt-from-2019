#ifndef VARTERM_H__
#define VARTERM_H__

#include "term.h"

struct VarTerm : public Term
{
  Variable *variable;
  Z3_ast interpretation;

  VarTerm(Variable *variable);

  vector<Variable *> computeVars();

  Sort *getSort();

  virtual Term *substitute(Substitution &);
  virtual Term *substituteSingleton(Variable *v, Term *t);
  Term *computeSubstitution(Substitution &, map<Term *, Term *> &);
  Term *computeSingletonSubstitution(Variable *v, Term *t, map<Term *, Term *> &);

  bool unifyWith(Term *, Substitution &);
  bool unifyWithFunTerm(FunTerm *, Substitution &);
  bool unifyWithVarTerm(VarTerm *, Substitution &);

  VarTerm *getAsVarTerm();
  FunTerm *getAsFunTerm();

  vector<void*> computeVarsAndFresh();

  bool computeIsNormalized(RewriteSystem &rewriteSystem, map<Term *, bool> &);
  Term *computeNormalize(RewriteSystem &, map<Term *, Term *> &, bool);

  bool computeIsInstanceOf(Term *, Substitution &, map<pair<Term *, Term *>, bool> &);
  bool computeIsGeneralizationOf(VarTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);
  bool computeIsGeneralizationOf(FunTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);

  Term *rewriteOneStep(RewriteSystem &, Substitution &how);
  Term *rewriteOneStep(pair<Term *, Term *>, Substitution &how);
  Term *rewriteOneStep(ConstrainedRewriteSystem &, Substitution &how);
  Term *rewriteOneStep(pair<ConstrainedTerm, Term *>, Substitution &how);

  int computeDagSize(map<Term *, int> &);

  Term *abstract(Substitution &);

  vector<ConstrainedSolution> rewriteSearch(RewriteSystem &);
  vector<ConstrainedSolution> narrowSearch(ConstrainedRewriteSystem &);

  Z3_ast toSmt();

  void computeToString();
  string toPrettyString();

  Term *compute();

  void getDefinedFunctions(std::set<Function *> &);
  Term *unsubstitute(std::vector<Term *> &cts, std::vector<Variable *> &vs);

  Term *toUniformTerm(std::vector<void*> &allVars, map<Variable*, Term*> *subst);

  Term *unsubstituteUnif(map<Variable*, Term*> &subst);

  int nrFuncInTerm(Function *f);
};

#endif

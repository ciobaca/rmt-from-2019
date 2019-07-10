#ifndef QUERYUNIFYAL_H__
#define QUERYUNIFYAL_H__

#include <string>
#include <map>
#include <set>
#include <vector>
#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include "triangularsubstitution.h"

struct QueryUnifyAL : public Query {
  Term *t1;
  Term *t2;

  QueryUnifyAL();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();

private:
  bool unifClosure(Term *s, Term *t);
  bool findSolution(Term *s);
  Term* findClass(Term *s);
  void joinClass(Term *s, Term *t);
  Term* getSchemaTerm(Term *s);
  Term* getEqClass(Term *s);
  int getSize(Term *s);
  std::vector<Variable*> getVars(Term *s);

  std::map<Term*, int> sz;
  std::map<Term*, std::vector<Variable*>> vars;
  std::map<Term*, Term*> eqClass;
  std::map<Term*, Term*> schemaTerm;
  std::set<Term*> visited;
  std::set<Term*> acyclic;
  TriangularSubstitution subst;
};

#endif

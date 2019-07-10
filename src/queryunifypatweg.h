#ifndef QUERYUNIFYPATWEG_H__
#define QUERYUNIFYPATWEG_H__

#include <string>
#include <map>
#include <set>
#include <vector>
#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include "substitution.h"

struct QueryUnifyPatWeg : public Query {
  Term *t1;
  Term *t2;

  QueryUnifyPatWeg();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();

private:
  void getParents(Term *s, Term *parent);
  bool finish(Term *r);
  Term* exploreVar(Term *s);
  Term* descend(Term *s);
  vector<Term*> exploreArgs(vector<Term*> v);

  std::map<Term*, std::vector<Term*>> parents;
  std::vector<Term*> varNodes;
  std::vector<Term*> funcNodes;
  std::set<Term*> complete;
  std::map<Term*, Term*> ptr;
  std::map<Term*, std::vector<Term*>> links;
  std::map<Term*, Term*> sigma;
  std::map<Term*, Term*> ready;
  Substitution subst;
};

#endif

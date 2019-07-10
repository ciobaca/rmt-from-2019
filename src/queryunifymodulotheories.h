#ifndef QUERYUNIFYMODULOTHEORIES_H__
#define QUERYUNIFYMODULOTHEORIES_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryUnifyModuloTheories : public Query
{
  Term *t1;
  Term *t2;

  QueryUnifyModuloTheories();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

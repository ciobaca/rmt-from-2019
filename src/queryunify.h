#ifndef QUERYUNIFY_H__
#define QUERYUNIFY_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryUnify : public Query
{
  Term *t1;
  Term *t2;

  QueryUnify();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

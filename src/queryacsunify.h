#ifndef QUERYACSUNIFY_H__
#define QUERYACSUNIFY_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryACSUnify : public Query {
  Term *t1;
  Term *t2;

  QueryACSUnify();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

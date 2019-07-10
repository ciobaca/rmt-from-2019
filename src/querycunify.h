#ifndef QUERYCUNIFY_H__
#define QUERYCUNIFY_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryCUnify : public Query {
  Term *t1;
  Term *t2;

  QueryCUnify();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

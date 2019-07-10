#ifndef QUERYACUUNIFY_H__
#define QUERYACUUNIFY_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryACUUnify : public Query {
  Term *t1;
  Term *t2;

  QueryACUUnify();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

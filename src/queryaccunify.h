#ifndef QUERYACCUNIFY_H__
#define QUERYACCUNIFY_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryACCUnify : public Query {
  Term *t1;
  Term *t2;

  QueryACCUnify();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

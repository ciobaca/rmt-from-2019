#ifndef QUERYDEFINEDREWRITE_H__
#define QUERYDEFINEDREWRITE_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryDefinedRewrite : public Query
{
  ConstrainedTerm ct;
  //  string funid;

  int minDepth;
  int maxDepth;

  QueryDefinedRewrite();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

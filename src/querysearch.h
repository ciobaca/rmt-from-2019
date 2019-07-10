#ifndef QUERYSEARCH_H__
#define QUERYSEARCH_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QuerySearch : public Query
{
  std::string rewriteSystemName;
  ConstrainedTerm ct;

  int minDepth;
  int maxDepth;

  QuerySearch();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

#ifndef QUERYDEFINEDSEARCH_H__
#define QUERYDEFINEDSEARCH_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryDefinedSearch : public Query
{
  ConstrainedTerm ct;
  //  string funid;

  int minDepth;
  int maxDepth;

  QueryDefinedSearch();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

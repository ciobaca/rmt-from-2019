#ifndef QUERYIMPLIES_H__
#define QUERYIMPLIES_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryImplies : public Query
{
  ConstrainedTerm ct1;
  ConstrainedTerm ct2;

  QueryImplies();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

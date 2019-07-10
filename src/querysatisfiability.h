#ifndef QUERYSATISFIABILITY_H__
#define QUERYSATISFIABILITY_H__

#include "term.h"
#include "query.h"
#include <string>
#include <map>

struct QuerySatisfiability : public Query
{
  Term *constraint;
  QuerySatisfiability();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

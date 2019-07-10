#ifndef QUERYSIMPLIFICATION_H__
#define QUERYSIMPLIFICATION_H__

#include "term.h"
#include "query.h"
#include <string>
#include <map>

struct QuerySimplification : public Query
{
  Term *constraint;
  QuerySimplification();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

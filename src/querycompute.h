#ifndef QUERYCOMPUTE_H__
#define QUERYCOMPUTE_H__

#include "query.h"
#include "term.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryCompute : public Query
{
  Term *t1;
  Term *t2;

  QueryCompute();
  virtual Query *create();
  virtual void parse(std::string &s, int &w);
  virtual void execute();
};

#endif

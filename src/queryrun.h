#ifndef QUERYRUN_H__
#define QUERYRUN_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryRun : public Query
{
  std::string rewriteSystemName;
  Term *term;

  QueryRun();
  
  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

#ifndef QUERYRUNDEFINED_H__
#define QUERYRUNDEFINED_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryRunDefined : public Query {
  std::string rewriteSystemName;
  Term *term;

  QueryRunDefined();
  
  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

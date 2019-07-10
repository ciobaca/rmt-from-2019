#ifndef QUERYDER_H__
#define QUERYDER_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryDer : public Query
{
  std::string rewriteSystemName;
  ConstrainedTerm ctLeft;
  ConstrainedTerm ctRight;

  QueryDer();

  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

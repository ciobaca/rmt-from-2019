#ifndef QUERYSUBS_H__
#define QUERYSUBS_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QuerySubs : public Query
{
  std::string rewriteSystemName;
  ConstrainedTerm ctLeft;
  ConstrainedTerm ctRight;

  QuerySubs();

  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

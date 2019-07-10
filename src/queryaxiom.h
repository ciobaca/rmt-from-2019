#ifndef QUERYAXIOM_H__
#define QUERYAXIOM_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryAxiom : public Query
{
  ConstrainedTerm ctLeft;
  ConstrainedTerm ctRight;

  QueryAxiom();

  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

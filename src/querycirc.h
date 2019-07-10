#ifndef QUERYCIRC_H__
#define QUERYCIRC_H__

#include "query.h"
#include "constrainedterm.h"
#include <string>
#include <map>

struct QueryCirc : public Query
{
  ConstrainedTerm circLeft, circRight;
  ConstrainedTerm ctLeft, ctRight;

  QueryCirc();

  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();
};

#endif

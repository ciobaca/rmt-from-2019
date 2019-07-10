#ifndef CONSTRAINEDREWRITESYSTEM_H__
#define CONSTRAINEDREWRITESYSTEM_H__

#include <vector>
#include <string>
#include "rewritesystem.h"
#include "constrainedterm.h"

struct ConstrainedRewriteSystem : public vector<pair<ConstrainedTerm, Term *> >
{
  ConstrainedRewriteSystem() {}
  ConstrainedRewriteSystem(RewriteSystem &);
  void addRule(ConstrainedTerm l, Term *r);
  ConstrainedRewriteSystem rename(string);
  ConstrainedRewriteSystem fresh();
  std::string toString();
};

#endif

#ifndef QUERYPROVEEQUIVALENCE_H__
#define QUERYPROVEEQUIVALENCE_H__

#include "query.h"
#include <string>
#include <vector>
#include "constrainedterm.h"
#include "constrainedrewritesystem.h"
#include "function.h"

struct QueryProveEquivalence : public Query
{
  std::string lrsName;
  std::string rrsName;
  std::vector<ConstrainedTerm> circularities;
  std::vector<ConstrainedTerm> base;

  Function *pairFun;

  Term *pair(Term *, Term *);
  ConstrainedTerm pairC(Term *, Term *, Term *);

  ConstrainedRewriteSystem crsLeft;
  ConstrainedRewriteSystem crsRight;

  int maxDepth;
  int maxBranchingDepth;

  QueryProveEquivalence();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);
  
  virtual void execute();

  Term *whenImpliesBase(ConstrainedTerm current);
  Term *whenImpliesCircularity(ConstrainedTerm current);

  bool possibleLhsBase(Term *);
  bool possibleRhsBase(Term *);
  bool possibleLhsCircularity(Term *lhs);
  bool possibleRhsCircularity(Term *rhs);
  bool possibleCircularity(ConstrainedTerm ct);

  bool proveEquivalenceForallLeft(ConstrainedTerm ct, bool progressLeft, bool progressRight, int depth, int branchingDepth);
  bool proveEquivalenceExistsRight(ConstrainedTerm ct, bool progressLeft, bool progressRight, int depth, int branchingDepth);
  bool proveEquivalence(ConstrainedTerm ct, bool progressLeft, bool progressRight, int depth, int branchingDepth);
  bool proveBaseCase(ConstrainedTerm ct, bool progressLeft, bool progressRight, int depth, int branchingDepth);
};

#endif

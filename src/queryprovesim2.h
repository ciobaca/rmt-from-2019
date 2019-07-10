#ifndef QUERYPROVESIM2_H__
#define QUERYPROVESIM2_H__

#include "query.h"
#include "term.h"

struct QueryProveSim2 : public Query {

  int boundL, boundR;

  ConstrainedRewriteSystem crsLeft, crsRight;
  vector<ConstrainedPair> circularities, base;

  vector<int> failedCircularities;

  QueryProveSim2();
  
  virtual Query *create();

  virtual void parse(std::string &s, int &w);

  virtual void execute();

  Term *prove(ConstrainedPair PQphi, int depth);
  Term *baseCase(ConstrainedPair PQphi, vector<ConstrainedPair> &base, int ldepth, int depth, int stepsRequired);
  Term *whenBase(ConstrainedPair PQphi, vector<ConstrainedPair> &base);

  //auxiliary functions
  void expandCircDefined(int from, int to);
};

#endif

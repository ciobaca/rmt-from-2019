#ifndef QUERYINSTRUMENT_H__
#define QUERYINSTRUMENT_H__

#include "query.h"
#include "constrainedterm.h"
#include "function.h"
#include <string>
#include <map>

struct QueryInstrument : public Query
{
  QueryInstrument();
  
  virtual Query *create();
  
  virtual void parse(std::string &s, int &w);

  virtual void execute();

private:
  std::string rewriteSystemName, newSystemName, oldStateSortName, newStateSortName,
    protectFunctionName;
  Term *leftSideProtection, *rightSideProtection, *naturalNumberConstraint;
  Function *protectFunction;
  bool initialize();
  void addRuleFromOldRule(ConstrainedRewriteSystem &nrs, Term *leftTerm, Term *leftConstraint, Term *rightTerm);
  void buildNewRewriteSystem();
};


#endif

#include "queryinstrument.h"
#include "parse.h"
#include "factories.h"
#include "z3driver.h"
#include "log.h"
#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

QueryInstrument::QueryInstrument()
{
}

Query *QueryInstrument::create()
{
  return new QueryInstrument();
}

void QueryInstrument::parse(std::string &s, int &w)
{
  matchString(s, w, "instrument");
  skipWhiteSpace(s, w);
  rewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  newSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  oldStateSortName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  newStateSortName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  protectFunctionName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

bool QueryInstrument::initialize() {
  if (!existsConstrainedRewriteSystem(rewriteSystemName) && !existsRewriteSystem(rewriteSystemName)) {
    Log(ERROR) << "Could not find constrained rewrite system " << rewriteSystemName << " (neigher regular or constrained)" << endl;
    return false;
  }
  if (existsConstrainedRewriteSystem(newSystemName) || existsRewriteSystem(newSystemName)) {
    Log(ERROR) << "There already exists a (constrained) rewrite system with name " << newSystemName << "." << endl;
    return false;
  }
  if (!sortExists(oldStateSortName)) {
    Log(ERROR) << "Sort of configurations " << oldStateSortName << " does not exist." << endl;
    return false;
  }
  if (!sortExists(newStateSortName)) {
    Log(ERROR) << "Sort of new configurations " << newStateSortName << " does not exist." << endl;
    return false;
  }
  protectFunction = getFunction(protectFunctionName);
  if (!protectFunction) {
    Log(ERROR) << "Function symbol" << protectFunctionName << " does not exist." << endl;
    return false;
  }
  if (protectFunction->hasInterpretation) {
    Log(ERROR) << "Function symbol" << protectFunctionName << " should be uninterpreted." << endl;
    return false;
  }
  {
    bool flag = true;
    if (protectFunction->arguments.size() != 2) flag = false;
    else if (protectFunction->arguments[0] != getSort(oldStateSortName)) flag = false;
    else if (protectFunction->arguments[1] != getIntSort()) flag = false;
    else if (protectFunction->result != getSort(newStateSortName)) flag = false;
    if (!flag) {
      Log(ERROR) << "Function symbol" << protectFunctionName << " is not of the appropriate arity." << endl;
      return false;
    }
  }
  Variable *protectVariable = createFreshVariable(getIntSort());

  leftSideProtection = getVarTerm(protectVariable);
  {
    vector<Term*> arguments;
    arguments.push_back(leftSideProtection);
    arguments.push_back(getIntOneConstant());
    rightSideProtection = getFunTerm(getMinusFunction(), arguments);
  }
  {
    vector<Term*> arguments;
    arguments.push_back(leftSideProtection);
    arguments.push_back(getIntZeroConstant());
    naturalNumberConstraint = bNot(getFunTerm(getLEFunction(), arguments));
  }
  return true;
}

void QueryInstrument::addRuleFromOldRule(ConstrainedRewriteSystem &nrs, Term *leftTerm, Term *leftConstraint, Term *rightTerm) {
  vector<Term*> arguments;

  if (leftTerm->getSort() == getSort(oldStateSortName)) {
    arguments.push_back(leftTerm);
    arguments.push_back(leftSideProtection);
    leftTerm = getFunTerm(protectFunction, arguments);
    leftConstraint = bAnd(leftConstraint, naturalNumberConstraint);
  }

  if (rightTerm->getSort() == getSort(oldStateSortName)) {
    arguments.clear();
    arguments.push_back(rightTerm);
    arguments.push_back(rightSideProtection);
    rightTerm = getFunTerm(protectFunction, arguments);
  }

  nrs.addRule(ConstrainedTerm(leftTerm, leftConstraint), rightTerm);
}

void QueryInstrument::buildNewRewriteSystem() {
  ConstrainedRewriteSystem nrs;

  ConstrainedRewriteSystem rs =
    existsConstrainedRewriteSystem(rewriteSystemName) ?
    getConstrainedRewriteSystem(rewriteSystemName) :
    ConstrainedRewriteSystem(getRewriteSystem(rewriteSystemName));
  for (const auto &it : rs)
    addRuleFromOldRule(nrs, it.first.term, it.first.constraint, it.second);

  putConstrainedRewriteSystem(newSystemName, nrs);
}

void QueryInstrument::execute()
{
  if (!initialize())
    return;

  buildNewRewriteSystem();

  ConstrainedRewriteSystem &testrs = getConstrainedRewriteSystem(newSystemName);
  for (const auto &it : testrs) {
    cout << it.first.term->toString() << " /\\ " << it.first.constraint->toString() << " => " <<
      it.second->toString() << endl;
  }
}

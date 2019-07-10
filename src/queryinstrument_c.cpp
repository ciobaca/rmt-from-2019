#include "queryinstrument_c.h"
#include "parse.h"
#include "factories.h"
#include "z3driver.h"
#include "log.h"
#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

QueryInstrument_C::QueryInstrument_C()
{
}

Query *QueryInstrument_C::create()
{
  return new QueryInstrument_C();
}

void QueryInstrument_C::parse(std::string &s, int &w)
{
  matchString(s, w, "cinstrument");
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

  if (!existsConstrainedRewriteSystem(rewriteSystemName) && !existsRewriteSystem(rewriteSystemName)) {
    Log(ERROR) << "Could not find constrained rewrite system " << rewriteSystemName << " (neigher regular or constrained)" << endl;
    return;
  }

  if (!sortExists(oldStateSortName)) {
    Log(ERROR) << "Sort of configurations " << oldStateSortName << " does not exist." << endl;
    return;
  }

  originalSystem.push_back(
    existsConstrainedRewriteSystem(rewriteSystemName) ?
    getConstrainedRewriteSystem(rewriteSystemName) :
    ConstrainedRewriteSystem(getRewriteSystem(rewriteSystemName)));

  for (const auto &it : originalSystem[0]) {
    if (it.first.term->getSort() != getSort(oldStateSortName)) continue;
    variants.push_back(parseTerm(s, w));
    skipWhiteSpace(s, w);
  }

  matchString(s, w, ";");
}

bool QueryInstrument_C::initialize() {
  if (existsConstrainedRewriteSystem(newSystemName) || existsRewriteSystem(newSystemName)) {
    Log(ERROR) << "There already exists a (constrained) rewrite system with name " << newSystemName << "." << endl;
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
  {
    Sort *intSort = getIntSort();
    leftSideProtection = getVarTerm(createFreshVariable(intSort)), rightSideProtection = getVarTerm(createFreshVariable(intSort));
    Function *mle = getLEFunction();
    vector<Term*> arguments;
    for (int i = 0; i < (int)variants.size(); ++i) {
      if (variants[i]->getSort() != intSort) {
        Log(ERROR) << "Variant number" << i + 1 << " is not of the appropriate (Int) sort." << endl;
        return false;
      }
      arguments.clear();
      arguments.push_back(variants[i]);
      arguments.push_back(leftSideProtection);
      variants[i] = getFunTerm(mle, arguments);
    }
  }

  {
    vector<Term*> arguments;
    arguments.push_back(getIntZeroConstant());
    arguments.push_back(leftSideProtection);
    naturalNumberConstraint = getFunTerm(getLEFunction(), arguments);
  }

  return true;
}

void QueryInstrument_C::addRuleFromOldRule(ConstrainedRewriteSystem &nrs, Term *leftTerm, Term *leftConstraint, Term *rightTerm, int &variantIndex) {
  if (leftTerm->getSort() != getSort(oldStateSortName)) {
    nrs.addRule(ConstrainedTerm(leftTerm, leftConstraint), rightTerm);
    return;
  }
  vector<Term*> arguments;

  if (leftTerm->getSort() == getSort(oldStateSortName)) {
    arguments.push_back(leftTerm);
    arguments.push_back(leftSideProtection);
    leftTerm = getFunTerm(protectFunction, arguments);
    leftConstraint = bAnd(bAnd(leftConstraint, naturalNumberConstraint), variants[variantIndex]);
    ++variantIndex;
  }

  if (rightTerm->getSort() == getSort(oldStateSortName)) {
    arguments.clear();
    arguments.push_back(rightTerm);
    arguments.push_back(rightSideProtection);
    rightTerm = getFunTerm(protectFunction, arguments);
  }

  nrs.addRule(ConstrainedTerm(leftTerm, leftConstraint), rightTerm);
}

void QueryInstrument_C::buildNewRewriteSystem() {
  ConstrainedRewriteSystem nrs;

  int variantIndex = 0;
  ConstrainedRewriteSystem &rs = originalSystem[0];
  for (int i = 0; i < (int)rs.size(); ++i)
    addRuleFromOldRule(nrs, rs[i].first.term, rs[i].first.constraint, rs[i].second, variantIndex);

  putConstrainedRewriteSystem(newSystemName, nrs);
}

void QueryInstrument_C::execute()
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

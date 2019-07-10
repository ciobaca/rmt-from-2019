#include "querysatisfiability.h"
#include "parse.h"
#include "z3driver.h"
#include <string>
#include <iostream>
#include <map>
#include <cassert>

using namespace std;

QuerySatisfiability::QuerySatisfiability()
{
}

Query *QuerySatisfiability::create()
{
  return new QuerySatisfiability();
}

void QuerySatisfiability::parse(std::string &s, int &w)
{
  matchString(s, w, "satisfiability");
  skipWhiteSpace(s, w);
  constraint = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QuerySatisfiability::execute()
{
  Z3Result result = isSatisfiable(constraint);
  if (result == sat) {
    cout << "The constraint " << constraint->toString() << " is SAT." << endl;
  } else if (result == unsat) {
    cout << "The constraint " << constraint->toString() << " is UNSAT." << endl;
  } else if (result == unknown) {
    cout << "Satisfiability check of constraint " << constraint->toString() << " is not conclusive." << endl;
  } else {
    assert(0);
  }
}

#include "querysimplification.h"
#include "parse.h"
#include "z3driver.h"
#include "factories.h"
#include <string>
#include <iostream>
#include <map>
#include <cassert>

using namespace std;

QuerySimplification::QuerySimplification()
{
}

Query *QuerySimplification::create()
{
  return new QuerySimplification();
}

void QuerySimplification::parse(std::string &s, int &w)
{
  matchString(s, w, "simplification");
  skipWhiteSpace(s, w);
  constraint = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QuerySimplification::execute()
{
  cout << "Simplifying " << constraint->toString() << endl;
  Term *result = simplifyTerm(constraint);
  cout << "The result is " << result->toString() << endl;
}

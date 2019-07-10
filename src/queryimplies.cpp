#include "queryimplies.h"
#include "parse.h"
#include "factories.h"
#include "z3driver.h"
#include "log.h"
#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

QueryImplies::QueryImplies() :
  ct1(0, 0),
  ct2(0, 0)
{
}

Query *QueryImplies::create()
{
  return new QueryImplies();
}
  
void QueryImplies::parse(std::string &s, int &w)
{
  matchString(s, w, "implies");
  skipWhiteSpace(s, w);
  ct1 = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  ct2 = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryImplies::execute()
{
  Substitution subst;
  assert(ct1.constraint);
  assert(ct2.constraint);
  cout << "Testing implication between " << ct1.toString() << " and " << ct2.toString() << endl;
  cout << "Implication holds in case: " << ct1.whenImplies(ct2)->toString() << endl;
}

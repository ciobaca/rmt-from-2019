#include "querycompute.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "factories.h"

using namespace std;

QueryCompute::QueryCompute()
{
}
  
Query *QueryCompute::create()
{
  return new QueryCompute();
}
  
void QueryCompute::parse(std::string &s, int &w)
{
  matchString(s, w, "compute");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryCompute::execute()
{
  Substitution subst;
  cout << "Computing " << t1->toString() << endl;
  Term *result = t1->compute();
  cout << "Result is " << result->toString() << endl;
}

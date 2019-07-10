#include "querydefinedrewrite.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QueryDefinedRewrite::QueryDefinedRewrite()
  : ct(0, 0)
{
}
  
Query *QueryDefinedRewrite::create()
{
  return new QueryDefinedRewrite();
}
  
void QueryDefinedRewrite::parse(std::string &s, int &w)
{
  matchString(s, w, "definedrewrite");
  skipWhiteSpace(s, w);
  if (lookAhead(s, w, "[")) {
    matchString(s, w, "[");
    skipWhiteSpace(s, w);
    minDepth = getNumber(s, w);
    skipWhiteSpace(s, w);
    matchString(s, w, ",");
    skipWhiteSpace(s, w);
    maxDepth = getNumber(s, w);
    skipWhiteSpace(s, w);
    matchString(s, w, "]");
    skipWhiteSpace(s, w);
  } else {
    minDepth = maxDepth = 1;
  }
  skipWhiteSpace(s, w);
  ct = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  // matchString(s, w, "for");
  // skipWhiteSpace(s, w);
  // funid = getIdentifier(s, w);
  // skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryDefinedRewrite::execute()
{
  Log(DEBUG) << "Defined rewriteing from " << ct.toString() << "." << endl;
  vector<ConstrainedSolution> sols = ct.smtRewriteDefined(0);
  vector<ConstrainedTerm> solutions = solutionsToSuccessors(sols);
  cout << "Success: " << solutions.size() << " solutions." << endl;
  for (int i = 0; i < (int)solutions.size(); ++i) {
    cout << "Solution #" << i + 1 << ":" << endl;
    cout << simplifyConstrainedTerm(solutions[i]).toString() << endl;
  }
}

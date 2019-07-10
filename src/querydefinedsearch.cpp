#include "querydefinedsearch.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QueryDefinedSearch::QueryDefinedSearch()
  : ct(0, 0)
{
}
  
Query *QueryDefinedSearch::create()
{
  return new QueryDefinedSearch();
}
  
void QueryDefinedSearch::parse(std::string &s, int &w)
{
  matchString(s, w, "definedsearch");
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

void QueryDefinedSearch::execute()
{
  ConstrainedRewriteSystem crsFinal = ct.getDefinedFunctionsSystem();

  Log(DEBUG) << "Defined searching from " << ct.toString() << "." << endl;
  vector<ConstrainedTerm> solutions = ct.smtNarrowSearch(crsFinal, minDepth, maxDepth);
  cout << "Success: " << solutions.size() << " solutions." << endl;
  for (int i = 0; i < (int)solutions.size(); ++i) {
    cout << "Solution #" << i + 1 << ":" << endl;
    cout << simplifyConstrainedTerm(solutions[i]).toString() << endl;
  }
}

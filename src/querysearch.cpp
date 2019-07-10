#include "querysearch.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QuerySearch::QuerySearch()
  : ct(0, 0)
{
}
  
Query *QuerySearch::create()
{
  return new QuerySearch();
}
  
void QuerySearch::parse(std::string &s, int &w)
{
  matchString(s, w, "search");
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
  matchString(s, w, "in");
  skipWhiteSpace(s, w);
  rewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ":");
  skipWhiteSpace(s, w);
  ct = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QuerySearch::execute()
{
  ConstrainedRewriteSystem crs;
  if (existsRewriteSystem(rewriteSystemName)) {
    crs = ConstrainedRewriteSystem(getRewriteSystem(rewriteSystemName));
  }
  else if (existsConstrainedRewriteSystem(rewriteSystemName)) {
    crs = getConstrainedRewriteSystem(rewriteSystemName);
  }
  else {
    Log(ERROR) << "Cannot find (constrained) rewrite system " << rewriteSystemName << endl;
    return;
  }

  Log(DEBUG) << "Narrowing search from " << ct.toString() << "." << endl;
  vector<ConstrainedTerm> solutions = ct.smtNarrowSearch(crs, minDepth, maxDepth);
  cout << "Success: " << solutions.size() << " solutions." << endl;
  for (int i = 0; i < (int)solutions.size(); ++i) {
    cout << "Solution #" << i + 1 << ":" << endl;
    //cout << solutions[i].toString() << endl;
    cout << simplifyConstrainedTerm(solutions[i]).toString() << endl;
  }
}

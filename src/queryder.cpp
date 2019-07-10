#include "queryder.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QueryDer::QueryDer()
  : ctLeft(0, 0), ctRight(0, 0)
{
}
  
Query *QueryDer::create()
{
  return new QueryDer();
}
  
void QueryDer::parse(std::string &s, int &w)
{
  matchString(s, w, "der");
  skipWhiteSpace(s, w);
  matchString(s, w, "in");
  skipWhiteSpace(s, w);
  rewriteSystemName = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ":");
  skipWhiteSpace(s, w);
  ctLeft = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, "=>");
  skipWhiteSpace(s, w);
  ctRight = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryDer::execute()
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

  vector<ConstrainedTerm> solutions = ctLeft.smtNarrowSearch(crs, 1, 1);
  cout << "Success: " << solutions.size() << " solutions." << endl;
  for (int i = 0; i < (int)solutions.size(); ++i) {
    cout << "Solution #" << i + 1 << ":" << endl;
    cout << simplifyConstrainedTerm(solutions[i]).toString() << " => " << simplifyConstrainedTerm(ctRight).toString() << endl;
  }
}

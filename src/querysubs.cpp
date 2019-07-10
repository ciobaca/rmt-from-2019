#include "querysubs.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QuerySubs::QuerySubs()
  : ctLeft(0, 0), ctRight(0, 0)
{
}
  
Query *QuerySubs::create()
{
  return new QuerySubs();
}
  
void QuerySubs::parse(std::string &s, int &w)
{
  matchString(s, w, "subs");
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

void QuerySubs::execute()
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

  Term *constraint = ctLeft.whenImplies(ctRight);
  ctLeft.constraint = bAnd(ctLeft.constraint, bNot(constraint));
  
  cout << "Success: " << 1 << " solutions." << endl;
  cout << "Solution #" << 1 << ":" << endl;
  cout << simplifyConstrainedTerm(ctLeft).toString() << " => " << simplifyConstrainedTerm(ctRight).toString() << endl;
}

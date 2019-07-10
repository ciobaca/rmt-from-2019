#include "querycirc.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"

using namespace std;

QueryCirc::QueryCirc()
  : circLeft(0, 0), circRight(0, 0), ctLeft(0, 0), ctRight(0, 0)
{
}
  
Query *QueryCirc::create()
{
  return new QueryCirc();
}
  
void QueryCirc::parse(std::string &s, int &w)
{
  matchString(s, w, "circ");
  skipWhiteSpace(s, w);
  circLeft = parseConstrainedTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, "=>");
  skipWhiteSpace(s, w);
  circRight = parseConstrainedTerm(s, w);
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

void QueryCirc::execute()
{
  Term *phiLeft = ctLeft.constraint;
  
  Term *phi = ctLeft.whenImplies(circLeft);
  ctLeft.constraint = bAnd(phiLeft, bNot(phi));

  circRight.constraint = bAnd(circRight.constraint, bAnd(phi, phiLeft));
  
  cout << "Success: " << 2 << " solutions." << endl;

  cout << "Solution #" << 1 << ":" << endl;
  cout << simplifyConstrainedTerm(ctLeft).toString() << " => " << simplifyConstrainedTerm(ctRight).toString() << endl;

  cout << "Solution #" << 2 << ":" << endl;
  cout << simplifyConstrainedTerm(circRight).toString() << " => " << simplifyConstrainedTerm(ctRight).toString() << endl;
}

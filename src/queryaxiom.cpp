#include "queryaxiom.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "log.h"
#include "z3driver.h"

using namespace std;

QueryAxiom::QueryAxiom()
  : ctLeft(0, 0), ctRight(0, 0)
{
}
  
Query *QueryAxiom::create()
{
  return new QueryAxiom();
}
  
void QueryAxiom::parse(std::string &s, int &w)
{
  matchString(s, w, "axiom");
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

void QueryAxiom::execute()
{
  if (isSatisfiable(ctLeft.constraint) == unsat) {
    cout << "Success: " << 0 << " solutions." << endl;
  } else {
    cout << "Error: left hand side constraint might be satisfiable." << endl;
  }
}

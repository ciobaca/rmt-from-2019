#include "queryunify.h"
#include "parse.h"
#include "factories.h"
#include <string>
#include <map>
#include <iostream>
#include "factories.h"

using namespace std;

QueryUnify::QueryUnify()
{
}
  
Query *QueryUnify::create()
{
  return new QueryUnify();
}
  
void QueryUnify::parse(std::string &s, int &w)
{
  matchString(s, w, "unify");
  skipWhiteSpace(s, w);
  t1 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  t2 = parseTerm(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ";");
}

void QueryUnify::execute()
{
  Substitution subst;
  cout << "Unifying " << t1->toString() << " and " << t2->toString() << endl;
  Term *constraint;
  if (t1->unifyModuloTheories(t2, subst, constraint)) {
    cout << "Unification results:" << endl;
    cout << "Substitution: " << subst.toString() << endl;
    cout << "Constraint: " << constraint->toString() << endl;
    cout << "Constraint (simplified): " << simplifyConstraint(constraint)->toString() << endl;
  } else {
    cout << "No unification." << endl;
  }
}

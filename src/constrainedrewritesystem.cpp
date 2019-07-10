#include "constrainedrewritesystem.h"
#include "term.h"
#include "substitution.h"
#include "log.h"
#include "variable.h"
#include "factories.h"
#include "helper.h"
#include "rewritesystem.h"
#include <cassert>
#include <string>
#include <sstream>

using namespace std;

ConstrainedRewriteSystem::ConstrainedRewriteSystem(RewriteSystem &rs)
{
  for (RewriteSystem::iterator it = rs.begin(); it != rs.end(); ++it) {
    addRule(ConstrainedTerm(it->first, bTrue()), it->second);
  }
}

void ConstrainedRewriteSystem::addRule(ConstrainedTerm l, Term *r)
{
  this->push_back(make_pair(l, r));
}

ConstrainedRewriteSystem ConstrainedRewriteSystem::rename(string s)
{
  vector<Variable *> vars;
  for (int i = 0; i < (int)this->size(); ++i) {
    append(vars, (*this)[i].first.vars());
    append(vars, (*this)[i].second->vars());
  }

  map<Variable *, Variable *> r = createRenaming(vars, s);
  Substitution subst = createSubstitution(r);

  ConstrainedRewriteSystem result;
  for (int i = 0; i < (int)this->size(); ++i) {
    ConstrainedTerm l = (*this)[i].first;
    Term *r = (*this)[i].second;
    result.push_back(make_pair(l.substitute(subst), r->substitute(subst)));
  }

  return result;
}

ConstrainedRewriteSystem ConstrainedRewriteSystem::fresh()
{
  Log(DEBUG8) << "Creating fresh rewrite system" << endl;
  //  static int counter = 100;
  vector<Variable *> myvars;
  for (int i = 0; i < (int)this->size(); ++i) {
    append(myvars, (*this)[i].first.vars());
    append(myvars, (*this)[i].second->vars());
  }
  Log(DEBUG8) << "Variables: " << varVecToString(myvars) << endl;

  map<Variable *, Variable *> renaming = freshRenaming(myvars);
  
  Substitution subst = createSubstitution(renaming);

  ConstrainedRewriteSystem result;
  for (int i = 0; i < (int)this->size(); ++i) {
    ConstrainedTerm l = (*this)[i].first;
    Term *r = (*this)[i].second;
    result.push_back(make_pair(l.substitute(subst), r->substitute(subst)));
  }

  return result;
}

string ConstrainedRewriteSystem::toString()
{
  ostringstream oss;
  for (int i = 0; i < (int)this->size(); ++i) {
    ConstrainedTerm l = (*this)[i].first;
    Term *r = (*this)[i].second;
    oss << l.toString() << " => " << r->toString();
    if (i != (int)this->size() - 1) {
      oss << ", ";
    }
  }
  oss << ";";
  return oss.str();
}

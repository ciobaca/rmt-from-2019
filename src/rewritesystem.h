#ifndef REWRITESYSTEM_H__
#define REWRITESYSTEM_H__

#include <vector>
#include <string>

struct Term;
struct Variable;

using namespace std;

struct RewriteSystem : public vector<pair<Term *, Term *> >
{
  void addRule(Term *l, Term *r);
  RewriteSystem rename(string);
  RewriteSystem fresh();
  string toString();
};

#endif

#ifndef TRIANGULARSUBSTITUTION_H__
#define TRIANGULARSUBSTITUTION_H__

#include <vector>
#include <utility>
#include <string>
#include "substitution.h"

struct Term;
struct Variable;

struct TriangularSubstitution : public std::vector<std::pair<Variable *, Term *>>
{
  void add(Variable *v, Term *t);
  bool inDomain(Variable *v);
  bool inRange(Variable *v);
  Term *image(Variable *v);
  std::string toString();
  Substitution getSubstitution();
};

#endif

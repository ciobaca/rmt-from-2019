#ifndef SUBSTITUTION_H__
#define SUBSTITUTION_H__
#include <vector>
#include <string>
#include <utility>

struct Term;
struct Variable;

struct Substitution : public std::vector<std::pair<Variable *, Term *>> {
  Substitution();
  Substitution(Variable *v, Term *t);
  void apply(Substitution &);
  void force(Variable *v, Term *t);
  void add(Variable *v, Term *t);
  bool inDomain(Variable *v);
  bool inRange(Variable *v);
  Term *image(Variable *v);
  Substitution unsubstitute(std::vector<Term *> cts, std::vector<Variable *> vs);
  std::string toString();
};
#endif

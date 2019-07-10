#ifndef VARIABLE_H__
#define VARIABLE_H__

#include <string>
#include <z3.h>

using namespace std;

struct Substitution;
struct Sort;

struct Variable
{
  Sort *sort;
  string name;
  Z3_ast interpretation;
  Variable *rename(Substitution &);

private:
  Variable(string, Sort *);

  friend Variable *createVariable(string, Sort *);
};

#endif

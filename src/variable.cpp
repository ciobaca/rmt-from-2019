#include "variable.h"
#include "substitution.h"
#include "term.h"
#include "varterm.h"
#include "sort.h"
#include <cassert>

using namespace std;

Variable::Variable(string name, Sort *sort)
{
  this->name = name;
  this->sort = sort;
  if (sort->hasInterpretation) {
    interpretation = z3_make_constant(this);
  }
}

Variable *Variable::rename(Substitution &subst)
{
  if (subst.inDomain(this)) {
    Term *image = subst.image(this);
    VarTerm *vt = dynamic_cast<VarTerm *>(image);
    assert(vt);
    return vt->variable;
  } else {
    return this;
  }
}

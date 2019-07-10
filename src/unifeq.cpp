#include "unifeq.h"

UnifEq::UnifEq(Term *t1, Term *t2) {
  if (t1 && t2 && t1->isFunTerm && t2->isVarTerm) {
    swap(t1, t2);
  }
  this->t1 = t1;
  this->t2 = t2;
}

#ifndef UNIFEQ_H__
#define UNIFEQ_H__

#include "term.h"

struct UnifEq {
  Term *t1;
  Term *t2;

  UnifEq(Term *t1, Term *t2);
};

#endif

#ifndef SORT_H__
#define SORT_H__

#include <vector>
#include <string>

#include <z3.h>
#include "z3driver.h"
#include "helper.h"

using namespace std;

struct Sort
{
  string name;

  bool hasInterpretation;
  Z3_sort interpretation;
  // string interpretation;

  vector<Sort *> subSorts;
  vector<Sort *> superSorts;

  Sort(string);
  
  Sort(string, string);
  
  bool addSubSort(Sort *);

  bool hasSubSortTR(Sort *);
};

#endif

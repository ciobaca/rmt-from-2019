#include <algorithm>
#include "unifeqsystem.h"
#include "substitution.h"
#include "factories.h"

UnifEqSystem::UnifEqSystem(Term *t1, Term *t2) {
  if (t1->isFunTerm && t2->isVarTerm) {
    std::swap(t1, t2);
  }
  this->emplace_back(t1, t2);
}

UnifEqSystem::UnifEqSystem(const UnifEq &eq) {
  this->push_back(eq);
}

UnifEqSystem::UnifEqSystem(const UnifEqSystem &ues) : std::vector<UnifEq> (ues) {
  this->sortUES();
}

UnifEqSystem::UnifEqSystem(const Substitution &sol, const UnifEqSystem &ues) : std::vector<UnifEq> (ues) {
  for (auto &it : sol) {
    this->emplace_back(getVarTerm(it.first), it.second);
  }
  this->sortUES();
}

void UnifEqSystem::addEq(const UnifEq &eq, bool toSort) {
  this->push_back(eq);
  if (toSort) {
    this->sortUES();
  }
}

void UnifEqSystem::decomp(FunTerm *t1, FunTerm *t2) {
  int n = t1->arguments.size();
  for (int i = 0; i < n; ++i) {
    this->push_back(UnifEq(t1->arguments[i], t2->arguments[i]));
  }
  this->sortUES();
}

void UnifEqSystem::sortUES() {
  std::sort(this->begin(), this->end(), [](const UnifEq &a, const UnifEq &b) {
    return a.t1->isVarTerm + a.t2->isVarTerm < b.t1->isVarTerm + b.t2->isVarTerm;
  });
}

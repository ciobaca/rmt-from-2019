#include "triangularsubstitution.h"
#include "substitution.h"
#include "helper.h"
#include "term.h"
#include <cassert>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <iostream>

using namespace std;

void TriangularSubstitution::add(Variable *v, Term *t) {
  this->emplace_back(v, t);
}

bool TriangularSubstitution::inDomain(Variable *v) {
  return find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.first == v; }) != this->end();
}

bool TriangularSubstitution::inRange(Variable *v) {
  return find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.second->hasVariable(v); }) != this->end();
}

Term *TriangularSubstitution::image(Variable *v) {
  auto ans = find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.first == v; });
  if (ans != this->end()) {
    return ans->second;
  }
  throw std::runtime_error("Triangular Substitution does not contain needed variable");
}

string TriangularSubstitution::toString() {
  if (!this->size()) {
    return "( )";
  }
  ostringstream oss;
  oss << "( ";
  auto lastItem = *(this->rbegin());
  for (const auto &it : *this) {
    oss << it.first->name << " |-> " << it.second->toString() << (it == lastItem  ? " )" : " | ");
  }
  return oss.str();
}

Substitution TriangularSubstitution::getSubstitution() {
  Substitution subst;
  for (const auto &it : *this) {
    subst.force(it.first, it.second);
  }
  return subst;
}

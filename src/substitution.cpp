#include "substitution.h"
#include "helper.h"
#include "term.h"
#include "varterm.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

Substitution::Substitution() {
}

Substitution::Substitution(Variable *v, Term *t) {
  if (!inDomain(v) || !(t->isVarTerm && v == t->getAsVarTerm()->variable)) {
    this->emplace_back(v, t);
  }
}

void Substitution::add(Variable *v, Term *t) {
  if (!inDomain(v) || !(t->isVarTerm && v == t->getAsVarTerm()->variable)) {
    this->emplace_back(v, t);
  }
}

void Substitution::apply(Substitution &s) {
  for (auto &it : *this) {
    it.second = it.second->substitute(s);
  }
}

void Substitution::force(Variable *v, Term *t) {
  for (auto &it : *this) {
    it.second = it.second->substituteSingleton(v, t);
  }
  add(v, t);
}

bool Substitution::inDomain(Variable *v) {
  return find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.first == v; }) != this->end();
}

bool Substitution::inRange(Variable *v) {
  return find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.second->hasVariable(v); }) != this->end();
}

Term *Substitution::image(Variable *v) {
  auto ans = find_if(this->begin(), this->end(), [&](const pair<Variable*, Term*> &it) { return it.first == v; });
  if (ans != this->end()) {
    return ans->second;
  }
  throw std::runtime_error("Substitution does not contain needed variable");
}

string Substitution::toString() {
  if (!this->size()) {
    return "( )";
  }
  ostringstream oss;
  oss << "( ";
  auto lastItem = *(this->rbegin());
  for (const auto &it : *this) {
    oss << it.first->name << " |-> " << it.second->toString() << (it == lastItem  ? " )" : "; ");
  }
  return oss.str();
}

Substitution Substitution::unsubstitute(vector<Term *> cts, vector<Variable *> vs)
{
  Substitution result = *this;
  for (int i = 0; i < (int)result.size(); ++i) {
    result[i].second = result[i].second->unsubstitute(cts, vs);
  }
  return result;
}


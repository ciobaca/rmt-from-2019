#include <cassert>
#include "varterm.h"
#include "funterm.h"
#include "log.h"
#include "sort.h"
#include "helper.h"
#include "factories.h"
#include <algorithm>
#include <sstream>

using namespace std;

VarTerm::VarTerm(Variable *variable) :
  variable(variable)
{
  hasDefinedFunctions = false;
  countDefinedFunctions = 0;
  isVarTerm = true;
  isFunTerm = false;
}

vector<Variable *> VarTerm::computeVars()
{
  vector<Variable *> result;
  result.push_back(variable);
  return result;
}

Term *VarTerm::computeSubstitution(Substitution &subst, map<Term *, Term *> &)
{
  if (subst.inDomain(variable)) {
    return subst.image(variable);
  } else {
    return this;
  }
}

Term *VarTerm::computeSingletonSubstitution(Variable *v, Term *t, map<Term *, Term *> &)
{
  if (this->variable == v) {
    return t;
  }
  return this;
}

Term *VarTerm::substitute(Substitution &subst)
{
  if (subst.inDomain(variable)) {
    return subst.image(variable);
  } else {
    return this;
  }
}

Term *VarTerm::substituteSingleton(Variable *v, Term *t)
{
  if (this->variable == v) {
    return t;
  }
  return this;
}

vector<void*> VarTerm::computeVarsAndFresh() {
  vector<void*> result;
  result.push_back((void*)variable);
  return result;
}

void VarTerm::computeToString()
{
  this->stringRepresentation = variable->name;
}

Z3_ast VarTerm::toSmt()
{
  if (variable->sort->hasInterpretation) {
    return variable->interpretation;
  } else {
    abortWithMessage("Trying to SMT an uninterpreted variable (its name is " + variable->name + ").");
    return 0;
  }
}

string VarTerm::toPrettyString()
{
  return variable->name;
}

bool VarTerm::unifyWith(Term *t, Substitution &subst)
{
  Log(DEBUG9) << "VarTerm::unifyWith " << this->toString() << " " << t->toString() << subst.toString() << endl;
  return t->unifyWithVarTerm(this, subst);
}

bool VarTerm::unifyWithVarTerm(VarTerm *t, Substitution &subst)
{
  Log(DEBUG9) << "VarTerm::unifyWithVarTerm " << this->toString() << " " << t->toString() << subst.toString() << endl;
  Log(DEBUG9) << "VarTerm::unifyWithVarTerm, sorts: " << this->variable->sort->name << " " << t->variable->sort->name << endl;
  if (this == t) {
    return true;
  }
  if (subst.inDomain(this->variable)) {
    if (subst.inDomain(t->variable)) {
      return subst.image(this->variable)->unifyWith(subst.image(t->variable), subst);
    } else {
      return subst.image(this->variable)->unifyWithVarTerm(t, subst);
    }
  } else {
    if (subst.inDomain(t->variable)) {
      return subst.image(t->variable)->unifyWithVarTerm(this, subst);
    } else {
      if (this->variable->sort->hasSubSortTR(t->variable->sort)){
        subst.force(this->variable, t);
      } else if (t->variable->sort->hasSubSortTR(this->variable->sort)){
        subst.force(t->variable, this);
      } else {
        // trying to unify two variables of incompatible sorts
        return false;
      }
    }
    return true;
  }
}

bool VarTerm::unifyWithFunTerm(FunTerm *t, Substitution &subst)
{
  Log(DEBUG9) << "VarTerm::unifyWithFunTerm " << this->toString() << " " << t->toString() << subst.toString() << endl;
  vector<Variable *> ocVars = t->vars();

  if (subst.inDomain(this->variable)) {
    return subst.image(this->variable)->unifyWithFunTerm(t, subst);
  } else if (contains(ocVars, this->variable)) {
    return false;
  } else {
    if (this->variable->sort->hasSubSortTR(t->function->result)) {
      subst.force(this->variable, t);
      return true;
    } else {
      return false;
    }
  }
}

VarTerm *VarTerm::getAsVarTerm()
{
  return (VarTerm *)this;
}

FunTerm *VarTerm::getAsFunTerm()
{
  assert(0);
  return 0;
}

bool VarTerm::computeIsNormalized(RewriteSystem &, map<Term *, bool> &)
{
  return true;
}

Term *VarTerm::computeNormalize(RewriteSystem &, map<Term *, Term *> &, bool)
{
  return this;
}

bool VarTerm::computeIsInstanceOf(Term *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  return t->computeIsGeneralizationOf(this, s, cache);
}

bool VarTerm::computeIsGeneralizationOf(VarTerm *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  if (!contains(cache, make_pair((Term *)t, (Term *)this))) {
    if (contains(s, this->variable)) {
      cache[make_pair(t, this)] = s.image(this->variable) == t;
    } else {
      if (this->variable->sort->hasSubSortTR(t->variable->sort)) {
        s.add(this->variable, t);
        cache[make_pair(t, this)] = true;
      } else {
        cache[make_pair(t, this)] = false;
      }
    }
  }
  return cache[make_pair(t, this)];
}

bool VarTerm::computeIsGeneralizationOf(FunTerm *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  if (!contains(cache, make_pair((Term *)t, (Term *)this))) {
    if (contains(s, this->variable)) {
      cache[make_pair(t, this)] = s.image(this->variable) == t;
    } else {
      if (this->variable->sort->hasSubSortTR(t->function->result)) {
        s.add(this->variable, t);
        cache[make_pair(t, this)] = true;
      } else {
        cache[make_pair(t, this)] = false;
      }
    }
  }
  return cache[make_pair(t, this)];
}

int VarTerm::computeDagSize(map<Term *, int> &cache)
{
  if (contains(cache, (Term *)this)) {
    return 0;
  } else {
    cache[this] = 1;
    return 1;
  }
}

Term *VarTerm::rewriteOneStep(RewriteSystem &rewrite, Substitution &how)
{
  return this->rewriteTopMost(rewrite, how);
}

Term *VarTerm::rewriteOneStep(pair<Term *, Term *> rewriteRule, Substitution &how)
{
  return this->rewriteTopMost(rewriteRule, how);
}

Term *VarTerm::rewriteOneStep(ConstrainedRewriteSystem &crs, Substitution &how)
{
  return this->rewriteTopMost(crs, how);
}

Term *VarTerm::rewriteOneStep(pair<ConstrainedTerm, Term *> crewriteRule, Substitution &how)
{
  return this->rewriteTopMost(crewriteRule, how);
}

Term *VarTerm::abstract(Substitution &substitution)
{
  if (variable->sort->hasInterpretation) {
    Variable *var = createFreshVariable(this->variable->sort);
    assert(!substitution.inDomain(var));
    substitution.add(var, this);
    Term *result = getVarTerm(var);
    return result;
  } else {
    return this;
  }
}

vector<ConstrainedSolution> VarTerm::rewriteSearch(RewriteSystem &rs)
{
  vector<ConstrainedSolution> result;
  for (int i = 0; i < len(rs); ++i) {
    pair<Term *, Term *> rewriteRule = rs[i];
    Term *l = rewriteRule.first;
    Term *r = rewriteRule.second;
    
    Substitution subst;
    if (this->isInstanceOf(l, subst)) {
      Term *term = r->substitute(subst);
      result.push_back(ConstrainedSolution(term, subst, l));
    }
  }
  return result;
}

vector<ConstrainedSolution> VarTerm::narrowSearch(ConstrainedRewriteSystem &crs)
{
  Log(DEBUG7) << "VarTerm::narrowSearch (crs) " << this->toString() << endl;
  vector<ConstrainedSolution> result;
  for (int i = 0; i < len(crs); ++i) {
    pair<ConstrainedTerm, Term *> crewriteRule = crs[i];
    ConstrainedTerm l = crewriteRule.first;
    Term *r = crewriteRule.second;

    Substitution subst;
    if (this->unifyWith(l.term, subst)) {
      Term *term = r;
      result.push_back(ConstrainedSolution(term, l.constraint, subst, l.term));
    }
  }
  return result;
}

Sort *VarTerm::getSort()
{
  return this->variable->sort;
}

Term *VarTerm::compute()
{
  return this;
}

void VarTerm::getDefinedFunctions(std::set<Function *> &)
{
  // intentionally empty; no defined functions in a variable term
}

Term *VarTerm::unsubstitute(vector<Term *> &, vector<Variable *> &)
{
  return this;
}

int VarTerm::nrFuncInTerm(Function *f) {
  return 0;
}

Term *VarTerm::toUniformTerm(std::vector<void*> &allVars, map<Variable*, Term*> *subst) {
  int x = distance(allVars.begin(),
    find(allVars.begin(), allVars.end(), (void*)variable));
  Variable *v = getUniformVariable(x, this->getSort());
  if (subst != NULL) (*subst)[v] = this;
  return getVarTerm(v);
}

Term *VarTerm::unsubstituteUnif(map<Variable*, Term*> &subst) {
  assert(subst.count(this->variable));
  return subst[this->variable];
}

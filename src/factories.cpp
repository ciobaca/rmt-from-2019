#include <map>
#include <sstream>
#include <cassert>
#include "sort.h"
#include "funterm.h"
#include "varterm.h"
#include "factories.h"
#include "helper.h"
#include "log.h"

using namespace std;

map<string, RewriteSystem> rewriteSystems;
map<string, ConstrainedRewriteSystem> cRewriteSystems;
map<string, Variable *> variables;
map<string, Sort *> sorts;
map<string, Sort *> builtinSorts;
map<string, Function *> functions;
map<pair<Function *, vector<Term *> >, Term *> funTerms;
map<Variable *, Term *> varTerms;
vector<Variable *> interpretedVariables;

int freshVariableCounter = 0;

// provides a fresh renaming for the set of variables in the "vars" vector
// if a variable appears twice in the vector, it will be be assigned a single new renaming
// the fresh variables are obtained by adding "_" and a fresh number to the
// end of the variable names
map<Variable *, Variable *> freshRenaming(vector<Variable *> vars)
{
  map<Variable *, Variable *> renaming;
  
  for (int i = 0; i < len(vars); ++i) {
    if (contains(renaming, vars[i])) {
      continue;
    }
    ostringstream oss;
    oss << vars[i]->name << "_" << freshVariableCounter;
    string varname = oss.str();
    createVariable(varname, vars[i]->sort);
    renaming[vars[i]] = getVariable(varname);
    Log(DEBUG9) << "Renaming " << vars[i]->name << " to " << getVariable(varname)->name << endl;
  }
  freshVariableCounter++;

  return renaming;
}

RewriteSystem &getRewriteSystem(string name)
{
  if (!existsRewriteSystem(name)) {
    Log(ERROR) << "Cannot find rewrite system " << name << endl;
    assert(0);
  }
  return rewriteSystems[name];
}

bool existsRewriteSystem(string name)
{
  return rewriteSystems.find(name) != rewriteSystems.end();
}

void putRewriteSystem(string name, RewriteSystem rewrite)
{
  rewriteSystems[name] = rewrite;
}

ConstrainedRewriteSystem &getConstrainedRewriteSystem(string name)
{
  return cRewriteSystems[name];
}

bool existsConstrainedRewriteSystem(string name)
{
  return cRewriteSystems.find(name) != cRewriteSystems.end();
}

void putConstrainedRewriteSystem(string name, ConstrainedRewriteSystem crewrite)
{
  cRewriteSystems[name] = crewrite;
}

Variable *getVariable(string name)
{
  if (contains(variables, name)) {
    return variables[name];
  } else {
    return 0;
  }
}

Variable *createVariable(string name, Sort *sort)
{
#ifndef NDEBUG
  Variable *v = getVariable(name);
  assert(!v);
#endif
  variables[name] = new Variable(name, sort);
  if (sort->hasInterpretation)
    interpretedVariables.push_back(variables[name]);
  return variables[name];
}

bool variableExists(string name)
{
  return contains(variables, name);
}

Sort *getSort(string name)
{
  if (contains(sorts, name)) {
    return sorts[name];
  } else {
    return 0;
  }
}

Sort *getIntSort()
{
  return getSort("Int");
}

Sort *getBoolSort()
{
  return getSort("Bool");
}

Sort *getBuiltinSort(Z3_string sortName) {
  if (builtinSorts.count(sortName)) {
    return builtinSorts[sortName];
  }
  else {
    return 0;
  }
}

void createUninterpretedSort(const string &sortName)
{
#ifndef NDEBUG
  Sort *s = getSort(sortName);
  assert(!s);
#endif
  
  sorts[sortName] = new Sort(sortName);
  Log(INFO) << "Creating uninterpreted sort " << sortName << "." << endl;
}

void createInterpretedSort(const string &sortName, const string &interpretation)
{
#ifndef NDEBUG
  Sort *sold = getSort(sortName);
  assert(!sold);
#endif
  
  Sort *s = new Sort(sortName, interpretation);
  sorts[sortName] = s;
  builtinSorts[z3_sort_to_string(s->interpretation)] = s;
  Log(INFO) << "Creating interpreted sort " << sortName << " as (" << interpretation << ")." << endl;
}

bool sortExists(string name)
{
  return contains(sorts, name);
}

bool isBuiltinSort(string name)
{
  assert(sortExists(name));
  return sorts[name]->hasInterpretation;
}

Variable *getInternalVariable(string name, Sort *sort)
{
  if (!getVariable(name)) {
    createVariable(name, sort);
  }
  assert(getVariable(name)->sort == sort);
  return getVariable(name);
}

static int number = 0;

Variable *createFreshVariable(Sort *sort)
{
  ostringstream oss;
  oss << "_" << number++;
  string freshName = oss.str();
  createVariable(freshName, sort);
  return getVariable(freshName);
}

Function *getFreshConstant(Sort *sort)
{
  ostringstream oss;
  oss << "_" << number++;
  string freshName = oss.str();
  z3_fresh *fresh = NULL;
  if (sort->hasInterpretation) {
    fresh = new z3_fresh(getIntSort());
    createInterpretedFunction(freshName, vector<Sort *>(), sort, fresh);
  } else {
    createUninterpretedFunction(freshName, vector<Sort *>(), sort, false, false, 0);
  }
  if (contains(functions, freshName)) {
    fresh->setTerm(getFunTerm(functions[freshName], vector0()));
    functions[freshName]->isFresh = true;
    return functions[freshName];
  } else {
    assert(0);
    return 0;
  }
}

Function *getFunction(string name)
{
  if (contains(functions, name)) {
    return functions[name];
  } else {
    return 0;
  }
}

Function *getMinusFunction()
{
  return getFunction("mminus");
}

Function *getLEFunction()
{
  return getFunction("mle");
}

Term *getIntOneConstant() {
  return getFunTerm(getFunction("1"), vector<Term*>());
}

Term *getIntZeroConstant() {
  return getFunTerm(getFunction("0"), vector<Term*>());
}

void updateDefinedFunction(string name, ConstrainedRewriteSystem &crewrite)
{
  Function *f = getFunction(name);
  assert(f != 0);
  f->updateDefined(crewrite);
}

void createUninterpretedFunction(string name, vector<Sort *> arguments, Sort *result, bool isCommutative, bool isAssociative, Function *unityElement)
{
#ifndef NDEBUG
  Function *f = getFunction(name);
  assert(f == 0);
#endif
  int arity = arguments.size();
  Log log(INFO);
  log << "Creating uninterpreted function " << name << " : ";
  for (int i = 0; i < arity; ++i) {
    log << arguments[i]->name << " ";
  }
  log << " -> " << result->name << endl;
  functions[name] = new Function(name, arguments, result, isCommutative, isAssociative, unityElement);
}

void createInterpretedFunction(string name, vector<Sort *> arguments, Sort *result, string interpretation)
{
#ifndef NDEBUG
  Function *fold = getFunction(name);
  assert(fold == 0);
  // ma asigura ca nu exista deja o functie cu acelasi nume
#endif
  int arity = arguments.size();
  Log log(INFO);
  log << "Creating interpreted function " << name << " (as " << interpretation << ") : ";
  for (int i = 0; i < arity; ++i) {
    log << arguments[i]->name << " ";
  }
  log << " -> " << result->name << endl;
  assert(interpretation != "");

  Function *f = new Function(name, arguments, result, interpretation);
  functions[name] = f;
  
  //handling special functions
  if (interpretation == "store") {
    if (arguments.size() != 3) {
      Log(ERROR) << "Wrong number of arguments for store function" << endl;
      assert(0);
    }
    (*getStoreFunctions())[make_pair(arguments[0], make_pair(arguments[1], arguments[2]))] = f;
  }
  else if (interpretation == "select") {
    if (arguments.size() != 2) {
      Log(ERROR) << "Wrong number of arguments for select function" << endl;
      assert(0);
    }
    (*getSelectFunctions())[make_pair(result, make_pair(arguments[0], arguments[1]))] = f;
  }
}

void createInterpretedFunction(string name, vector<Sort *> arguments, Sort *result, Z3_func_decl interpretation)
{
#ifndef NDEBUG
  Function *f = getFunction(name);
  assert(f == 0);
#endif
  int arity = arguments.size();
  Log log(INFO);
  log << "Creating interpreted function " << name << " (as " << interpretation << ") : ";
  for (int i = 0; i < arity; ++i) {
    log << arguments[i]->name << " ";
  }
  log << " -> " << result->name << endl;
  functions[name] = new Function(name, arguments, result, interpretation);
}

void createInterpretedFunction(string name, vector<Sort *> arguments, Sort *result, Z3Function *interpretation)
{
#ifndef NDEBUG
  Function *f = getFunction(name);
  assert(f == 0);
#endif
  int arity = arguments.size();
  Log log(INFO);
  log << "Creating interpreted function " << name << ": ";
  for (int i = 0; i < arity; ++i) {
    log << arguments[i]->name << " ";
  }
  log << " -> " << result->name << endl;
  functions[name] = new Function(name, arguments, result, interpretation);
}

Term *getFunTerm(Function *f, vector<Term *> arguments)
{
  pair<Function *, vector<Term *> > content = make_pair(f, arguments);
  if (contains(funTerms, content)) {
    return funTerms[content];
  } else {
    return funTerms[content] = new FunTerm(f, arguments);
  }
}

Term *getVarTerm(Variable *v)
{
  if (contains(varTerms, v)) {
    return varTerms[v];
  } else {
    return varTerms[v] = new VarTerm(v);
  }
}

vector<Variable *> &getInterpretedVariables()
{
  return interpretedVariables;
}

vector<Term *> vector0()
{
  vector<Term *> result;
  return result;
}

vector<Term *> vector1(Term *term)
{
  vector<Term *> result;
  result.push_back(term);
  return result;
}

vector<Term *> vector2(Term *term1, Term *term2)
{
  vector<Term *> result;
  result.push_back(term1);
  result.push_back(term2);
  return result;
}

vector<Term *> vector3(Term *term1, Term *term2, Term *term3)
{
  vector<Term *> result;
  result.push_back(term1);
  result.push_back(term2);
  result.push_back(term3);
  return result;
}

Function *TrueFun;
Function *FalseFun;
Function *NotFun;
Function *AndFun;
Function *ImpliesFun;
Function *OrFun;
Function *EqualsFun;
Function *MleFun;

Function *MEqualsFun;

map< pair< Sort*, pair<Sort*, Sort*> >, Function* > SelectFun;
map< pair< Sort*, pair<Sort*, Sort*> >, Function* > StoreFun;
map< pair< Sort*, pair<Sort*, Sort*> >, Function* > *getSelectFunctions() {
  return &SelectFun;
}
map< pair< Sort*, pair<Sort*, Sort*> >, Function* > *getStoreFunctions() {
  return &StoreFun;
}

map<Sort *, Function *> ExistsFun;
map<Sort *, Function *> ForallFun;

// small hacks for existential and universal quantifiers

void createBuiltinExistsFunction(Sort *s) {
  vector<Sort *> args;
  args.push_back(s);
  args.push_back(sorts["Bool"]);
  ostringstream funname;
  funname << "_exists" << s->name;
  Log(DEBUG) << "Creating exists function " << funname.str() << endl;
  createUninterpretedFunction(funname.str(), args, sorts["Bool"]);
  ExistsFun[s] = getFunction(funname.str());
  assert(ExistsFun[s]);
}

void createBuiltinForallFunction(Sort *s) {
  vector<Sort *> args;
  args.push_back(s);
  args.push_back(sorts["Bool"]);
  ostringstream funname;
  funname << "_forall" << s->name;
  Log(DEBUG) << "Creating forall function " << funname.str() << endl;
  createUninterpretedFunction(funname.str(), args, sorts["Bool"]);
  ForallFun[s] = getFunction(funname.str());
  assert(ForallFun[s]);
}

void createBuiltIns()
{
  TrueFun = getFunction("true");
  assert(TrueFun);
  FalseFun = getFunction("false");
  assert(FalseFun);
  NotFun = getFunction("bnot");
  assert(NotFun);
  AndFun = getFunction("band");
  assert(AndFun);
  ImpliesFun = getFunction("bimplies");
  assert(ImpliesFun);
  OrFun = getFunction("bor");
  assert(OrFun);
  EqualsFun = getFunction("bequals");
  assert(EqualsFun);
  MEqualsFun = getFunction("mequals");
  assert(MEqualsFun);
  MleFun = getFunction("mle");
  assert(MleFun);

  assert(sortExists("Bool"));
  assert(sortExists("Int"));

  Log(DEBUG) << "Creating built ins" << endl;
  for (map<string, Sort *>::iterator it = sorts.begin(); it != sorts.end(); ++it) {
    createBuiltinExistsFunction(it->second);
  }
  
  for (map<string, Sort *>::iterator it = sorts.begin(); it != sorts.end(); ++it) {
    createBuiltinForallFunction(it->second);
  }
}

bool isExistsFunction(Function *f)
{
  for (map<Sort *, Function *>::iterator it = ExistsFun.begin(); it != ExistsFun.end(); ++it) {
    if (it->second == f) {
      return true;
    }
  }
  return false;
}

bool isForallFunction(Function *f)
{
  for (map<Sort *, Function *>::iterator it = ForallFun.begin(); it != ForallFun.end(); ++it) {
    if (it->second == f) {
      return true;
    }
  }
  return false;
}

bool isQuantifierFunction(Function *f)
{
  return isExistsFunction(f) || isForallFunction(f);
}

Term *bExists(Variable *var, Term *condition)
{
  assert(ExistsFun[var->sort]);
  Term *result = getFunTerm(ExistsFun[var->sort], vector2(getVarTerm(var), condition));
  return result;
}

Term *bForall(Variable *var, Term *condition)
{
  assert(ForallFun[var->sort]);
  Term *result = getFunTerm(ForallFun[var->sort], vector2(getVarTerm(var), condition));
  return result;
}

Term *bImplies(Term *left, Term *right)
{
  return getFunTerm(ImpliesFun, vector2(left, right));
}

Term *bAnd(Term *left, Term *right)
{
  return getFunTerm(AndFun, vector2(left, right));
}

Term *bAndVector(std::vector<Term *> args, int start)
{
  assert(0 <= start && start < static_cast<int>(args.size()));
  if (start == static_cast<int>(args.size()) - 1) {
    return args[start];
  } else {
    return bAnd(args[start], bAndVector(args, start + 1));
  }
}

Term *bOrVector(std::vector<Term *> args, int start)
{
  assert(0 <= start && start < static_cast<int>(args.size()));
  if (start == static_cast<int>(args.size()) - 1) {
    return args[start];
  }
  else {
    return bOr(args[start], bOrVector(args, start + 1));
  }
}

Term *bOr(Term *left, Term *right)
{
  return getFunTerm(OrFun, vector2(left, right));
}

Term *bNot(Term *term)
{
  return getFunTerm(NotFun, vector1(term));
}

Term *bTrue()
{
  return getFunTerm(TrueFun, vector0());
}

Term *bFalse()
{
  return getFunTerm(FalseFun, vector0());
}

Term *mEquals(Term *left, Term *right)
{
  return getFunTerm(MEqualsFun, vector2(left, right));
}

Term *mle(Term *left, Term *right)
{
  return getFunTerm(MleFun, vector2(left, right));
}

Term *mplus(Term *left, Term *right)
{
  return getFunTerm(getFunction("mplus"), vector2(left, right));
}

Term *mPlusVector(std::vector<Term *> args, int start)
{
  if (start == static_cast<int>(args.size() - 1)) {
    return args[start];
  } else {
    return mplus(args[start], mPlusVector(args, start + 1));
  }
}

Term *mminus(Term *left, Term *right)
{
  return getFunTerm(getFunction("mminus"), vector2(left, right));
}

Term *mdiv(Term *left, Term *right)
{
  return getFunTerm(getFunction("mdiv"), vector2(left, right));
}

Term *mmod(Term *left, Term *right)
{
  return getFunTerm(getFunction("mmod"), vector2(left, right));
}

Term *mtimes(Term *left, Term *right)
{
  return getFunTerm(getFunction("mtimes"), vector2(left, right));
}

Term *mTimesVector(std::vector<Term *> args, int start)
{
  if (start == static_cast<int>(args.size() - 1)) {
    return args[start];
  } else {
    return mtimes(args[start], mTimesVector(args, start + 1));
  }
}

Term *bEquals(Term *left, Term *right)
{
  return getFunTerm(EqualsFun, vector2(left, right));
}

Term *simplifyConstraint(Term *constraint)
{
  Log(DEBUG) << "Simplifying constraint  " << constraint->toString() << "." << endl;
  assert(constraint->getSort() == getBoolSort());
  Term *result = simplifyTerm(constraint);
  Log(DEBUG) << "Simplified constraint = " << result->toString() << "." << endl;
  return result;;
}

Term *simplifyTerm_helper(Term *term)
{
  if (term->isFunTerm) {
    FunTerm *funterm = term->getAsFunTerm();
    Function *function = funterm->function;
    if (function->hasInterpretation) {
      Log(DEBUG) << "Simplifying builtin " << term->toString() << "." << endl;
      Z3_ast result = z3_simplify(term);
      Term *termResult = unZ3(result, function->result);
      Log(DEBUG) << "Simplified builtin = " << termResult->toString() << "." << endl;
      return termResult;
    } else {
      vector<Term *> arguments = funterm->arguments;
      for (int i = 0; i < static_cast<int>(arguments.size()); ++i) {
        arguments[i] = simplifyTerm(arguments[i]);
      }
      return getFunTerm(function, arguments);
    }
  } else {
    assert(term->isVarTerm);
    return term;
  }
}

map<Term*, Term*> simplifyCache;

Term *simplifyTerm(Term *term) {
  vector<void*> usedVars = term->varsAndFresh();
  map<Variable*, Term*> subst;
  Term *unifTerm = term->toUniformTerm(usedVars, &subst);
  if (!simplifyCache.count(unifTerm))
    simplifyCache[unifTerm] = simplifyTerm_helper(unifTerm);
  return simplifyCache[unifTerm]->unsubstituteUnif(subst);
}

ConstrainedTerm simplifyConstrainedTerm(ConstrainedTerm ct)
{
  return ConstrainedTerm(simplifyTerm(ct.term), simplifyTerm(ct.constraint));
}

Function *getEqualsFunction(Sort *sort)
{
  for (map<string, Function *>::iterator it = functions.begin(); it != functions.end(); ++it) {
    Function *f = it->second;
    if (f->arguments.size() != 2) {
      continue;
    }
    if (f->arguments[0] != sort) {
      continue;
    }
    if (f->arguments[1] != sort) {
      continue;
    }
    if (!f->hasInterpretation) {
      continue;
    }
    if (!f->isEqualityFunction) {
      continue;
    }
    return f;
  }
  return 0;
}

Term *createEqualityConstraint(Term *t1, Term *t2)
{
  assert(t1);
  assert(t2);
  Log(DEBUG2) << "Creating equality constraint between " << t1->toString() << " and " << t2->toString() << endl;
  Sort *s1 = t1->getSort();
  Sort *s2 = t2->getSort();
  if (s1 != s2) {
    assert(0);
  }
  Log(DEBUG2) << "Equality sort is " << s1->name << endl;
  Function *equalsFun = getEqualsFunction(s1);
  if (!equalsFun) {
    Log(ERROR) << "Cannot find equality function symbol for sort " << s1->name << endl;
    assert(0);
  }
  Term *result = getFunTerm(equalsFun, vector2(t1, t2));
  Log(DEBUG2) << "Equality constraint is " << result->toString() << "." << endl;
  return result;
}

Term *introduceExists(Term *constraint, vector<Variable *> vars)
{
  for (int i = 0; i < (int)vars.size(); ++i) {
    if (vars[i]->sort->hasInterpretation && constraint->hasVariable(vars[i])) {
      constraint = bExists(vars[i], constraint);
    }
  }
  return constraint;
}

vector<Function*> getDefinedFunctions() {
  vector<Function*> ans;
  for (const auto &it : functions)
    if (it.second->isDefined) ans.push_back(it.second);
  return ans;
}

ConstrainedRewriteSystem getDefinedFunctionsSystem(vector<Function *> definedFunctions) {
  ConstrainedRewriteSystem crsFinal;

  for (int i = 0; i < static_cast<int>(definedFunctions.size()); ++i) {
    Function *f = definedFunctions[i];
    if (!f) {
      abortWithMessage(string("Function ") + f->name + " not found.");
    }
    if (!f->isDefined) {
      abortWithMessage(string("Function ") + f->name + " is not a defined function.");
    }
    ConstrainedRewriteSystem crs = f->crewrite;
    for (int j = 0; j < static_cast<int>(crs.size()); ++j) {
      crsFinal.addRule(crs[j].first, crs[j].second);
    }
  }

  return crsFinal;
}

Variable *getOrCreateVariable(std::string name, Sort *s) {
  Variable *res = getVariable(name);
  if (res == NULL)
    res = createVariable(name, s);
  assert(res->sort == s);
  return res;
}

Variable *getUniformVariable(int varIdx, Sort *s) {
  ostringstream ss;
  ss << "_$_" << s->name << "_";
  ss << varIdx;
  return getOrCreateVariable(ss.str(), s);
}

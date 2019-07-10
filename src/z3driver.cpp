#include <cstdlib>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>
#include "z3driver.h"
#include "variable.h"
#include "log.h"
#include "term.h"
#include "sort.h"
#include "factories.h"
#include "helper.h"
#include <string>
#include <map>

#include <z3.h>

Z3_context z3context;

Z3_sort z3BoolSort;
Z3_sort z3IntSort;

using namespace std;

unsigned z3_symbol_count = 0;

map<Z3_symbol, Variable *> z3_const_to_var;
map<Z3_symbol, Term *> z3_const_to_const;
map<Z3_symbol, Z3_func_decl> symbol_to_func_decl;
map<Z3_func_decl, Function *> func_decl_to_function;
vector<Z3_ast> z3asserts;
Z3_params simplifyParams;
Z3_tactic simplifyTactic;
Z3_params tacticParams;

Z3_string z3_sort_to_string(Z3_sort s) {
  return Z3_sort_to_string(z3context, s);
}

Z3_ast z3_make_constant(Variable *variable)
{
  Z3_symbol symbol = Z3_mk_int_symbol(z3context, z3_symbol_count++);
  Z3_ast result = Z3_mk_const(z3context, symbol, variable->sort->interpretation);
  z3_const_to_var[symbol] = variable;
  return result;
}

Z3_ast z3_simplify(Term *term)
{
  Z3_ast toSimplify = term->toSmt();
  if (term->getSort() == getBoolSort()) {
    Z3_goal goal = Z3_mk_goal(z3context, false, false, false);
    Z3_goal_inc_ref(z3context, goal);
    Z3_goal_assert(z3context, goal, toSimplify);
    Z3_apply_result res = Z3_tactic_apply_ex(z3context, simplifyTactic, goal, tacticParams);
    Z3_apply_result_inc_ref(z3context, res);
    vector<Z3_ast> clauses;
    int nrgoals = Z3_apply_result_get_num_subgoals(z3context, res);
    for (int i = 0; i < nrgoals; ++i) {
      Z3_goal g = Z3_apply_result_get_subgoal(z3context, res, i);
      Z3_goal_inc_ref(z3context, g);
      int sz = Z3_goal_size(z3context, g);
      for (int j = 0; j < sz; ++j)
        clauses.push_back(Z3_goal_formula(z3context, g, j));
      Z3_goal_dec_ref(z3context, g);
    }
    Z3_ast result = clauses.empty() ? Z3_mk_true(z3context) : (
      (clauses.size() == 1) ? clauses[0] :
      Z3_mk_and(z3context, clauses.size(), &clauses[0])
      );
    toSimplify = result;
    Z3_goal_dec_ref(z3context, goal);
    Z3_apply_result_dec_ref(z3context, res);
  }
  return Z3_simplify_ex(z3context, toSimplify, simplifyParams);
}

Z3_sort z3_bool()
{
  return z3BoolSort;
}

Z3_sort z3_int()
{
  return z3IntSort;
}

Z3_sort z3_uninterpreted_sort(string sortName) {
  return Z3_mk_uninterpreted_sort(z3context,
    Z3_mk_string_symbol(z3context, sortName.c_str()));
}

Z3_sort z3_array(string);

string extractSortName(string fullString, int from, int to) {
  int brackets = 0;
  for (int i = from; i < to; ++i) {
    if (fullString[i] == '(') {
      ++brackets;
      continue;
    }
    if (fullString[i] == ')') {
      --brackets;
      if (brackets < 0) {
        abortWithMessage("Unable to parse sort " + fullString);
      }
      continue;
    }
    if (fullString[i] == ' ' && brackets == 0) {
      return fullString.substr(from, i - from);
    }
  }
  if (brackets != 0) {
    abortWithMessage("Unable to parse sort " + fullString);
  }
  return fullString.substr(from, to - from);
}

Z3_sort z3_getInterpretation(string i) {
  if (i == "Bool") {
    return z3_bool();
  }
  else if (i == "Int") {
    return z3_int();
  }
  else if (i.front() == '(' && i.back() == ')' &&
    i.find("Array") == 1) {
    return z3_array(i);
  }
  else {
    return z3_uninterpreted_sort(i);
  }
}

Z3_sort z3_array(string i) {
  string domainName = extractSortName(i, 2 + string("Array").size(), i.size() - 1);
  string rangeName = extractSortName(i, 3 + string("Array").size() + domainName.size(), i.size() - 1);
  Log(INFO) << "Creating Array sort with domain " << domainName << " and range " << rangeName << endl;
  return Z3_mk_array_sort(z3context, z3_getInterpretation(domainName), z3_getInterpretation(rangeName));
}

Z3_ast z3_add::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  Z3_ast z3args[2];
  z3args[0] = args[0]->toSmt();
  z3args[1] = args[1]->toSmt();
  return Z3_mk_add(z3context, 2, z3args);
}

Z3_ast z3_mul::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  Z3_ast z3args[2];
  z3args[0] = args[0]->toSmt();
  z3args[1] = args[1]->toSmt();
  return Z3_mk_mul(z3context, 2, z3args);
}

Z3_ast z3_sub::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  Z3_ast z3args[2];
  z3args[0] = args[0]->toSmt();
  z3args[1] = args[1]->toSmt();
  return Z3_mk_sub(z3context, 2, z3args);
}

Z3_ast z3_div::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_div(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_mod::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_mod(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_le::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_le(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_lt::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_lt(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_ge::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_ge(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_gt::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_gt(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_eq::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_eq(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_ct_0::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 0, z3IntSort);
}

Z3_ast z3_ct_1::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 1, z3IntSort);
}

Z3_ast z3_ct_2::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 2, z3IntSort);
}

Z3_ast z3_ct_3::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 3, z3IntSort);
}

Z3_ast z3_ct_4::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 4, z3IntSort);
}

Z3_ast z3_ct_5::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 5, z3IntSort);
}

Z3_ast z3_ct_6::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 6, z3IntSort);
}

Z3_ast z3_ct_7::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 7, z3IntSort);
}

Z3_ast z3_ct_8::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 8, z3IntSort);
}

Z3_ast z3_ct_9::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 9, z3IntSort);
}

Z3_ast z3_ct_10::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 10, z3IntSort);
}

Z3_ast z3_ct_11::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 11, z3IntSort);
}

Z3_ast z3_ct_12::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 12, z3IntSort);
}

Z3_ast z3_ct_13::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 13, z3IntSort);
}

Z3_ast z3_ct_14::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 14, z3IntSort);
}

Z3_ast z3_ct_15::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, 15, z3IntSort);
}

Z3_ast z3_ct::operator()(vector<Term *>)
{
  return Z3_mk_int(z3context, num, z3IntSort);
}

Z3_ast z3_true::operator()(vector<Term *>)
{
  return Z3_mk_true(z3context);
}

Z3_ast z3_false::operator()(vector<Term *>)
{
  return Z3_mk_false(z3context);
}


Z3_ast z3_not::operator()(vector<Term *> args)
{
  assert(args.size() == 1);
  return Z3_mk_not(z3context, args[0]->toSmt());
}

Z3_ast z3_and::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  Z3_ast z3args[2];
  z3args[0] = args[0]->toSmt();
  z3args[1] = args[1]->toSmt();
  return Z3_mk_and(z3context, 2, z3args);
}

Z3_ast z3_or::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  Z3_ast z3args[2];
  z3args[0] = args[0]->toSmt();
  z3args[1] = args[1]->toSmt();
  return Z3_mk_or(z3context, 2, z3args);
}

Z3_ast z3_iff::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_iff(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_implies::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_implies(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_ite::operator()(vector<Term *> args)
{
  assert(args.size() == 3);
  return Z3_mk_ite(z3context, args[0]->toSmt(), args[1]->toSmt(), args[2]->toSmt());
}

Z3_ast z3_select::operator()(vector<Term *> args)
{
  assert(args.size() == 2);
  return Z3_mk_select(z3context, args[0]->toSmt(), args[1]->toSmt());
}

Z3_ast z3_store::operator()(vector<Term *> args)
{
  assert(args.size() == 3);
  return Z3_mk_store(z3context, args[0]->toSmt(), args[1]->toSmt(), args[2]->toSmt());
}

z3_custom_func::z3_custom_func(Z3_func_decl func, Function *fun)
{
  this->func = func;
  func_decl_to_function[func] = fun;
}

Z3_ast z3_custom_func::operator()(vector<Term *> args)
{
  Z3_ast domain[16];
  unsigned size = args.size();
  assert(size < 16);
  for (int i = 0; i < static_cast<int>(size); ++i) {
    domain[i] = args[i]->toSmt();
  }

  return Z3_mk_app(z3context, func, size, domain);
}

void z3_error_handler(Z3_context context, Z3_error_code error)
{
  Z3_string string_error = Z3_get_error_msg(context, error);
  abortWithMessage(string("Z3 returned non OK error code (") + string_error + ").");
}

void start_z3_api()
{
  Z3_config z3config = Z3_mk_config();
  Z3_set_param_value(z3config, "timeout", "2000");
  Z3_set_param_value(z3config, "auto_config", "true");
  z3context = Z3_mk_context(z3config);
  Z3_set_error_handler(z3context, z3_error_handler);

  simplifyParams = Z3_mk_params(z3context);
  Z3_params_inc_ref(z3context, simplifyParams);
  Z3_params_set_bool(z3context, simplifyParams, Z3_mk_string_symbol(z3context, "sort_store"), true);

  tacticParams = Z3_mk_params(z3context);
  Z3_params_inc_ref(z3context, tacticParams);
  // Z3_params_set_uint(z3context, tacticParams, Z3_mk_string_symbol(z3context, "timeout"), 2000);

  simplifyTactic = Z3_mk_tactic(z3context, "ctx-solver-simplify");
  Z3_tactic_inc_ref(z3context, simplifyTactic);
  //simplifyTactic = Z3_tactic_using_params(z3context, simplifyTactic, simplifyParams);


  z3BoolSort = Z3_mk_bool_sort(z3context);
  z3IntSort = Z3_mk_int_sort(z3context);

  // Z3_ast z3True = Z3_mk_true(z3context);
  // Z3_ast z3False = Z3_mk_false(z3context);

  // Z3_solver z3solver = Z3_mk_solver(z3context);
  // Z3_solver_assert(z3context, z3solver, z3True);
  // if (Z3_solver_check(z3context, z3solver) != Z3_L_TRUE) {
  //   abortWithMessage("Z3 said true is unsatisfiable or undef.");
  // }
  // Z3_solver_assert(z3context, z3solver, z3False);
  // if (Z3_solver_check(z3context, z3solver) != Z3_L_FALSE) {
  //   abortWithMessage("Z3 said true /\\ false is satisfiable or undef.");
  // }
}

void test_z3_api()
{
  Z3_solver z3solver = Z3_mk_solver(z3context);
  Term *N = getVarTerm(createFreshVariable(getSort("Int")));
  vector<Term *> empty;
  Term *One = getFunTerm(getFunction("0"), empty);
  //  Term *B = getVarTerm(createFreshVariable(getSort("Bool")));
  Term *testTerm = bAnd(bTrue(), mle(One, N));
  Log(DEBUG7) << "Asserting " << testTerm->toString() << "." << endl;
  Z3_ast ast = testTerm->toSmt();
  Log(DEBUG7) << "Z3 Speak: asserting " << Z3_ast_to_string(z3context, ast) << "." << endl;
  Z3_solver_assert(z3context, z3solver, testTerm->toSmt());
  switch (Z3_solver_check(z3context, z3solver)) {
  case Z3_L_TRUE:
    cout << "satisfiable" << endl;
    break;
  case Z3_L_FALSE:
    cout << "unsatisfiable" << endl;
    break;
  case Z3_L_UNDEF:
    cout << "unknown" << endl;
    break;
  }
}

void Z3Theory::addVariable(Variable *var)
{
  variables.push_back(var);
}

void Z3Theory::addEqualityConstraint(Term *left, Term *right)
{
  Function *EqualsFun = getFunction("bequals");
  vector<Term *> sides;
  sides.push_back(left);
  sides.push_back(right);
  addConstraint(getFunTerm(EqualsFun, sides));
}

void Z3Theory::addConstraint(Term *constraint)
{
  constraints.push_back(constraint);
}

map<Term*, Z3Result> knownResults;

Term *Z3Theory::toNormTerm() {
  Term *t = bTrue();
  for (auto &it : this->constraints) {
    vector<void*> usedVars = it->varsAndFresh();
    t = bAnd(t, it->toUniformTerm(usedVars));
  }
  return t;
}

Z3Result Z3Theory::isSatisfiableHelper() {
  Z3_solver z3solver = Z3_mk_solver(z3context);
  for (int i = 0; i < static_cast<int>(z3asserts.size()); ++i) {
    Log(DEBUG7) << "Asserting (from prelude) " << Z3_ast_to_string(z3context, z3asserts[i]) << "." << endl;
    Z3_solver_assert(z3context, z3solver, z3asserts[i]);
  }
  for (int i = 0; i < static_cast<int>(constraints.size()); ++i) {
    Z3_ast constraint = constraints[i]->toSmt();
    Log(DEBUG7) << "Asserting " << constraints[i]->toString() << "." << endl;
    Log(DEBUG7) << "Z3 Speak: asserting " << Z3_ast_to_string(z3context, constraint) << "." << endl;
    Z3_solver_assert(z3context, z3solver, constraint);
  }
  Log(DEBUG7) << "Calling solver." << endl;
  Z3_lbool result = Z3_solver_check(z3context, z3solver);
  if (result == Z3_L_TRUE) {
    Log(LOGSAT) << "Result is SAT" << endl;
    return sat;
  }
  else if (result == Z3_L_FALSE) {
    Log(LOGSAT) << "Result is UNSAT" << endl;
    return unsat;
  }
  else if (result == Z3_L_UNDEF) {
    Log(LOGSAT) << "Result is UNKNOWN (or timeout)" << endl;
    Log(WARNING) << "Result is UNKNOWN (or timeout)" << endl;
    return unknown;
  }
  else {
    assert(0);
    return unknown;
  }
}

Z3Result Z3Theory::isSatisfiable()
{
  Term *key = this->toNormTerm();
  Log(INFO) << "Computing result for " << key << endl;

  if (!knownResults.count(key)) {
    knownResults[key] = this->isSatisfiableHelper();
  }
  else {
    Log(INFO) << "Result is already known for " << key << endl;
  }
  return knownResults[key];
}

// Z3Result Z3Theory::isSatisfiable()
// {
//   ostringstream oss;
//   for (int i = 0; i < (int)variables.size(); ++i) {
//     assert(variables[i]->sort->hasInterpretation);
//     oss << "(declare-const " << variables[i]->name << " " << variables[i]->sort->interpretation << ")" << endl;
//   }
//   for (int i = 0; i < (int)constraints.size(); ++i) {
//     oss << "(assert " << constraints[i]->toSmtString() << ")" << endl;
//   }
//   oss << "(check-sat)" << endl;
//   string z3string = oss.str();
//   Log(LOGSAT) << "Sending the following to Z3:" << endl << z3string;
//   string result = callz3(z3string);
//   if (result == "sat") {
//     Log(LOGSAT) << "Result is SAT" << endl;
//     return sat;
//   } else if (result == "unsat") {
//     Log(LOGSAT) << "Result is UNSAT" << endl;
//     return unsat;
//   } else if (result == "unknown") {
//     Log(LOGSAT) << "Result is UNKNOWN" << endl;
//     Log(WARNING) << "Result is UNKNOWN" << endl;
//     return unknown;
//   } else if (result == "timeout") {
//     Log(LOGSAT) << "Result is timeout" << endl;
//     Log(WARNING) << "Result is timeout" << endl;
//     return unknown;
//   } else {
//     Log(ERROR) << "Internal error - Z3 did not return an expected satisfiability value." << endl << z3string;
//     Log(ERROR) << "Tried to smt the following constraints:" << endl;
//     for (int i = 0; i < (int)constraints.size(); ++i) {
//       Log(ERROR) << constraints[i]->toString() << endl;
//     }
//     fprintf(stderr, "Cannot interpret result returned by Z3: \"%s\".\n", result.c_str());
//     assert(0);
//     return unknown;
//   }
// }

Z3Result isSatisfiable(Term *constraint)
{
  Log(LOGSAT) << "Testing satisfiability of " << constraint->toString() << endl;
  Z3Theory theory;
  vector<Variable *> interpretedVariables = getInterpretedVariables();
  for (int i = 0; i < (int)interpretedVariables.size(); ++i) {
    theory.addVariable(interpretedVariables[i]);
  }
  theory.addConstraint(constraint);
  return theory.isSatisfiable();
}

bool isValid(Term *constraint) {
  return isSatisfiable(bNot(constraint)) == unsat;
}

Term *unZ3(Z3_ast ast, Sort *sort, vector<Variable *> boundVars)
{
  Log(DEBUG8) << "UnZ3-ing " << Z3_ast_to_string(z3context, ast) << "." << endl;
  switch (Z3_get_ast_kind(z3context, ast)) {
  case Z3_APP_AST:
  {
    Log(DEBUG8) << "UnZ3: entering Z3_APP_AST." << endl;
    Z3_app app = Z3_to_app(z3context, ast);
    Z3_func_decl func_decl = Z3_get_app_decl(z3context, app);
    switch (Z3_get_decl_kind(z3context, func_decl)) {
    case Z3_OP_UNINTERPRETED:
    {
      Log(DEBUG8) << "UnZ3: entering Z3_OP_UNINTERPRETED." << endl;
      Z3_symbol symbol = Z3_get_decl_name(z3context, func_decl);
      if (z3_const_to_var.find(symbol) != z3_const_to_var.end()) {
        Term *resultUnZ3 = getVarTerm(z3_const_to_var[symbol]);
        Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
        return resultUnZ3;
      }
      else if (z3_const_to_const.find(symbol) != z3_const_to_const.end()) {
        Term *resultUnZ3 = z3_const_to_const[symbol];
        Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
        return resultUnZ3;
      }
      else {
        assert(symbol_to_func_decl.find(symbol) != symbol_to_func_decl.end());
        Z3_func_decl func = symbol_to_func_decl[symbol];
        assert(func_decl_to_function.find(func) != func_decl_to_function.end());
        Function *function = func_decl_to_function[func];
        unsigned size = Z3_get_app_num_args(z3context, app);
        vector<Term *> arguments;
        for (int i = 0; i < static_cast<int>(size); ++i) {
          arguments.push_back(unZ3(Z3_get_app_arg(z3context, app, i), function->arguments[i], boundVars));
        }
        Term *resultUnZ3 = getFunTerm(function, arguments);
        Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
        return resultUnZ3;
      }
    }
    break;
    case Z3_OP_TRUE:
    {
      Term *resultUnZ3 = bTrue();
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_FALSE:
    {
      Term *resultUnZ3 = bFalse();
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_LE:
    {
      assert(sort == getBoolSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_LE application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Term *resultUnZ3 = mle(unZ3(arg1, getIntSort(), boundVars), unZ3(arg2, getIntSort(), boundVars));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_EQ:
    {
      assert(sort == getBoolSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_EQ application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Sort *sa1 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg1)));
      Sort *sa2 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg2)));
      if ((!sa1) || (!sa2)) {
        abortWithMessage("Cannot find sorts in Z3_OP_EQ application.");
      }
      if (sa1 != sa2) {
        abortWithMessage("Equality for different sorts in Z3_OP_EQ application.");
      }
      Function *fun = getEqualsFunction(sa1);
      if (!fun) {
        abortWithMessage("Cannot find equal function in Z3_OP_EQ application.");
      }
      Term *resultUnZ3 = getFunTerm(fun, vector2(
        unZ3(arg1, sa1, boundVars),
        unZ3(arg2, sa2, boundVars)));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_DISTINCT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DISTINCT    .");
      break;
    case Z3_OP_ITE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ITE .");
      break;
    case Z3_OP_AND:
    {
      assert(sort == getBoolSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) < 2) {
        abortWithMessage("Expected >= 2 arguments in Z3_OP_AND application.");
      }
      vector<Term *> args;
      for (int i = 0; i < static_cast<int>(Z3_get_app_num_args(z3context, app)); ++i) {
        args.push_back(unZ3(Z3_get_app_arg(z3context, app, i), getBoolSort(), boundVars));
      }
      Term *resultUnZ3 = bAndVector(args);
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_OR:
    {
      assert(sort == getBoolSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) < 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_OR application.");
      }

      vector<Term *> args;
      for (int i = 0; i < static_cast<int>(Z3_get_app_num_args(z3context, app)); ++i) {
        args.push_back(unZ3(Z3_get_app_arg(z3context, app, i), getBoolSort(), boundVars));
      }
      Term *resultUnZ3 = bOrVector(args);
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_IFF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_IFF .");
      break;
    case Z3_OP_XOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_XOR .");
      break;
    case Z3_OP_NOT:
    {
      assert(sort == getBoolSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 1) {
        abortWithMessage("Expected 1 argument in Z3_OP_NOT application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Term *resultUnZ3 = bNot(unZ3(arg1, getBoolSort(), boundVars));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_IMPLIES:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_IMPLIES.");
      break;
    case Z3_OP_OEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_OEQ .");
      break;
    case Z3_OP_ANUM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ANUM.");
      break;
    case Z3_OP_AGNUM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_AGNUM.");
      break;
    case Z3_OP_GE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_GE  .");
      break;
    case Z3_OP_LT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_LT  .");
      break;
    case Z3_OP_GT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_GT  .");
      break;
    case Z3_OP_ADD:
    {
      assert(sort == getIntSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) < 2) {
        abortWithMessage("Expected >= 2 arguments in Z3_OP_ADD application.");
      }
      vector<Term *> args;
      for (int i = 0; i < static_cast<int>(Z3_get_app_num_args(z3context, app)); ++i) {
        args.push_back(unZ3(Z3_get_app_arg(z3context, app, i), getIntSort(), boundVars));
      }
      Term *resultUnZ3 = mPlusVector(args);
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_SUB:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SUB .");
      break;
    case Z3_OP_UMINUS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_UMINUS.");
      break;
    case Z3_OP_MUL:
    {
      assert(sort == getIntSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) < 2) {
        abortWithMessage("Expected >= 2 arguments in Z3_OP_MUL application.");
      }
      vector<Term *> args;
      for (int i = 0; i < static_cast<int>(Z3_get_app_num_args(z3context, app)); ++i) {
        args.push_back(unZ3(Z3_get_app_arg(z3context, app, i), getIntSort(), boundVars));
      }
      Term *resultUnZ3 = mTimesVector(args);
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_DIV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DIV .");
      break;
    case Z3_OP_IDIV:
    {
      assert(sort == getIntSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_IDIV application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Term *resultUnZ3 = mdiv(unZ3(arg1, getIntSort(), boundVars), unZ3(arg2, getIntSort(), boundVars));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_REM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_REM .");
      break;
    case Z3_OP_MOD:
    {
      assert(sort == getIntSort());
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_MOD application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Term *resultUnZ3 = mmod(unZ3(arg1, getIntSort(), boundVars), unZ3(arg2, getIntSort(), boundVars));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_TO_REAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_TO_REAL.");
      break;
    case Z3_OP_TO_INT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_TO_INT.");
      break;
    case Z3_OP_IS_INT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_IS_INT.");
      break;
    case Z3_OP_POWER:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_POWER.");
      break;
    case Z3_OP_STORE:
    {
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 3) {
        abortWithMessage("Expected 2 arguments in Z3_OP_STORE application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Z3_ast arg3 = Z3_get_app_arg(z3context, app, 2);
      Sort *sa1 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg1)));
      Sort *sa2 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg2)));
      Sort *sa3 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg3)));
      if ((!sa1) || (!sa2) || (!sa3)) {
        abortWithMessage("Cannot find sorts in Z3_OP_STORE application.");
      }
      pair< Sort*, pair<Sort*, Sort*> > key = make_pair(sa1, make_pair(sa2, sa3));
      Function *fun = (*getStoreFunctions())[key];
      if (!fun) {
        abortWithMessage("Cannot find select function in Z3_OP_STORE application.");
      }
      if (fun->result != sa1) {
        abortWithMessage("Select function has wrong sort in Z3_OP_STORE application.");
      }
      Term *resultUnZ3 = getFunTerm(fun, vector3(
        unZ3(arg1, sa1, boundVars),
        unZ3(arg2, sa2, boundVars),
        unZ3(arg3, sa3, boundVars)
      ));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_SELECT:
    {
      Z3_app app = Z3_to_app(z3context, ast);
      if (Z3_get_app_num_args(z3context, app) != 2) {
        abortWithMessage("Expected 2 arguments in Z3_OP_SELECT application.");
      }
      Z3_ast arg1 = Z3_get_app_arg(z3context, app, 0);
      Z3_ast arg2 = Z3_get_app_arg(z3context, app, 1);
      Sort *sa1 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg1)));
      Sort *sa2 = getBuiltinSort(z3_sort_to_string(Z3_get_sort(z3context, arg2)));
      if ((!sa1) || (!sa2)) {
        abortWithMessage("Cannot find sorts in Z3_OP_SELECT application.");
      }
      pair< Sort*, pair<Sort*, Sort*> > key = make_pair(sort, make_pair(sa1, sa2));
      Function *fun = (*getSelectFunctions())[key];
      if (!fun) {
        abortWithMessage("Cannot find select function in Z3_OP_SELECT application.");
      }
      if (fun->result != sort) {
        abortWithMessage("Select function has wrong sort in Z3_OP_SELECT application.");
      }
      Term *resultUnZ3 = getFunTerm(fun, vector2(
        unZ3(arg1, sa1, boundVars),
        unZ3(arg2, sa2, boundVars)));
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    break;
    case Z3_OP_CONST_ARRAY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_CONST_ARRAY .");
      break;
    case Z3_OP_ARRAY_MAP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ARRAY_MAP   .");
      break;
    case Z3_OP_ARRAY_DEFAULT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ARRAY_DEFAULT.");
      break;
    case Z3_OP_SET_UNION:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SET_UNION   .");
      break;
    case Z3_OP_SET_INTERSECT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SET_INTERSECT.");
      break;
    case Z3_OP_SET_DIFFERENCE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SET_DIFFERENCE.");
      break;
    case Z3_OP_SET_COMPLEMENT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SET_COMPLEMENT.");
      break;
    case Z3_OP_SET_SUBSET:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SET_SUBSET  .");
      break;
    case Z3_OP_AS_ARRAY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_AS_ARRAY    .");
      break;
    case Z3_OP_ARRAY_EXT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ARRAY_EXT   .");
      break;
    case Z3_OP_BNUM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BNUM.");
      break;
    case Z3_OP_BIT1:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BIT1.");
      break;
    case Z3_OP_BIT0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BIT0.");
      break;
    case Z3_OP_BNEG:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BNEG.");
      break;
    case Z3_OP_BADD:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BADD.");
      break;
    case Z3_OP_BSUB:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSUB.");
      break;
    case Z3_OP_BMUL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BMUL.");
      break;
    case Z3_OP_BSDIV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSDIV.");
      break;
    case Z3_OP_BUDIV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUDIV.");
      break;
    case Z3_OP_BSREM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSREM.");
      break;
    case Z3_OP_BUREM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUREM.");
      break;
    case Z3_OP_BSMOD:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSMOD.");
      break;
    case Z3_OP_BSDIV0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSDIV0.");
      break;
    case Z3_OP_BUDIV0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUDIV0.");
      break;
    case Z3_OP_BSREM0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSREM0.");
      break;
    case Z3_OP_BUREM0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUREM0.");
      break;
    case Z3_OP_BSMOD0:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSMOD0.");
      break;
    case Z3_OP_ULEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ULEQ.");
      break;
    case Z3_OP_SLEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SLEQ.");
      break;
    case Z3_OP_UGEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_UGEQ.");
      break;
    case Z3_OP_SGEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SGEQ.");
      break;
    case Z3_OP_ULT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ULT .");
      break;
    case Z3_OP_SLT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SLT .");
      break;
    case Z3_OP_UGT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_UGT .");
      break;
    case Z3_OP_SGT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SGT .");
      break;
    case Z3_OP_BAND:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BAND.");
      break;
    case Z3_OP_BOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BOR .");
      break;
    case Z3_OP_BNOT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BNOT.");
      break;
    case Z3_OP_BXOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BXOR.");
      break;
    case Z3_OP_BNAND:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BNAND.");
      break;
    case Z3_OP_BNOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BNOR.");
      break;
    case Z3_OP_BXNOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BXNOR.");
      break;
    case Z3_OP_CONCAT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_CONCAT.");
      break;
    case Z3_OP_SIGN_EXT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SIGN_EXT    .");
      break;
    case Z3_OP_ZERO_EXT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ZERO_EXT    .");
      break;
    case Z3_OP_EXTRACT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_EXTRACT.");
      break;
    case Z3_OP_REPEAT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_REPEAT.");
      break;
    case Z3_OP_BREDOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BREDOR.");
      break;
    case Z3_OP_BREDAND:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BREDAND.");
      break;
    case Z3_OP_BCOMP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BCOMP.");
      break;
    case Z3_OP_BSHL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSHL.");
      break;
    case Z3_OP_BLSHR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BLSHR.");
      break;
    case Z3_OP_BASHR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BASHR.");
      break;
    case Z3_OP_ROTATE_LEFT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ROTATE_LEFT .");
      break;
    case Z3_OP_ROTATE_RIGHT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_ROTATE_RIGHT.");
      break;
    case Z3_OP_EXT_ROTATE_LEFT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_EXT_ROTATE_LEFT.");
      break;
    case Z3_OP_EXT_ROTATE_RIGHT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_EXT_ROTATE_RIGHT    .");
      break;
    case Z3_OP_BIT2BOOL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BIT2BOOL    .");
      break;
    case Z3_OP_INT2BV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_INT2BV.");
      break;
    case Z3_OP_BV2INT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BV2I.");
      break;
    case Z3_OP_CARRY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_CARRY.");
      break;
    case Z3_OP_XOR3:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_XOR3.");
      break;
    case Z3_OP_BSMUL_NO_OVFL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSMUL_NO_OVFL.");
      break;
    case Z3_OP_BUMUL_NO_OVFL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUMUL_NO_OVFL.");
      break;
    case Z3_OP_BSMUL_NO_UDFL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSMUL_NO_UDFL.");
      break;
    case Z3_OP_BSDIV_I:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSDIV_I.");
      break;
    case Z3_OP_BUDIV_I:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUDIV_I.");
      break;
    case Z3_OP_BSREM_I:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSREM_I.");
      break;
    case Z3_OP_BUREM_I:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BUREM_I.");
      break;
    case Z3_OP_BSMOD_I:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_BSMOD_I.");
      break;
    case Z3_OP_PR_UNDEF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_UNDEF    .");
      break;
    case Z3_OP_PR_TRUE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_TRUE.");
      break;
    case Z3_OP_PR_ASSERTED:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_ASSERTED .");
      break;
    case Z3_OP_PR_GOAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_GOAL.");
      break;
    case Z3_OP_PR_MODUS_PONENS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_MODUS_PONENS.");
      break;
    case Z3_OP_PR_REFLEXIVITY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_REFLEXIVITY.");
      break;
    case Z3_OP_PR_SYMMETRY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_SYMMETRY .");
      break;
    case Z3_OP_PR_TRANSITIVITY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_TRANSITIVITY.");
      break;
    case Z3_OP_PR_TRANSITIVITY_STAR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_TRANSITIVITY_STAR.");
      break;
    case Z3_OP_PR_MONOTONICITY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_MONOTONICITY.");
      break;
    case Z3_OP_PR_QUANT_INTRO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_QUANT_INTRO.");
      break;
    case Z3_OP_PR_BIND:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_BIND.");
      break;
    case Z3_OP_PR_DISTRIBUTIVITY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_DISTRIBUTIVITY   .");
      break;
    case Z3_OP_PR_AND_ELIM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_AND_ELIM .");
      break;
    case Z3_OP_PR_NOT_OR_ELIM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_NOT_OR_ELIM.");
      break;
    case Z3_OP_PR_REWRITE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_REWRITE  .");
      break;
    case Z3_OP_PR_REWRITE_STAR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_REWRITE_STAR.");
      break;
    case Z3_OP_PR_PULL_QUANT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_PULL_QUANT.");
      break;
    case Z3_OP_PR_PUSH_QUANT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_PUSH_QUANT.");
      break;
    case Z3_OP_PR_ELIM_UNUSED_VARS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_ELIM_UNUSED_VARS .");
      break;
    case Z3_OP_PR_DER:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_DER.");
      break;
    case Z3_OP_PR_QUANT_INST:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_QUANT_INST.");
      break;
    case Z3_OP_PR_HYPOTHESIS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_HYPOTHESIS.");
      break;
    case Z3_OP_PR_LEMMA:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_LEMMA    .");
      break;
    case Z3_OP_PR_UNIT_RESOLUTION:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_UNIT_RESOLUTION  .");
      break;
    case Z3_OP_PR_IFF_TRUE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_IFF_TRUE .");
      break;
    case Z3_OP_PR_IFF_FALSE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_IFF_FALSE.");
      break;
    case Z3_OP_PR_COMMUTATIVITY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_COMMUTATIVITY    .");
      break;
    case Z3_OP_PR_DEF_AXIOM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_DEF_AXIOM.");
      break;
    case Z3_OP_PR_DEF_INTRO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_DEF_INTRO.");
      break;
    case Z3_OP_PR_APPLY_DEF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_APPLY_DEF.");
      break;
    case Z3_OP_PR_IFF_OEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_IFF_OEQ  .");
      break;
    case Z3_OP_PR_NNF_POS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_NNF_POS  .");
      break;
    case Z3_OP_PR_NNF_NEG:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_NNF_NEG  .");
      break;
    case Z3_OP_PR_SKOLEMIZE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_SKOLEMIZE.");
      break;
    case Z3_OP_PR_MODUS_PONENS_OEQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_MODUS_PONENS_OEQ .");
      break;
    case Z3_OP_PR_TH_LEMMA:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_TH_LEMMA .");
      break;
    case Z3_OP_PR_HYPER_RESOLVE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PR_HYPER_RESOLVE    .");
      break;
    case Z3_OP_RA_STORE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_STORE    .");
      break;
    case Z3_OP_RA_EMPTY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_EMPTY    .");
      break;
    case Z3_OP_RA_IS_EMPTY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_IS_EMPTY .");
      break;
    case Z3_OP_RA_JOIN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_JOIN.");
      break;
    case Z3_OP_RA_UNION:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_UNION    .");
      break;
    case Z3_OP_RA_WIDEN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_WIDEN    .");
      break;
    case Z3_OP_RA_PROJECT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_PROJECT  .");
      break;
    case Z3_OP_RA_FILTER:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_FILTER   .");
      break;
    case Z3_OP_RA_NEGATION_FILTER:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_NEGATION_FILTER  .");
      break;
    case Z3_OP_RA_RENAME:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_RENAME   .");
      break;
    case Z3_OP_RA_COMPLEMENT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_COMPLEMENT.");
      break;
    case Z3_OP_RA_SELECT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_SELECT   .");
      break;
    case Z3_OP_RA_CLONE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RA_CLONE    .");
      break;
    case Z3_OP_FD_CONSTANT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FD_CONSTANT .");
      break;
    case Z3_OP_FD_LT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FD_LT.");
      break;
    case Z3_OP_SEQ_UNIT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_UNIT    .");
      break;
    case Z3_OP_SEQ_EMPTY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_EMPTY   .");
      break;
    case Z3_OP_SEQ_CONCAT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_CONCAT  .");
      break;
    case Z3_OP_SEQ_PREFIX:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_PREFIX  .");
      break;
    case Z3_OP_SEQ_SUFFIX:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_SUFFIX  .");
      break;
    case Z3_OP_SEQ_CONTAINS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_CONTAINS.");
      break;
    case Z3_OP_SEQ_EXTRACT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_EXTRACT .");
      break;
    case Z3_OP_SEQ_REPLACE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_REPLACE .");
      break;
    case Z3_OP_SEQ_AT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_AT.");
      break;
    case Z3_OP_SEQ_LENGTH:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_LENGTH  .");
      break;
    case Z3_OP_SEQ_INDEX:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_INDEX   .");
      break;
    case Z3_OP_SEQ_TO_RE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_TO_RE   .");
      break;
    case Z3_OP_SEQ_IN_RE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_SEQ_IN_RE   .");
      break;
    case Z3_OP_STR_TO_INT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_STR_TO_INT  .");
      break;
    case Z3_OP_INT_TO_STR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_INT_TO_STR  .");
      break;
    case Z3_OP_RE_PLUS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_PLUS.");
      break;
    case Z3_OP_RE_STAR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_STAR.");
      break;
    case Z3_OP_RE_OPTION:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_OPTION   .");
      break;
    case Z3_OP_RE_CONCAT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_CONCAT   .");
      break;
    case Z3_OP_RE_UNION:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_UNION    .");
      break;
    case Z3_OP_RE_RANGE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_RANGE    .");
      break;
    case Z3_OP_RE_LOOP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_LOOP.");
      break;
    case Z3_OP_RE_INTERSECT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_INTERSECT.");
      break;
    case Z3_OP_RE_EMPTY_SET:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_EMPTY_SET.");
      break;
    case Z3_OP_RE_FULL_SET:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_FULL_SET .");
      break;
    case Z3_OP_RE_COMPLEMENT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_RE_COMPLEMENT.");
      break;
    case Z3_OP_LABEL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_LABEL.");
      break;
    case Z3_OP_LABEL_LIT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_LABEL_LIT   .");
      break;
    case Z3_OP_DT_CONSTRUCTOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DT_CONSTRUCTOR.");
      break;
    case Z3_OP_DT_RECOGNISER:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DT_RECOGNISER.");
      break;
    case Z3_OP_DT_IS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DT_IS");
      break;
    case Z3_OP_DT_ACCESSOR:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DT_ACCESSOR .");
      break;
    case Z3_OP_DT_UPDATE_FIELD:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_DT_UPDATE_FIELD.");
      break;
    case Z3_OP_PB_AT_MOST:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PB_AT_MOST  .");
      break;
    case Z3_OP_PB_AT_LEAST:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PB_AT_LEAST .");
      break;
    case Z3_OP_PB_LE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PB_LE.");
      break;
    case Z3_OP_PB_GE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PB_GE.");
      break;
    case Z3_OP_PB_EQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_PB_EQ.");
      break;
    case Z3_OP_FPA_RM_NEAREST_TIES_TO_EVEN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_RM_NEAREST_TIES_TO_EVEN .");
      break;
    case Z3_OP_FPA_RM_NEAREST_TIES_TO_AWAY:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_RM_NEAREST_TIES_TO_AWAY .");
      break;
    case Z3_OP_FPA_RM_TOWARD_POSITIVE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_RM_TOWARD_POSITIVE.");
      break;
    case Z3_OP_FPA_RM_TOWARD_NEGATIVE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_RM_TOWARD_NEGATIVE.");
      break;
    case Z3_OP_FPA_RM_TOWARD_ZERO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_RM_TOWARD_ZERO  .");
      break;
    case Z3_OP_FPA_NUM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_NUM.");
      break;
    case Z3_OP_FPA_PLUS_INF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_PLUS_INF.");
      break;
    case Z3_OP_FPA_MINUS_INF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_MINUS_INF.");
      break;
    case Z3_OP_FPA_NAN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_NAN.");
      break;
    case Z3_OP_FPA_PLUS_ZERO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_PLUS_ZERO.");
      break;
    case Z3_OP_FPA_MINUS_ZERO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_MINUS_ZERO.");
      break;
    case Z3_OP_FPA_ADD:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_ADD.");
      break;
    case Z3_OP_FPA_SUB:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_SUB.");
      break;
    case Z3_OP_FPA_NEG:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_NEG.");
      break;
    case Z3_OP_FPA_MUL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_MUL.");
      break;
    case Z3_OP_FPA_DIV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_DIV.");
      break;
    case Z3_OP_FPA_REM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_REM.");
      break;
    case Z3_OP_FPA_ABS:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_ABS.");
      break;
    case Z3_OP_FPA_MIN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_MIN.");
      break;
    case Z3_OP_FPA_MAX:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_MAX.");
      break;
    case Z3_OP_FPA_FMA:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_FMA.");
      break;
    case Z3_OP_FPA_SQRT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_SQRT    .");
      break;
    case Z3_OP_FPA_ROUND_TO_INTEGRAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_ROUND_TO_INTEGRAL.");
      break;
    case Z3_OP_FPA_EQ:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_EQ.");
      break;
    case Z3_OP_FPA_LT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_LT.");
      break;
    case Z3_OP_FPA_GT:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_GT.");
      break;
    case Z3_OP_FPA_LE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_LE.");
      break;
    case Z3_OP_FPA_GE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_GE.");
      break;
    case Z3_OP_FPA_IS_NAN:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_NAN  .");
      break;
    case Z3_OP_FPA_IS_INF:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_INF  .");
      break;
    case Z3_OP_FPA_IS_ZERO:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_ZERO .");
      break;
    case Z3_OP_FPA_IS_NORMAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_NORMAL.");
      break;
    case Z3_OP_FPA_IS_SUBNORMAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_SUBNORMAL    .");
      break;
    case Z3_OP_FPA_IS_NEGATIVE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_NEGATIVE.");
      break;
    case Z3_OP_FPA_IS_POSITIVE:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_IS_POSITIVE.");
      break;
    case Z3_OP_FPA_FP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_FP.");
      break;
    case Z3_OP_FPA_TO_FP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_FP   .");
      break;
    case Z3_OP_FPA_TO_FP_UNSIGNED:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_FP_UNSIGNED  .");
      break;
    case Z3_OP_FPA_TO_UBV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_UBV  .");
      break;
    case Z3_OP_FPA_TO_SBV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_SBV  .");
      break;
    case Z3_OP_FPA_TO_REAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_REAL .");
      break;
    case Z3_OP_FPA_TO_IEEE_BV:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_TO_IEEE_BV.");
      break;
    case Z3_OP_FPA_BVWRAP:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_BVWRAP  .");
      break;
    case Z3_OP_FPA_BV2RM:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_FPA_BV2RM   .");
      break;
    case Z3_OP_INTERNAL:
      abortWithMessage("In unZ3, cannot handle decl kind Z3_OP_INTERNAL    .");
      break;
    }
  }
  break;
  case Z3_NUMERAL_AST:
  {
    int result;
    if (Z3_get_numeral_int(z3context, ast, &result)) {
      string funname = string_from_int(result);
      Function *function = getFunction(funname);
      if (!function) {
        vector<Sort *> arguments;
        extern map<string, Function *> functions;
        functions[funname] = new Function(funname, arguments, getIntSort(), new z3_ct(result));
        function = getFunction(funname);
        assert(function != 0);
      }
      Term *resultUnZ3 = getFunTerm(function, vector<Term*>());
      Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
      return resultUnZ3;
    }
    else {
      abortWithMessage("For ast of kind Z3_NUMERAL_AST, cannot retrieve numeral outside of int range.");
    }
  }
  break;
  case Z3_VAR_AST:
  {
    unsigned index = Z3_get_index_value(z3context, ast);
    Log(DEBUG) << "Got index of " << index << " and boundVars.size() = " << boundVars.size() << "." << endl;
    if (index >= boundVars.size()) {
      abortWithMessage("Error in unz3-ing an ast of kind Z3_VAR_AST (bound variable out of range).");
    }
    Term *resultUnZ3 = getVarTerm(boundVars[boundVars.size() - 1 - index]);
    Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
    return resultUnZ3;
  }
  break;
  case Z3_QUANTIFIER_AST:
  {
    Log(DEBUG) << "UnZ3-ing quantifier" << endl;
    int nrBoundVars = (int)Z3_get_quantifier_num_bound(z3context, ast);
    vector<Variable *> newBoundVars = boundVars;
    vector<Variable *> freshBoundVars;
    for (int bvarnum = 0; bvarnum < nrBoundVars; ++bvarnum) {
      Z3_symbol symbol = Z3_get_quantifier_bound_name(z3context, ast, bvarnum);
      assert(z3_const_to_var.find(symbol) != z3_const_to_var.end());
      Variable *boundVar = z3_const_to_var[symbol];
      newBoundVars.push_back(boundVar);
      freshBoundVars.push_back(boundVar);
    }

    Z3_ast body = Z3_get_quantifier_body(z3context, ast);
    Term *bodyTerm = unZ3(body, getBoolSort(), newBoundVars);
    Term *(*whichQ)(Variable*, Term*) = NULL;
    if (Z3_is_quantifier_forall(z3context, ast)) whichQ = bForall;
    else if (Z3_is_quantifier_exists(z3context, ast)) whichQ = bExists;
    else {
      abortWithMessage("In UnZ3: unknown quantifier.");
    }
    reverse(freshBoundVars.begin(), freshBoundVars.end());
    Term *resultUnZ3 = bodyTerm;
    for (const auto &boundVar : freshBoundVars)
      resultUnZ3 = whichQ(boundVar, resultUnZ3);
    Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
    return resultUnZ3;
  }
  break;
  case Z3_SORT_AST:
    abortWithMessage("Cannot unz3 an ast of kind Z3_SORT_AST.");
    break;
  case Z3_FUNC_DECL_AST:
    abortWithMessage("Cannot unz3 an ast of kind Z3_FUNC_DECL_AST.");
    break;
  case Z3_UNKNOWN_AST:
    abortWithMessage("Cannot unz3 an ast of kind Z3_UNKNOWN_AST.");
    break;
  default:
    Log(DEBUG8) << "unZ3: undefined case." << endl;
    break;
}
assert(0);
Term *resultUnZ3 = 0;
Log(DEBUG8) << "Result of unZ3 = " << resultUnZ3->toString() << "." << endl;
return resultUnZ3;
}

Z3_func_decl createZ3FunctionSymbol(string name, std::vector<Sort *> arguments, Sort *resultSort)
{
  assert(arguments.size() < 16);
  Z3_sort domain[16];
  unsigned size = arguments.size();
  for (int i = 0; i < static_cast<int>(size); ++i) {
    assert(arguments[i]->hasInterpretation);
    domain[i] = arguments[i]->interpretation;
  }
  assert(resultSort->hasInterpretation);
  Z3_sort range = resultSort->interpretation;
  Z3_symbol symbol = Z3_mk_string_symbol(z3context, name.c_str());
  Z3_func_decl result = Z3_mk_func_decl(z3context, symbol, size, domain, range);
  symbol_to_func_decl[symbol] = result;
  return result;
}

void addZ3Assert(Term *formula)
{
  z3asserts.push_back(formula->toSmt());
}

Z3_ast z3exists(Variable *variable, Term *term)
{
  Log(DEBUG) << "z3exists " << variable->name << "." << term->toString() << endl;
  Z3_app bound[4];
  Z3_pattern patterns[4];
  assert(Z3_get_ast_kind(z3context, variable->interpretation) == Z3_APP_AST);
  bound[0] = Z3_to_app(z3context, variable->interpretation);
  return Z3_mk_exists_const(z3context, 0, 1, bound, 0, patterns, term->toSmt());
}

Z3_ast z3forall(Variable *variable, Term *term)
{
  Log(DEBUG) << "z3forall " << variable->name << "." << term->toString() << endl;
  Z3_app bound[4];
  Z3_pattern patterns[4];
  assert(Z3_get_ast_kind(z3context, variable->interpretation) == Z3_APP_AST);
  bound[0] = Z3_to_app(z3context, variable->interpretation);
  return Z3_mk_forall_const(z3context, 0, 1, bound, 0, patterns, term->toSmt());
}

z3_fresh::z3_fresh(Sort *sort) :
  sort(sort)
{
  if (sort == getIntSort()) {
    fresh_symbol = Z3_mk_int_symbol(z3context, z3_symbol_count++);
  }
  else {
    assert(0);
  }
}

void z3_fresh::setTerm(Term *term)
{
  z3_const_to_const[fresh_symbol] = term;
}

Z3_ast z3_fresh::operator()(std::vector<Term *> args)
{
  assert(args.size() == 0);
  return Z3_mk_const(z3context, fresh_symbol, sort->interpretation);
}

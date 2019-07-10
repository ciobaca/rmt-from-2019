#ifndef Z3_DRIVER__
#define Z3_DRIVER__

#include <vector>
#include <string>
#include <z3.h>

struct Sort;

struct Variable;
struct Term;

Z3_string z3_sort_to_string(Z3_sort s);

enum Z3Result
{
  sat,
  unsat,
  unknown
};

struct Z3Theory
{
  std::vector<Variable *> variables;
  std::vector<Term *> constraints;
  
  void addVariable(Variable *);
  void addEqualityConstraint(Term *, Term *);
  void addConstraint(Term *);

  Term *toNormTerm();

  Z3Result isSatisfiable();
  Z3Result isSatisfiableHelper();
};

Z3Result isSatisfiable(Term *constraint);

bool isValid(Term *constraint);

void parse_z3_prelude(std::string);
void start_z3_api();
void test_z3_api();

//typedef Z3_ast (*func_interpretation)(std::vector<Term *>);

Z3_ast z3_simplify(Term *);

Z3_sort z3_bool();

Z3_sort z3_int();

Z3_sort z3_uninterpreted_sort(std::string sortName);

Z3_sort z3_getInterpretation(std::string i);

class Z3Function {
public:
  Z3Function() {};
  virtual Z3_ast operator()(std::vector<Term *>) = 0;
};

class z3_add : public Z3Function {
public:
  z3_add() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_mul : public Z3Function {
 public:
  z3_mul() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_sub : public Z3Function {
 public:
  z3_sub() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_div : public Z3Function {
 public:
  z3_div() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_mod : public Z3Function {
 public:
  z3_mod() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ge : public Z3Function {
 public:
  z3_ge() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_gt : public Z3Function {
 public:
  z3_gt() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_le : public Z3Function {
 public:
  z3_le() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_lt : public Z3Function {
 public:
  z3_lt() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_eq : public Z3Function {
 public:
  z3_eq() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_0 : public Z3Function {
 public:
  z3_ct_0() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_1 : public Z3Function {
 public:
  z3_ct_1() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_2 : public Z3Function {
 public:
  z3_ct_2() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_3 : public Z3Function {
 public:
  z3_ct_3() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_4 : public Z3Function {
 public:
  z3_ct_4() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_5 : public Z3Function {
 public:
  z3_ct_5() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_6 : public Z3Function {
 public:
  z3_ct_6() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_7 : public Z3Function {
 public:
  z3_ct_7() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_8 : public Z3Function {
 public:
  z3_ct_8() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_9 : public Z3Function {
 public:
  z3_ct_9() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_10 : public Z3Function {
 public:
  z3_ct_10() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_11 : public Z3Function {
 public:
  z3_ct_11() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_12 : public Z3Function {
 public:
  z3_ct_12() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_13 : public Z3Function {
 public:
  z3_ct_13() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_14 : public Z3Function {
 public:
  z3_ct_14() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct_15 : public Z3Function {
 public:
  z3_ct_15() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_fresh : public Z3Function {
 private:
  Sort *sort;
  Z3_symbol fresh_symbol;
 public:
  z3_fresh(Sort *sort);
  void setTerm(Term *);
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ct : public Z3Function {
private:
  int num;
public:
  z3_ct(int num) : num(num) {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_true : public Z3Function {
 public:
  z3_true() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_false : public Z3Function {
 public:
  z3_false() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_not : public Z3Function {
 public:
  z3_not() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_and : public Z3Function {
 public:
  z3_and() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_or : public Z3Function {
 public:
  z3_or() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_iff : public Z3Function {
 public:
  z3_iff() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_ite : public Z3Function {
 public:
  z3_ite() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_implies : public Z3Function {
 public:
  z3_implies() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_select : public Z3Function {
public : 
  z3_select() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

class z3_store : public Z3Function {
public:
  z3_store() {};
  virtual Z3_ast operator()(std::vector<Term *> args);
};

struct Function;

class z3_custom_func : public Z3Function {
  Z3_func_decl func;
public:
  z3_custom_func(Z3_func_decl func, Function *);
  virtual Z3_ast operator()(std::vector<Term *> args);
};

Z3_ast z3_make_constant(Variable *);

Term *unZ3(Z3_ast, Sort *, std::vector<Variable *> = std::vector<Variable *>());

Z3_func_decl createZ3FunctionSymbol(std::string name, std::vector<Sort *> arguments, Sort *result);

void addZ3Assert(Term *);

Z3_ast z3exists(Variable *, Term *);

Z3_ast z3forall(Variable *, Term *);

#endif

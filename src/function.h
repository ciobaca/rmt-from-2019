#ifndef FUNCTION_H__
#define FUNCTION_H__

#include <string>
#include <vector>
#include "z3driver.h"
#include "helper.h"
#include "constrainedrewritesystem.h"

struct Sort;

using namespace std;

struct Function
{
  string name;
  vector<Sort *> arguments;
  Sort *result;

  bool isCommutative;
  bool isAssociative;
  Function *unityElement;

  bool hasInterpretation; // should be interpreted as a builtin function
  Z3Function *interpretation; // the interpretation of the function
  // only valid if hasIntrepretation == true

  bool isEqualityFunction; // true if this is mequals, bequals, etc.

  bool isDefined; // is a defined function
  ConstrainedRewriteSystem crewrite; // only valid if it's a defined function

  bool isFresh;

  void updateDefined(ConstrainedRewriteSystem &crewrite)
  {
    this->isDefined = true;
    this->crewrite = crewrite;
  }

private:

  Function(string name, vector<Sort *> arguments, Sort *result, bool isCommutative = false, bool isAssociative = false, Function *unityElement = nullptr)
  {
    this->name = name;
    this->arguments = arguments;
    this->result = result;
    this->hasInterpretation = false;
    this->isDefined = false;
    this->isEqualityFunction = false;
    this->isCommutative = isCommutative;
    this->isAssociative = isAssociative;
    this->unityElement = unityElement;
    this->isFresh = false;
  }

  Function(string name, vector<Sort *> arguments, Sort *result, string interpretation)
  {
    this->name = name;
    this->arguments = arguments;
    this->result = result;
    this->hasInterpretation = true;
    this->isEqualityFunction = false;
    this->isDefined = false;
    if (interpretation == "+") {
      this->interpretation = new z3_add();
    } else if (interpretation == "*") {
      this->interpretation = new z3_mul();
    } else if (interpretation == "-") {
      this->interpretation = new z3_sub();
    } else if (interpretation == "div") {
      this->interpretation = new z3_div();
    } else if (interpretation == "mod") {
      this->interpretation = new z3_mod();
    } else if (interpretation == "<=") {
      this->interpretation = new z3_le();
    } else if (interpretation == "<") {
      this->interpretation = new z3_lt();
    } else if (interpretation == "=") {
      this->interpretation = new z3_eq();
      this->isEqualityFunction = true;
    } else if (interpretation == ">") {
      this->interpretation = new z3_gt();
    } else if (interpretation == ">=") {
      this->interpretation = new z3_ge();
    } else if (interpretation == "0") {
      this->interpretation = new z3_ct_0();
    } else if (interpretation == "1") {
      this->interpretation = new z3_ct_1();
    } else if (interpretation == "2") {
      this->interpretation = new z3_ct_2();
    } else if (interpretation == "3") {
      this->interpretation = new z3_ct_3();
    } else if (interpretation == "4") {
      this->interpretation = new z3_ct_4();
    } else if (interpretation == "5") {
      this->interpretation = new z3_ct_5();
    } else if (interpretation == "6") {
      this->interpretation = new z3_ct_6();
    } else if (interpretation == "7") {
      this->interpretation = new z3_ct_7();
    } else if (interpretation == "8") {
      this->interpretation = new z3_ct_8();
    } else if (interpretation == "9") {
      this->interpretation = new z3_ct_9();
    } else if (interpretation == "10") {
      this->interpretation = new z3_ct_10();
    } else if (interpretation == "11") {
      this->interpretation = new z3_ct_11();
    } else if (interpretation == "12") {
      this->interpretation = new z3_ct_12();
    } else if (interpretation == "13") {
      this->interpretation = new z3_ct_13();
    } else if (interpretation == "14") {
      this->interpretation = new z3_ct_14();
    } else if (interpretation == "15") {
      this->interpretation = new z3_ct_15();
    } else if (interpretation == "true") {
      this->interpretation = new z3_true();
    } else if (interpretation == "false") {
      this->interpretation = new z3_false(); 
    } else if (interpretation == "not") {
      this->interpretation = new z3_not();
    } else if (interpretation == "and") {
      this->interpretation = new z3_and();
    } else if (interpretation == "iff") {
      this->interpretation = new z3_iff();
    } else if (interpretation == "or") {
      this->interpretation = new z3_or();
    } else if (interpretation == "=>") {
      this->interpretation = new z3_implies();
    } else if (interpretation == "ite") {
      this->interpretation = new z3_ite();
    } else if (interpretation == "select") {
      this->interpretation = new z3_select();
    } else if (interpretation == "store") {
      this->interpretation = new z3_store();
    } else {
      abortWithMessage(std::string("Unknown interpretation of function (") + interpretation + ").");
    }
    
    this->isDefined = false;

    this->isCommutative = false;
    this->isAssociative = false;
    this->unityElement = NULL;
    this->isFresh = false;
  }

  Function(string name, vector<Sort *> arguments, Sort *result, Z3Function *interpretation)
  {
    this->name = name;
    this->arguments = arguments;
    this->result = result;
    this->hasInterpretation = true;
    this->interpretation = interpretation;
    this->isDefined = false;
    this->isCommutative = false;
    this->isAssociative = false;
    this->unityElement = NULL;
    this->isFresh = false;
  }

  Function(string name, vector<Sort *> arguments, Sort *result, Z3_func_decl interpretation)
  {
    this->name = name;
    this->arguments = arguments;
    this->result = result;
    this->hasInterpretation = true;
    this->interpretation = new z3_custom_func(interpretation, this);
    this->isEqualityFunction = false;
    this->isDefined = false;
    this->isCommutative = false;
    this->isAssociative = false;
    this->unityElement = NULL;
    this->isFresh = false;
  }

  friend Term *unZ3(Z3_ast ast, Sort *sort, vector<Variable *> boundVars);
  friend void createUninterpretedFunction(string, vector<Sort *>, Sort *, bool, bool, Function*);
  friend void createInterpretedFunction(string, vector<Sort *>, Sort *, string);
  friend void createInterpretedFunction(string, vector<Sort *>, Sort *, Z3_func_decl);
  friend void createInterpretedFunction(string, vector<Sort *>, Sort *, Z3Function *);
};

#endif

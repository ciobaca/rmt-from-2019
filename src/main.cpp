/*

Entry point of the RMT tool.

(C) 2016 - 2018 Stefan Ciobaca
stefan.ciobaca@gmail.com
Faculty of Computer Science
Alexandru Ioan Cuza University
Iasi, Romania

http://profs.info.uaic.ro/~stefan.ciobaca/

*/
#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include "log.h"

#include "helper.h"
#include "term.h"
#include "parse.h"
#include "factories.h"
#include "sort.h"
#include "z3driver.h"
#include "search.h"
#include "constrainedterm.h"
#include "query.h"
#include "funterm.h"
#include "query.h"
#include "constrainedrewritesystem.h"
//#include "fastterm.h"

using namespace std;

void outputRewriteSystem(RewriteSystem &rewriteSystem)
{
  for (int i = 0; i < len(rewriteSystem); ++i) {
    pair<Term *, Term *> rewriteRule = rewriteSystem[i];
    cout << rewriteRule.first->toString() << " " << rewriteRule.second->toString() << endl;
  }
}

void createSubsort(vector<string> &identifiersLeft,
  vector<string> &identifiersRight)
{
  Log log(INFO);
  log << "Declaring the sorts: ";
  for (int i = 0; i < (int)identifiersLeft.size(); ++i) {
    log << identifiersLeft[i] << " ";
  }
  log << "as subsorts of ";
  for (int i = 0; i < (int)identifiersRight.size(); ++i) {
    log << identifiersRight[i] << " ";
  }
  log << endl;
  for (int i = 0; i < (int)identifiersLeft.size(); ++i) {
    for (int j = 0; j < (int)identifiersRight.size(); ++j) {
      Sort *left = getSort(identifiersLeft[i]);
      Sort *right = getSort(identifiersRight[j]);
      assert(left);
      assert(right);
      right->addSubSort(left);
    }
  }
}

void parseBuiltins(string &s, int &w)
{
  skipWhiteSpace(s, w);
  if (!lookAhead(s, w, "builtins")) {
    return;
  }
  matchString(s, w, "builtins");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string f = getIdentifier(s, w);
    skipWhiteSpace(s, w);
    match(s, w, ':');
    skipWhiteSpace(s, w);
    vector<Sort *> arguments;
    while (lookAheadIdentifier(s, w)) {
      string id = getIdentifier(s, w);
      if (!sortExists(id)) {
        parseError("(while parsing builtins) sort does not exist", w, s);
      }
      if (!isBuiltinSort(id)) {
        parseError("(while parsing builtins) sort is not builtin", w, s);
      }
      skipWhiteSpace(s, w);
      arguments.push_back(getSort(id));
    }
    matchString(s, w, "->");
    skipWhiteSpace(s, w);
    string id = getIdentifier(s, w);
    if (!sortExists(id)) {
      Log(ERROR) << "SORT: " << id << endl;
      parseError("(while parsing builtins) sort does not exist", w, s);
    }
    if (!isBuiltinSort(id)) {
      parseError("(while parsing builtins) sort is not builtin", w, s);
    }
    Sort *result = getSort(id);
    string interpretation = f;
    skipWhiteSpace(s, w);

    createInterpretedFunction(f, arguments, result, createZ3FunctionSymbol(f, arguments, result));
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more builtin function symbols", w, s);
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
}

void parseAsserts(string &s, int &w)
{
  skipWhiteSpace(s, w);
  while (lookAhead(s, w, "assert")) {
    matchString(s, w, "assert");
    skipWhiteSpace(s, w);
    Term *term = parseTerm(s, w);
    if (term->getSort() != getBoolSort()) {
      parseError("(while parsing asserts) asserts should be of sort Bool.", w, s);
    }
    addZ3Assert(term);
    skipWhiteSpace(s, w);
    matchString(s, w, ";");
    skipWhiteSpace(s, w);
  }
}

void parseDefinedFunctions(string &s, int &w)
{
  skipWhiteSpace(s, w);
  while (lookAhead(s, w, "define")) {
    matchString(s, w, "define");
    skipWhiteSpace(s, w);

    string f = getIdentifier(s, w);
    skipWhiteSpace(s, w);
    match(s, w, ':');
    skipWhiteSpace(s, w);
    vector<Sort *> arguments;
    while (lookAheadIdentifier(s, w)) {
      string id = getIdentifier(s, w);
      if (!sortExists(id)) {
        parseError("(while parsing defined function) sort does not exist", w, s);
      }
      skipWhiteSpace(s, w);
      arguments.push_back(getSort(id));
    }
    matchString(s, w, "->");
    skipWhiteSpace(s, w);
    string id = getIdentifier(s, w);
    if (!sortExists(id)) {
      Log(ERROR) << "SORT: " << id << endl;
      parseError("(while parsing defined function) sort does not exist", w, s);
    }
    Sort *result = getSort(id);

    createUninterpretedFunction(f, arguments, result);

    ConstrainedRewriteSystem crewrite;
    skipWhiteSpace(s, w);
    matchString(s, w, "by");
    skipWhiteSpace(s, w);
    while (w < len(s)) {
      skipWhiteSpace(s, w);
      ConstrainedTerm t = parseConstrainedTerm(s, w);
      skipWhiteSpace(s, w);
      matchString(s, w, "=>");
      skipWhiteSpace(s, w);
      Term *tp = parseTerm(s, w);
      Log(INFO) << "Parsed rewrite rule for func def: " << t.toString() << " => " << tp->toString() << endl;
      crewrite.addRule(t, tp);
      skipWhiteSpace(s, w);
      if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
        expected("more constrained rewrite rules", w, s);
      }
      if (s[w] == ',') {
        match(s, w, ',');
        continue;
      }
      else {
        match(s, w, ';');
        break;
      }
    }
    updateDefinedFunction(f, crewrite);

    skipWhiteSpace(s, w);
  }
}

void addPredefinedSorts()
{
  createInterpretedSort("Bool", "Bool");
  createInterpretedSort("Int", "Int");
}

void parseSorts(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "sorts");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string sortName = getIdentifier(s, w);
    string sortInterpretation;
    bool hasInterpretation = false;
    skipWhiteSpace(s, w);
    if (w < len(s) && s[w] == '/') {
      match(s, w, '/');
      skipWhiteSpace(s, w);
      sortInterpretation = getQuotedString(s, w);
      hasInterpretation = true;
      skipWhiteSpace(s, w);
    }
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more sort symbols", w, s);
    }
    if (sortExists(sortName)) {
      parseError("sort already exists", w, s);
    }
    if (hasInterpretation) {
      //this is a builtin sort!! .. will be refactored later (TODO)
      createInterpretedSort(sortName, sortInterpretation);
      createBuiltinExistsFunction(getSort(sortName));
      createBuiltinForallFunction(getSort(sortName));
    }
    else {
      createUninterpretedSort(sortName);
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
}

void parseSubsort(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "subsort");
  skipWhiteSpace(s, w);
  vector<string> identifiersLeft;
  if (!lookAheadIdentifier(s, w)) {
    expected("identifier", w, s);
  }
  else {
    do {
      string id = getIdentifier(s, w);
      if (!sortExists(id)) {
        parseError("sort does not exist", w, s);
      }
      identifiersLeft.push_back(id);
      skipWhiteSpace(s, w);
    } while (lookAheadIdentifier(s, w));
  }
  match(s, w, '<');
  skipWhiteSpace(s, w);
  vector<string> identifiersRight;
  if (!lookAheadIdentifier(s, w)) {
    expected("identifier", w, s);
  }
  else {
    do {
      string id = getIdentifier(s, w);
      if (!sortExists(id)) {
        parseError("sort does not exist", w, s);
      }
      identifiersRight.push_back(id);
      skipWhiteSpace(s, w);
    } while (lookAheadIdentifier(s, w));
  }
  match(s, w, ';');
  createSubsort(identifiersLeft, identifiersRight);
}

void parseSubsorts(string &s, int &w)
{
  skipWhiteSpace(s, w);
  while (lookAhead(s, w, "subsort")) {
    parseSubsort(s, w);
    skipWhiteSpace(s, w);
  }
}

void addPredefinedFunctions()
{
  Sort *intSort = getSort("Int");
  Sort *boolSort = getSort("Bool");
  assert(intSort);
  assert(boolSort);

  {
    vector<Sort *> arguments;
    createInterpretedFunction("0", arguments, intSort, "0");
    createInterpretedFunction("1", arguments, intSort, "1");
    createInterpretedFunction("2", arguments, intSort, "2");
    createInterpretedFunction("3", arguments, intSort, "3");
    createInterpretedFunction("4", arguments, intSort, "4");
    createInterpretedFunction("5", arguments, intSort, "5");
    createInterpretedFunction("6", arguments, intSort, "6");
    createInterpretedFunction("7", arguments, intSort, "7");
    createInterpretedFunction("8", arguments, intSort, "8");
    createInterpretedFunction("9", arguments, intSort, "9");
    createInterpretedFunction("10", arguments, intSort, "10");
    createInterpretedFunction("11", arguments, intSort, "11");
    createInterpretedFunction("12", arguments, intSort, "12");
    createInterpretedFunction("13", arguments, intSort, "13");
    createInterpretedFunction("14", arguments, intSort, "14");
    createInterpretedFunction("15", arguments, intSort, "15");

    arguments.push_back(intSort);
    arguments.push_back(intSort);
    createInterpretedFunction("mplus", arguments, intSort, "+");
    createInterpretedFunction("mminus", arguments, intSort, "-");
    createInterpretedFunction("mtimes", arguments, intSort, "*");
    createInterpretedFunction("mdiv", arguments, intSort, "div");
    createInterpretedFunction("mmod", arguments, intSort, "mod");
    createInterpretedFunction("mle", arguments, boolSort, "<=");
    createInterpretedFunction("mge", arguments, boolSort, ">=");
    createInterpretedFunction("mless", arguments, boolSort, "<");
    createInterpretedFunction("mgt", arguments, boolSort, ">");
    createInterpretedFunction("mequals", arguments, boolSort, "=");

    arguments.push_back(boolSort);
    std::reverse(arguments.begin(), arguments.end());
    createInterpretedFunction("iteInt", arguments, boolSort, "ite");
  }

  {
    vector<Sort *> arguments;
    createInterpretedFunction("true", arguments, boolSort, "true");
    createInterpretedFunction("false", arguments, boolSort, "false");

    arguments.push_back(boolSort);
    createInterpretedFunction("bnot", arguments, boolSort, "not");

    arguments.push_back(boolSort);
    createInterpretedFunction("band", arguments, boolSort, "and");
    createInterpretedFunction("biff", arguments, boolSort, "iff");
    createInterpretedFunction("bor", arguments, boolSort, "or");
    createInterpretedFunction("bimplies", arguments, boolSort, "=>");
    createInterpretedFunction("bequals", arguments, boolSort, "=");
  }
}

void parseFunctions(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "signature");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string f = getIdentifier(s, w);
    skipWhiteSpace(s, w);
    match(s, w, ':');
    skipWhiteSpace(s, w);
    vector<Sort *> arguments;
    while (lookAheadIdentifier(s, w)) {
      string id = getIdentifier(s, w);
      if (!sortExists(id)) {
        parseError("(while parsing function symbol arguments) sort does not exist", w, s);
      }
      skipWhiteSpace(s, w);
      arguments.push_back(getSort(id));
    }
    matchString(s, w, "->");
    skipWhiteSpace(s, w);
    string id = getIdentifier(s, w);
    if (!sortExists(id)) {
      Log(ERROR) << "SORT: " << id << endl;
      parseError("(while parsing function symbol result) sort does not exist", w, s);
    }
    Sort *result = getSort(id);
    string interpretation;
    bool hasInterpretation = false;
    skipWhiteSpace(s, w);
    if (lookAhead(s, w, "/")) {
      matchString(s, w, "/");
      skipWhiteSpace(s, w);
      if (lookAhead(s, w, "\"")) {
        interpretation = getQuotedString(s, w);
        hasInterpretation = true;
      }
      else {
        matchString(s, w, "\"");
      }
    }
    skipWhiteSpace(s, w);
    bool isCommutative = false;
    bool isAssociative = false;
    Function *unityElement = nullptr;
    if (lookAhead(s, w, "[")) {
      matchString(s, w, "[");
      skipWhiteSpace(s, w);
      while (w < len(s) && s[w] != ']') {
        switch(s[w]) {
          case 'a': isAssociative = true; ++w; break;
          case 'c': isCommutative = true; ++w; break;
          case 'u': {
            matchString(s, w, "u");
            skipWhiteSpace(s, w);
            if (lookAhead(s, w, "(")) {
              matchString(s, w, "(");
              skipWhiteSpace(s, w);
              string id = getIdentifier(s, w);
              unityElement = getFunction(id);
              skipWhiteSpace(s, w);
            } else {
              parseError("(while parsing unity element) '(' is missing", w, s);
            }
            if (lookAhead(s, w, ")")) {
              matchString(s, w, ")");
            } else {
              parseError("(while parsing unity element) ')' is missing", w, s);
            }
          }
        }
        skipWhiteSpace(s, w);
      }
      matchString(s, w, "]");
    }
    if (hasInterpretation) {
      createInterpretedFunction(f, arguments, result, interpretation);
    } else {
      if (isCommutative && isAssociative && arguments.size() != 2) {
        parseError("AC-functions must have two arguments.", w, s);
      }
      createUninterpretedFunction(f, arguments, result, isCommutative, isAssociative, unityElement);
    }
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more function symbols", w, s);
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
}

void parseVariables(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "variables");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string n = getIdentifier(s, w);
    if (variableExists(n)) {
      parseError("variable already exists", w, s);
    }
    skipWhiteSpace(s, w);
    matchString(s, w, ":");
    skipWhiteSpace(s, w);
    string id = getIdentifier(s, w);
    if (!sortExists(id)) {
      parseError("(while parsing variables) sort does not exist", w, s);
    }
    Sort *sort = getSort(id);
    createVariable(n, sort);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more variables", w, s);
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
}

void addPredefinedRewriteSystems()
{
  return;

  RewriteSystem rewrite;

  Sort *boolSort = getSort("Bool");
  Sort *intSort = getSort("Int");
  Term *B = getVarTerm(createFreshVariable(boolSort));
  Term *N = getVarTerm(createFreshVariable(intSort));

  rewrite.addRule(bNot(bFalse()), bTrue());
  rewrite.addRule(bNot(bTrue()), bFalse());
  rewrite.addRule(bNot(bNot(B)), B);

  rewrite.addRule(bAnd(bFalse(), B), bFalse());
  rewrite.addRule(bAnd(bTrue(), B), B);
  rewrite.addRule(bAnd(B, bFalse()), bFalse());
  rewrite.addRule(bAnd(B, bTrue()), B);

  rewrite.addRule(bImplies(bFalse(), B), bTrue());
  rewrite.addRule(bImplies(bTrue(), B), B);

  rewrite.addRule(bOr(bFalse(), B), B);
  rewrite.addRule(bOr(bTrue(), B), bTrue());
  rewrite.addRule(bOr(B, bFalse()), B);
  rewrite.addRule(bOr(B, bTrue()), bTrue());

  rewrite.addRule(mEquals(N, N), bTrue());

  rewrite.addRule(bEquals(bTrue(), B), B);
  rewrite.addRule(bEquals(bFalse(), B), bNot(B));
  rewrite.addRule(bEquals(B, bTrue()), B);
  rewrite.addRule(bEquals(B, bFalse()), bNot(B));

  rewrite.addRule(bAnd(B, B), B);
  rewrite.addRule(bOr(B, B), B);

  putRewriteSystem("simplifications", rewrite);
}

string parseRewriteSystem(string &s, int &w, RewriteSystem &rewrite)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "rewrite-system");
  skipWhiteSpace(s, w);
  string name = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    Term *t = parseTerm(s, w);
    skipWhiteSpace(s, w);
    matchString(s, w, "=>");
    skipWhiteSpace(s, w);
    Term *tp = parseTerm(s, w);
    Log(INFO) << "Parsed rewrite rule: " << t->toString() << " => " << tp->toString() << endl;
    rewrite.addRule(t, tp);
    skipWhiteSpace(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more rewrite rules", w, s);
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
  return name;
}

string parseConstrainedRewriteSystem(string &s, int &w, ConstrainedRewriteSystem &crewrite)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "constrained-rewrite-system");
  skipWhiteSpace(s, w);
  string name = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    ConstrainedTerm t = parseConstrainedTerm(s, w);
    skipWhiteSpace(s, w);
    matchString(s, w, "=>");
    skipWhiteSpace(s, w);
    Term *tp = parseTerm(s, w);
    Log(INFO) << "Parsed rewrite rule: " << t.toString() << " => " << tp->toString() << endl;
    crewrite.addRule(t, tp);
    skipWhiteSpace(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more constrained rewrite rules", w, s);
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    }
    else {
      match(s, w, ';');
      break;
    }
  }
  return name;
}

/*
int testFastTerm()
{
  char buffer[1024];

  FastSort sort = newSort("State");
  FastVar x1 = newVar("x1", sort);
  FastVar x2 = newVar("x2", sort);
  // FastVar x3 = newVar("x3", sort);
  FastFunc e = newConst("e", sort);

  FastSort sorts[16];
  for (int i = 0; i < 16; ++i) {
    sorts[i] = sort;
  }

  FastFunc i = newFunc("i", sort, 1, sorts);
  FastFunc f = newFunc("f", sort, 2, sorts);
  FastFunc g = newFunc("g", sort, 3, sorts);

  FastTerm array[16];

  array[0] = x1;
  FastTerm t1 = newFuncTerm(i, array);
  printTerm(t1, buffer, 1024);
  printf("t1 = %s\n", buffer);

  array[0] = x1;
  array[1] = x2;
  FastTerm t2 = newFuncTerm(f, array);
  printTerm(t2, buffer, 1024);
  printf("t2 = %s\n", buffer);

  array[0] = t1;
  array[1] = t2;
  FastTerm t3 = newFuncTerm(f, array);
  printTerm(t3, buffer, 1024);
  printf("t3 = %s\n", buffer);

  FastTerm t4 = newFuncTerm(e, array);
  printTerm(t4, buffer, 1024);
  printf("t4 = %s\n", buffer);

  array[0] = t1;
  array[1] = t2;
  array[2] = t4;
  FastTerm t5 = newFuncTerm(g, array);
  printTerm(t5, buffer, 1024);
  printf("t5 = %s\n", buffer);
  return 0;
}
*/

/*

Entry point to the RMT tool.

*/
int main(int argc, char **argv)
{
  if (0) {
    //testFastTerm();
  }
  bool readFromStdin = false;
  char *filename = argv[argc - 1];
  for (int i = 1; i < argc; ++i) {
    if (i < argc - 1 && strcmp(argv[i], "-v") == 0) {
      char *end;
      Log(INFO) << "Got verbosity level " << argv[2] << "." << endl;
      Log::debug_level = strtol(argv[i + 1], &end, 10);
      if (*end) {
        abortWithMessage("Syntax: ./rmt [-v <level>] file.in");
      }
    }
    if (strcmp(argv[i], "--in") == 0) {
      readFromStdin = true;
    }
  }
  // if (argc != 2) {
  //   if (argc == 4) {
  //     if (strcmp(argv[1], "-v") != 0) {
  //       abortWithMessage("Syntax: ./rmt [-v <level>] file.in");
  //     }
  //     char *end;
  //     Log(INFO) << "Got verbosity level " << argv[2] << "." << endl;
  //     Log::debug_level = strtol(argv[2], &end, 10);
  //     if (*end) {
  //       abortWithMessage("Syntax: ./rmt [-v <level>] file.in");
  //     }
  //     filename = argv[3];
  //   } else {
  //     abortWithMessage("Syntax: ./rmt [-v <level>] file.in");
  //   }
  // } else {
  //   filename = argv[1];
  // }

  ifstream inputf;
  if (!readFromStdin) {
    inputf.open(filename);
    if (!inputf.is_open()) {
      abortWithMessage(string("Cannot open file ") + filename + ".");
    }
  }
  istream &input = readFromStdin ? cin : inputf;
  string s;
  int w = 0;

  while (!input.eof()) {
    string tmp;
    getline(input, tmp);
    s += tmp;
    s += "\n";
  }
  if (!readFromStdin) {
    inputf.close();
  }

  start_z3_api();

  addPredefinedSorts();
  addPredefinedFunctions();
  createBuiltIns();
  addPredefinedRewriteSystems();

  for (skipWhiteSpace(s, w); w < len(s); skipWhiteSpace(s, w)) {
    Query *query = NULL;
    if (lookAhead(s, w, "sorts")) parseSorts(s, w);
    else if (lookAhead(s, w, "subsort")) parseSubsorts(s, w);
    else if (lookAhead(s, w, "signature")) parseFunctions(s, w);
    else if (lookAhead(s, w, "variables")) parseVariables(s, w);
    else if (lookAhead(s, w, "builtins")) parseBuiltins(s, w);
    else if (lookAhead(s, w, "assert")) parseAsserts(s, w);
    else if (lookAhead(s, w, "define")) parseDefinedFunctions(s, w);
    else if (lookAhead(s, w, "rewrite-system")) {
      RewriteSystem rewrite;
      string name = parseRewriteSystem(s, w, rewrite);
      putRewriteSystem(name, rewrite);
      skipWhiteSpace(s, w);
    }
    else if (lookAhead(s, w, "constrained-rewrite-system")) {
      ConstrainedRewriteSystem crewrite;
      string name = parseConstrainedRewriteSystem(s, w, crewrite);
      putConstrainedRewriteSystem(name, crewrite);
      skipWhiteSpace(s, w);
    }
    else if ((query = Query::lookAheadQuery(s, w))) {
      query->parse(s, w);
      skipWhiteSpace(s, w);
      query->execute();
    }
    else if (lookAhead(s, w, "!EOF!")) {
      break;
    }
    else {
      expected("valid command", w, s);
    }
  }
  printDebugInformation();
  return 0;
}

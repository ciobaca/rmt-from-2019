#ifndef PARSE_H__
#define PARSE_H__

#include <string>
#include "term.h"
#include "constrainedterm.h"

using namespace std;

ConstrainedTerm parseConstrainedTerm(string &s, int &w);
Term *parseTerm(string &s);
Term *parseTerm(const char *s);
void matchString(string &s, int &pos, string what);
void match(string &s, int &pos, char c);
void skipWhiteSpace(string &s, int &pos);
string getQuotedString(string &s, int &pos);
bool lookAhead(string &s, int &pos, string what);
bool lookAheadIdentifier(string &s, int &pos);
string getIdentifier(string &s, int &pos);
int getNumber(string &s, int &pos);
Term *parseTerm(string &s, int &pos);
void expected(string what, int &where, string text);
void parseError(string error, int &where, string text);
ConstrainedRewriteSystem parseCRSfromName(string &s, int &w);
ConstrainedPair parseConstrainedPair(string &s, int &w);

#endif

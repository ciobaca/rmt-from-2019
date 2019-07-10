#ifndef LOG_H__
#define LOG_H__

#include <iostream>
#include <fstream>

const int ERROR = 0;
const int WARNING = 1;
const int INFO = 2;
const int DEBUG = 3;
const int DEBUG1 = 4;
const int DEBUG2 = 5;
const int DEBUG3 = 6;
const int DEBUG4 = 7;
const int DEBUG5 = 8;
const int DEBUG6 = 9;
const int DEBUG7 = 10;
const int DEBUG8 = 11;
const int DEBUG9 = 12;

std::string levelToString(const int);

enum Feature
{
  LOGSAT
};

struct Log
{
  static int debug_level;
  int level;
  bool hasFeature;
  Feature feature;

  static const bool debugSatisfiability;

  Log(int);
  Log(Feature f);

  bool loggingEnabled();

  template<class T> Log &operator<<(const T &t)
  {
    if (loggingEnabled()) {
      std::cerr << t;
    }
    return *this;
  }

  typedef std::basic_ostream<char, std::char_traits<char> > tcerr;
  typedef tcerr &(*stdeol)(tcerr &);
  
  Log& operator<<(stdeol);
};

#endif

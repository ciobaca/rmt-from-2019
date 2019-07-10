#include "log.h"
#include <iostream>
#include <string>
#include <cassert>

using namespace std;

int Log::debug_level = INFO;

const bool Log::debugSatisfiability = false;

bool Log::loggingEnabled()
{
  if (hasFeature) {
    switch (feature) {
    case LOGSAT:
      return debugSatisfiability;
    default:
      assert(0);
	  return false;
    }
  } else {
    return level <= debug_level;
  }
}

std::string levelToString(const int level)
{
  if (level == ERROR) {
    return "ERR ";
  } else if (level == WARNING) {
    return "WRN ";
  } else if (level == INFO) {
    return "INF ";
  } else if (level == DEBUG) {
    return "DBG ";
  } else if (level == DEBUG1) {
    return "DB1 ";
  } else if (level == DEBUG2) {
    return "DB2 ";
  } else if (level == DEBUG3) {
    return "DB3 ";
  } else if (level == DEBUG4) {
    return "DB4 ";
  } else if (level == DEBUG5) {
    return "DB5 ";
  } else if (level == DEBUG6) {
    return "DB6 ";
  } else if (level == DEBUG7) {
    return "DB7 ";
  } else if (level == DEBUG8) {
    return "DB8 ";
  } else if (level == DEBUG9) {
    return "DB9 ";
  } else {
    return "OTH ";
  }
}

Log::Log(int level) :
  level(level),
  hasFeature(false)
{
  if (loggingEnabled()) {
    std::cerr << levelToString(level);
  }
}

Log::Log(Feature feature) :
  level(0),
  hasFeature(true),
  feature(feature)
{
  if (loggingEnabled()) {
    std::cerr << levelToString(level);
  }
}

Log& Log::operator<<(stdeol m)
{
  if (loggingEnabled()) {
    m(std::cerr);
  }
  return *this;
}

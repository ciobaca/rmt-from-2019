#ifndef QUERY_H__
#define QUERY_H__

#include <string>
#include <map>

struct Query;

typedef Query *(*QueryCreator)();

struct Query
{
private:
  static std::map<std::string, QueryCreator> *commands;

public:
  static void registerQueryCreator(std::string command,
				   QueryCreator constructor);
 
  static Query *lookAheadQuery(std::string &s, int &w);
  
  virtual Query *create() = 0;
  virtual void parse(std::string &, int &) = 0;
  virtual void execute() = 0;
};

#endif

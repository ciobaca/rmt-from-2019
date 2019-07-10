#include "query.h"
#include "parse.h"
#include "queryimplies.h"
#include "queryinstrument.h"
#include "queryinstrument_c.h"
#include "queryunify.h"
#include "queryunifypatweg.h"
#include "queryunifyal.h"
#include "queryunifymodulotheories.h"
#include "querycunify.h"
#include "queryacuunify.h"
#include "queryacsunify.h"
#include "queryacunify.h"
#include "queryaccunify.h"
#include "querycompute.h"
#include "queryrun.h"
#include "queryrundefined.h"
#include "querysearch.h"
#include "querydefinedsearch.h"
#include "querydefinedrewrite.h"
#include "queryder.h"
#include "querysubs.h"
#include "queryaxiom.h"
#include "querycirc.h"
#include "queryprovereachability.h"
#include "queryproveequivalence.h"
#include "querysatisfiability.h"
#include "querysimplification.h"
#include "queryprovesim.h"
#include "queryprovesim2.h"

#include <string>
#include <map>

using namespace std;

void Query::registerQueryCreator(std::string command,
             QueryCreator constructor)
{
  (*commands)[command] = constructor;
}
 
Query *Query::lookAheadQuery(std::string &s, int &w)
{
  typedef std::map<std::string, QueryCreator>::iterator it_type;
    
  for(it_type it = (*commands).begin(); it != (*commands).end(); it++) {
    string command = it->first; 
   if (lookAhead(s, w, command)) {
      QueryCreator nqc = it->second;
      return nqc();
    }
  }
  return 0;
}

Query *createQueryDer()
{
  return new QueryDer();
}

Query *createQuerySubs()
{
  return new QuerySubs();
}

Query *createQueryAxiom()
{
  return new QueryAxiom();
}

Query *createQueryCirc()
{
  return new QueryCirc();
}

Query *createQuerySearch()
{
  return new QuerySearch();
}

Query *createQueryDefinedSearch()
{
  return new QueryDefinedSearch();
}

Query *createQueryDefinedRewrite()
{
  return new QueryDefinedRewrite();
}

Query *createQueryUnify()
{
  return new QueryUnify();
}

Query *createQueryUnifyAL()
{
  return new QueryUnifyAL();
}

Query *createQueryUnifyPatWeg()
{
  return new QueryUnifyPatWeg();
}

Query *createQueryUnifyModuloTheories()
{
  return new QueryUnifyModuloTheories();
}

Query *createQueryCUnify()
{
  return new QueryCUnify();
}

Query *createQueryACUUnify()
{
  return new QueryACUUnify();
}

Query *createQueryACSUnify()
{
  return new QueryACSUnify();
}

Query *createQueryACUnify()
{
  return new QueryACUnify();
}

Query *createQueryACCUnify()
{
  return new QueryACCUnify();
}

Query *createQueryCompute()
{
  return new QueryCompute();
}

Query *createQueryImplies()
{
  return new QueryImplies();
}

Query *createQueryInstrument()
{
  return new QueryInstrument();
}

Query *createQueryInstrument_C()
{
  return new QueryInstrument_C();
}

Query *createQueryProveReachability()
{
  return new QueryProveReachability();
}

Query *createQueryProveEquivalence()
{
  return new QueryProveEquivalence();
}

Query *createQueryProveSim()
{
  return new QueryProveSim();
}

Query *createQueryProveSim2()
{
  return new QueryProveSim2();
}

Query *createQuerySatisfiability()
{
  return new QuerySatisfiability();
}

Query *createQuerySimplification()
{
  return new QuerySimplification();
}

Query *createQueryRun()
{
  return new QueryRun();
}

Query *createQueryRunDefined()
{
  return new QueryRunDefined();
}

std::map<std::string, QueryCreator> *registerQueries()
{
  std::map<std::string, QueryCreator> *map;
  map = new std::map<std::string, QueryCreator>;
  (*map)["search"] = createQuerySearch;
  (*map)["definedsearch"] = createQueryDefinedSearch;
  (*map)["definedrewrite"] = createQueryDefinedRewrite;
  (*map)["der"] = createQueryDer;
  (*map)["subs"] = createQuerySubs;
  (*map)["axiom"] = createQueryAxiom;
  (*map)["circ"] = createQueryCirc;
  (*map)["n*unify"] = createQueryUnifyAL;
  (*map)["pat-weg-unify"] = createQueryUnifyPatWeg;
  (*map)["unify"] = createQueryUnify;
  (*map)["unifyModuloTheories"] = createQueryUnifyModuloTheories;
  (*map)["c-unify"] = createQueryCUnify;
  (*map)["ac-unify"] = createQueryACUnify;
  (*map)["acu-unify"] = createQueryACUUnify;
  (*map)["acs-unify"] = createQueryACSUnify;
  (*map)["acc-unify"] = createQueryACCUnify;
  (*map)["implies"] = createQueryImplies;
  (*map)["instrument"] = createQueryInstrument;
  (*map)["cinstrument"] = createQueryInstrument_C;
  (*map)["prove"] = createQueryProveReachability;
  (*map)["show-equivalent"] = createQueryProveEquivalence;
  (*map)["show-simulation"] = createQueryProveSim;
  (*map)["show-simulation2"] = createQueryProveSim2;
  (*map)["run"] = createQueryRun;
  (*map)["rundefined"] = createQueryRunDefined;
  (*map)["compute"] = createQueryCompute;

  // SMT related queries
  (*map)["satisfiability"] = createQuerySatisfiability;
  (*map)["simplification"] = createQuerySimplification;
  return map;
}

std::map<std::string, QueryCreator> *Query::commands = registerQueries();  

#include "queryprovesim2.h"
#include "parse.h"
#include "log.h"
#include "constrainedrewritesystem.h"
#include "rewritesystem.h"
#include "factories.h"
#include "term.h"
#include "funterm.h"
#include <string>
#include <cassert>
#include <queue>
using namespace std;

QueryProveSim2::QueryProveSim2() {
}

Query *QueryProveSim2::create() {
  return new QueryProveSim2();
}

void QueryProveSim2::parse(std::string &s, int &w) {
  matchString(s, w, "show-simulation2");
  skipWhiteSpace(s, w);

  /* defaults */
  boundL = boundR = 30;

  matchString(s, w, "in");
  crsLeft = parseCRSfromName(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, "and");
  crsRight = parseCRSfromName(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, ":");
  skipWhiteSpace(s, w);

  do {
    ConstrainedPair ctp = parseConstrainedPair(s, w);
    circularities.push_back(ctp);
    skipWhiteSpace(s, w);
    if (lookAhead(s, w, ",")) {
      matchString(s, w, ",");
      skipWhiteSpace(s, w);
    }
    else {
      break;
    }
  } while (1);

  matchString(s, w, "with-base");
  do {
    ConstrainedPair ctp = parseConstrainedPair(s, w);
    base.push_back(ctp);
    if (lookAhead(s, w, ",")) {
      matchString(s, w, ",");
      skipWhiteSpace(s, w);
    }
    else {
      break;
    }
  } while (1);
  matchString(s, w, ";");
}

// solves phi(x) -> (P(x), Q(x)) \in [base]
Term *QueryProveSim2::whenBase(ConstrainedPair PQphi, vector<ConstrainedPair> &base) {
  ConstrainedTerm P = ConstrainedTerm(PQphi.lhs, PQphi.constraint);
  ConstrainedTerm Q = ConstrainedTerm(PQphi.rhs, PQphi.constraint);
  Term *psi = bFalse();
  for(const auto &circ : base) {
    Term *constraint = simplifyConstraint(bAnd(
      P.whenImplies(ConstrainedTerm(circ.lhs, circ.constraint)),
      Q.whenImplies(ConstrainedTerm(circ.rhs, circ.constraint)))
    );
    psi = simplifyConstraint(bOr(psi, constraint));
  }
  return psi;
}

// solves psi(x) = phi(x) -> ( exists Q' . Q(x) |-> Q' /\ (P(x), Q') \in [base] )
Term *QueryProveSim2::baseCase(ConstrainedPair PQphi, vector<ConstrainedPair> &base, int ldepth, int depth, int stepsRequired) {

  if (depth - ldepth >= boundR) {
    if (depth >= boundL) {
      cout << spaces(depth) << "! done searching (exceeded maximum depth) " << PQphi.toString() << endl;
      return bFalse();
    }
  }

  cout << spaces(depth) << "+ searching for base " << PQphi.toString() << endl;

  Term *psi = bFalse();

  if (stepsRequired <= 0) {
    psi = whenBase(PQphi, base);
  }

  if (psi != bFalse())
    cout << spaces(depth) << "in base when " << psi->toString() << endl;

  ConstrainedTerm rhs = ConstrainedTerm(PQphi.rhs, PQphi.constraint);

  vector<ConstrainedSolution> solutions = rhs.smtNarrowSearch(crsRight);
  --stepsRequired;
  if (solutions.empty()) {
    cout << spaces(depth) << "no successors, taking defined symbols" << "(" << rhs.toString() << ")" << endl;
    solutions = rhs.smtNarrowDefinedSearch(depth);
    ++stepsRequired;
  }
  vector<ConstrainedTerm> successors = solutionsToSuccessors(solutions);

  for (const auto &succ : successors) {
    psi = simplifyConstraint(bOr(psi,
      baseCase(ConstrainedPair(
        PQphi.lhs, succ.term, simplifyConstraint(bAnd(PQphi.constraint, succ.constraint))
      ), base, ldepth, depth + 1, stepsRequired)));
  }

  cout << spaces(depth) << "final condition " << psi->toString() << endl;

  return psi;
}

Term *QueryProveSim2::prove(ConstrainedPair PQphi, int depth) {
  if (depth >= boundL) {
    cout << spaces(depth) << "! proof failed (exceeded maximum depth) " << PQphi.toString() << endl;
    return bFalse();
  }

  cout << spaces(depth) << "+ proving " << PQphi.toString() << endl;

  Term *phi_B = baseCase(PQphi, base, depth, depth, 0);
  cout << spaces(depth) << "in base when " << phi_B->toString() << endl;
  Term *phi_C = baseCase(PQphi, circularities, depth, depth, 1);
  cout << spaces(depth) << "in circs when " << phi_B->toString() << endl;

  Term *phi_succs = bTrue();

  ConstrainedTerm lhs = ConstrainedTerm(PQphi.lhs, PQphi.constraint);

  vector<ConstrainedSolution> solutions = lhs.smtNarrowSearch(crsLeft);
  if (solutions.empty()) {
    cout << spaces(depth) << "no successors, taking defined symbols" << "(" << lhs.toString() << ")" << endl;
    solutions = lhs.smtNarrowDefinedSearch(depth);
  }
  vector<ConstrainedTerm> successors = solutionsToSuccessors(solutions);

  for (const auto &succ : successors) {
    phi_succs = simplifyConstraint(bAnd(phi_succs,
      prove(ConstrainedPair(
        succ.term, PQphi.rhs, simplifyConstraint(bAnd(PQphi.constraint, succ.constraint))
      ), depth + 1)));
  }

  cout << spaces(depth) << "successors when " << phi_B->toString() << endl;

  Term *result = simplifyConstraint(bOr(bOr(phi_B, phi_C), phi_succs));

  cout << spaces(depth) << "overall resulting constraint " << phi_B->toString() << endl;

  return result;
}

void QueryProveSim2::expandCircDefined(int from, int to) {
  for (int i = from; i < to; ++i) {
    ConstrainedPair *circ = &circularities[i];
    vector<ConstrainedTerm>
      csols_lhs = ConstrainedTerm(circ->lhs, circ->constraint).smtNarrowDefinedSearch(0, 1),
      csols_rhs = ConstrainedTerm(circ->rhs, circ->constraint).smtNarrowDefinedSearch(0, 1);
    for (int j = 0; j < (int)csols_lhs.size(); ++j)
      for (int k = 0; k < (int)csols_rhs.size(); ++k) {
        if (j == 0 && k == 0) {
          //this is not a new circularity
          continue;
        }
        ConstrainedTerm *L = &csols_lhs[j], *R = &csols_rhs[k];
        ConstrainedPair newCirc(L->term, R->term, simplifyConstraint(bAnd(L->constraint, R->constraint)));
        Log(INFO) << "Adding new circ (not necessary to prove) " << newCirc.toString() << endl;
        circularities.push_back(newCirc);
      }
  }
}

void QueryProveSim2::execute() {
  Log(DEBUG9) << "Proving simulation (new algorithm)" << endl;

  int circCount = (int)circularities.size();

  // expand all defined functions in circularities (twice)
  expandCircDefined(0, circCount);
  expandCircDefined(circCount, (int)circularities.size());
  Log(INFO) << "Number of circularities post-expansion " << circularities.size() << endl;

  for (int i = 1; i <= circCount; ++i) {
    cout << "Proving simulation circularity #" << i << endl;
    if (isValid(prove(circularities[i - 1], 0))) {
      cout << "Succeeded in proving circularity #" << i << endl;
    }
    else {
      cout << "Failed to prove circularity #" << i << endl;
      failedCircularities.push_back(i);
    }
  }
}

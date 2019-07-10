#ifndef LDELEXALG_H__
#define LDELEXALG_H__
#include <vector>
#include <utility>
#include "ldealg.h"

using namespace std;

struct LDELexAlg : public LDEAlg {
  LDELexAlg();
  LDELexAlg(const vector<int> &a, const vector<int> &b, int c = 0, int boundFlag = 0);
  void lexAlg0(int poz, int diff);
  void lexAlg1(int poz, int diff, int suma = 0, int sumb = 0);
  void lexAlg2(int poz, int diff, int suma = 0, int sumb = 0);
  vector<pair<vector<int>, vector<int>>> solve();

  private:
    int boundFlag, maxa, maxb;
    vector<int> partialSol, vab;

    void extendedEuclid(int a, int b, int &x, int &y, int &g);
};

#endif
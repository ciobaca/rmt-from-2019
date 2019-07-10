#ifndef LDECOMPALG_H__
#define LDECOMPALG_H__
#include <vector>
#include <utility>
#include "ldealg.h"

using namespace std;

struct LDECompAlg : public LDEAlg {
  LDECompAlg();
  LDECompAlg(const vector<int> &a, const vector<int> &b, int c = 0, bool withOrderOnComputations = true);
  void compAlg0();
  void compAlg1();
  vector<pair<vector<int>, vector<int>>> solve();

  private:
    bool withOrderOnComputations;
};

#endif
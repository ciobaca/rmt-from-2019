#ifndef LDESLOPESALG_H__
#define LDESLOPESALG_H__
#include <vector>
#include <utility>
#include "ldealg.h"

using namespace std;

struct LDESlopesAlg : public LDEAlg {
  LDESlopesAlg();
  LDESlopesAlg(const vector<int> &a, const vector<int> &b, int c = 0);
  void lexEnum(int poz, int diff, int suma = 0, int sumb = 0);
  vector<pair<vector<int>, vector<int>>> solve();

  private:
    int maxa, maxb;
    vector<int> partialSol, vab;

    int gcd(int a, int b);
    void extendedEuclid(int a, int b, int &x, int &y, int &g);
    int modInv(int a, int m);
    int getMultiplier(int a, int b, int ymax);
    int getMc(int a, int b, int c);
    vector<tuple<int, int, int>> solve(int a, int b, int c);
    vector<tuple<int, int, int>> solve(int a, int b, int c, int v);
};

#endif
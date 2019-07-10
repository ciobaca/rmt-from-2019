#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include "ldelexalg.h"

using namespace std;

LDELexAlg::LDELexAlg() {}

LDELexAlg::LDELexAlg(const vector<int> &a, const vector<int> &b, int c, int boundFlag) {
  if ((a.size() && *min_element(a.begin(), a.end()) <= 0) || (b.size() && *min_element(b.begin(), b.end()) <= 0)) {
    cout << "Wrong parameters: 'a' or 'b' contains a coefficient less or equal to zero" << endl;
    exit(0);
  }
  this->a = a;
  this->b = b;
  this->c = c;
  this->boundFlag = max(min(boundFlag, 2), 0);
  this->vab = a;
  this->vab.insert(this->vab.end(), b.begin(), b.end());
  for(int i = (int)a.size(); i < (int)this->vab.size(); ++i) {
    vab[i] *= -1;
  }

  maxa = this->a.size() ? *max_element(a.begin(), a.end()) : 0;
  maxb = this->b.size() ? *max_element(b.begin(), b.end()) : 0;
  partialSol.resize(a.size() + b.size());
}

void LDELexAlg::lexAlg0(int poz, int diff) {
  if (poz + 1 == (int)partialSol.size()) {
    if (diff * vab.back() >= 0 && diff % vab.back() == 0) {
      partialSol[poz] = diff / vab.back();
      if (isMinimal(partialSol)) {
        addSolution(partialSol);
      }
    }
    return;
  }

  int limit = poz < (int)a.size() ? maxb : maxa;
  for (int val = 0; val <= limit; ++val) {
    partialSol[poz] = val;
    lexAlg0(poz + 1, diff - val * vab[poz]);
  }
}

void LDELexAlg::lexAlg1(int poz, int diff, int suma, int sumb) {
  if (poz + 1 == (int)partialSol.size()) {
    if (diff * vab.back() >= 0 && diff % vab.back() == 0) {
      partialSol[poz] = diff / vab.back();
      if (isMinimal(partialSol)) {
        addSolution(partialSol);
      }
    }
    return;
  }

  if (poz < (int)a.size()) {
    for (int val = 0; suma + val <= maxb; ++val) {
      partialSol[poz] = val;
      lexAlg1(poz + 1, diff - val * vab[poz], suma + val, sumb);
    }
  } else {
    for (int val = 0; sumb + val <= maxa; ++val) {
      partialSol[poz] = val;
      lexAlg1(poz + 1, diff - val * vab[poz], suma, sumb + val);
    }
  }
}

void LDELexAlg::extendedEuclid(int a, int b, int &x, int &y, int &g) {
  int xx, yy;
  if (!b) {
    g = a;
    x = 1;
    y = 0;
  } else {
    extendedEuclid(b, a % b, xx, yy, g);
    x = yy;
    y = xx - (a / b) * yy;
  }
}

void LDELexAlg::lexAlg2(int poz, int diff, int suma, int sumb) {
  if (poz + 2 == (int)partialSol.size()) {
    int x, y, g;
    extendedEuclid(vab[(int)vab.size() - 2], vab.back(), x, y, g);
    if (!(diff % g) && (x >= 0 || y >= 0)) {
      int coef1 = vab.back() / g;
      int coef2 = vab[(int)vab.size() - 2] / g;
      if (x < 0) {
        int k = (-x + coef1 - 1) / coef1;
        x += k * coef1;
        y -= k * coef2;
      } else {
        int k = -((abs(y) + coef2 - 1) / coef2);
        x += k * coef1;
        y -= k * coef2;
      }

      if (x >= 0 && y >= 0) {
        partialSol[(int)vab.size() - 1] = y;
        partialSol[(int)vab.size() - 2] = x;
        if (isMinimal(partialSol)) {
          addSolution(partialSol);
        }
      }
    }
    return;
  }

  if (poz < (int)a.size()) {
    for (int val = 0; suma + val <= maxb; ++val) {
      partialSol[poz] = val;
      lexAlg2(poz + 1, diff - val * vab[poz], suma + val, sumb);
    }
  } else {
    for (int val = 0; sumb + val <= maxa; ++val) {
      partialSol[poz] = val;
      lexAlg2(poz + 1, diff - val * vab[poz], suma, sumb + val);
    }
  }
}

vector<pair<vector<int>, vector<int>>> LDELexAlg::solve() {
  basis.clear();
  switch (boundFlag) {
    case 0:
      lexAlg0(0, c);
      break;
    case 1:
      lexAlg1(0, c);
      break;
    case 2:
      lexAlg2(0, c);
      break;
  }
  return this->basis;
}
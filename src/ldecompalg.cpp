#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include "ldecompalg.h"

using namespace std;

LDECompAlg::LDECompAlg() {}

LDECompAlg::LDECompAlg(const vector<int> &a, const vector<int> &b, int c, bool withOrderOnComputations) {
  if ((a.size() && *min_element(a.begin(), a.end()) <= 0) || (b.size() && *min_element(b.begin(), b.end()) <= 0)) {
    cout << "Wrong parameters: 'a' or 'b' contains a coefficient less or equal to zero" << endl;
    exit(0);
  }
  this->a = a;
  this->b = b;
  this->c = c;
  this->withOrderOnComputations = withOrderOnComputations;
}

void LDECompAlg::compAlg0() {
  vector<pair<vector<int>, int>> p;
  for (int i = 0; i < (int)a.size(); ++i) {
    vector<int> aux(a.size() + b.size(), 0);
    aux[i] = 1;
    if (c == a[i]) {
      addSolution(aux);
    } else {
      p.emplace_back(aux, c - a[i]);
    }
  }

  while (p.size()) {
    vector<pair<vector<int>, int>> newp;
    for (auto &it : p) {
      if (it.second < 0) {
        for (int i = 0; i < (int)b.size(); ++i) {
          vector<int> aux = it.first;
          ++aux[a.size() + i];
          if (it.second == -b[i]) {
            addSolution(aux);
          } else {
            if (isMinimal(aux)) {
              newp.emplace_back(aux, it.second + b[i]);
            }
          }
        }
      } else {
        for (int i = 0; i < (int)a.size(); ++i) {
          vector<int> aux = it.first;
          ++aux[i];
          if (it.second == a[i]) {
            addSolution(aux);
          } else {
            if (isMinimal(aux)) {
              newp.emplace_back(aux, it.second - a[i]);
            }
          }
        }
      }
    }

    p.swap(newp);
    sort(p.begin(), p.end());
    p.erase(unique(p.begin(), p.end()), p.end());
  }
}

void LDECompAlg::compAlg1() {
  vector<pair<vector<int>, int>> p;
  for (int i = 0; i < (int)a.size(); ++i) {
    vector<int> aux(a.size() + b.size(), 0);
    aux[i] = 1;
    if (c == a[i]) {
      addSolution(aux);
    } else {
      p.emplace_back(aux, c - a[i]);
    }
  }

  while (p.size()) {
    vector<pair<vector<int>, int>> newp;
    for (auto &it : p) {
      if (it.second < 0) {
        for (int i = (int)b.size() - 1; i >= 0; --i) {
          vector<int> aux = it.first;
          ++aux[a.size() + i];
          if (it.second == -b[i]) {
            addSolution(aux);
          } else {
            if (isMinimal(aux)) {
              newp.emplace_back(aux, it.second + b[i]);
            }
          }

          if (it.first[a.size() + i]) {
            break;
          }
        }
      } else {
        for (int i = (int)a.size() - 1; i >= 0; --i) {
          vector<int> aux = it.first;
          ++aux[i];
          if (it.second == a[i]) {
            addSolution(aux);
          } else {
            if (isMinimal(aux)) {
              newp.emplace_back(aux, it.second - a[i]);
            }
          }

          if (it.first[i]) {
            break;
          }
        }
      }
    }

    p.swap(newp);
  }
}

vector<pair<vector<int>, vector<int>>> LDECompAlg::solve() {
  basis.clear();
  if (!withOrderOnComputations) {
    compAlg0();
    sort(basis.begin(), basis.end());
    basis.erase(unique(basis.begin(), basis.end()), basis.end());
  } else {
    compAlg1();
  }
  return this->basis;
}
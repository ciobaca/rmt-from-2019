#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <tuple>
#include "ldeslopesalg.h"

using namespace std;

LDESlopesAlg::LDESlopesAlg() {}

LDESlopesAlg::LDESlopesAlg(const vector<int> &a, const vector<int> &b, int c) {
  if (!a.size() || !b.size()) {
    cout << "Wrong parameters: 'a' or 'b' is empty" << endl;
    exit(0);
  }
  if ((a.size() && *min_element(a.begin(), a.end()) <= 0) || (b.size() && *min_element(b.begin(), b.end()) <= 0)) {
    cout << "Wrong parameters: 'a' or 'b' contains a coefficient less or equal to zero" << endl;
    exit(0);
  }
  this->a = a;
  this->b = b;
  this->c = c;
  this->vab = this->a;
  this->vab.insert(this->vab.end(), this->b.begin(), this->b.end());
  for(int i = (int)a.size(); i < (int)this->vab.size(); ++i) {
    vab[i] *= -1;
  }
  maxa = *max_element(a.begin(), a.end());
  maxb = *max_element(b.begin(), b.end());
  partialSol.resize(a.size() + b.size());
}

int LDESlopesAlg::gcd(int a, int b) {
  while (b) {
    int c = b;
    b = a % b;
    a = c;
  }
  return a;
}

void LDESlopesAlg::extendedEuclid(int a, int b, int &x, int &y, int &g) {
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

// returns a^-1 mod m, when gcd(a, m) == 1
int LDESlopesAlg::modInv(int a, int m) {
  return a == 1 ? 1 : (1 - modInv(m % a, a) * m) / a + m;
}

// returns a number <mb>, such that gcd(a, b) == ma * a + <mb> * b
// this is the same as saying that mb is the inverse of b / gcd(a, b) in Z_{ymax}
int LDESlopesAlg::getMultiplier(int a, int b, int ymax) {
  if(!b) return 0;
  return modInv(b / gcd(a, b), ymax);
}

// returns c that satisfies a * x + b * y + c * z = gcd(x, y, z), here x, y, z are known constants
// Solution: gcd(x, y, z) = gcd(x, gcd(y, z))
// gcd(x, gcd(y, z)) = ax + b * gcd(y, z) (1), gcd(y, z) = cy + dz (2)
// (1) + (2) -> gcd(x, y, z) = ax + bcy + bdz
// Note: it's ok to return a negative value
int LDESlopesAlg::getMc(int x, int y, int z) {
  int a, b, c, d, g;
  extendedEuclid(y, z, c, d, g);
  extendedEuclid(x, gcd(y, z), a, b, g);
  return b * d;
}

// solves a * x == b * y + c * z
// returns the set of minimal solutions (a basis, in other words)
vector<tuple<int, int, int>> LDESlopesAlg::solve(int a, int b, int c) {
  vector<tuple<int, int, int>> basis;
  int gb = gcd(a, b);
  int gc = gcd(a, c);
  int gabc = gcd(gb, c);
  int ymax = a / gb;
  int zmax = a / gc;
  int dz = gb / gabc;
  int dy = (c * getMultiplier(a, b, ymax) / gabc) % ymax;
  int y = ymax - dy;
  int z = dz;
  basis.emplace_back(b / gb, ymax, 0);
  if (y != ymax) {
    basis.emplace_back((b * y + c * z) / a, y, z);
  }
  while (dy > 0) {
    while (y > dy) {
      y -= dy;
      z += dz;
      basis.emplace_back((b * y + c * z) / a, y, z);
    }
    dz += (dy / y) * z;
    dy %= y;
  }
  basis.emplace_back(c / gc, 0, zmax);
  return basis;
}

vector<tuple<int, int, int>> LDESlopesAlg::solve(int a, int b, int c, int v) {
  if (!v) {
    return solve(a, b, c);
  }
  vector<tuple<int, int, int>> basis;
  int gb = gcd(a, b);
  int gabc = gcd(gb, c);
  if (v % gabc) {
    return basis;
  }
  int gc = gcd(a, c);
  int ymax = a / gb;
  int zmax = a / gc;
  int mb = getMultiplier(a, b, ymax);
  int dz = gb / gabc;
  int dy = (c * mb / gabc) % ymax;
  int z = ((-v * getMc(a, b, c) / gabc) % dz + dz) % dz;
  int y = ((((-v - z * c) * mb / gb) % ymax) + ymax) % ymax;
  y += ymax * max(0, (-b * y - c * z - v + b * ymax + 1) / (b * ymax));
  int x = (b * y + c * z + v) / a;

  auto isLess = [&](tuple<int, int, int> &x, tuple<int, int, int> &y) {
    return (get<0>(x) <= get<0>(y)) && (get<1>(x) <= get<1>(y)) && (get<2>(x) <= get<2>(y));
  };

  if (v < 0) {
    int coef = (-c * z - v - b * y) / (b * ymax) + ((-c * z - v - b * y) % (b * ymax) > 0);
    coef = max(coef, -y / ymax);
    y += coef * ymax;
    tuple<int, int, int> now = make_tuple((b * y + c * z + v) / a, y, z);
    basis.push_back(now);
    int p = ymax;
    zmax = max(zmax, -v);
    while (z <= zmax && p > 0) {
      if (y < p && z >= 0 && y >= 0) {
        coef = (-c * z - v - b * y) / (b * ymax) + ((-c * z - v - b * y) % (b * ymax) > 0);
        coef = max(coef, -y / ymax);
        y += coef * ymax;
        now = make_tuple((b * y + c * z + v) / a, y, z);
        while(basis.size() && isLess(now, basis.back())) basis.pop_back();
        if (!basis.size() || !isLess(basis.back(), now)) {
          basis.push_back(now);
        }
      }
      z += dz;
      y = (y - dy + ymax) % ymax;
    }
    coef = (-c * z - v - b * y) / (b * ymax) + ((-c * z - v - b * y) % (b * ymax) > 0);
    coef = max(coef, -y / ymax);
    y += coef * ymax;
    now = make_tuple((b * y + c * z + v) / a, y, z);
    while(basis.size() && isLess(now, basis.back())) basis.pop_back();
    if (!basis.size() || !isLess(basis.back(), now)) {
      basis.push_back(now);
    }
  } else {
    vector<tuple<int, int, int>> sols = solve(ymax, ymax - 1, dy);
    basis.emplace_back(x, y, z);
    for (int i = 0; i < (int)sols.size(); ++i) {
      dy = get<1>(sols[i]);
      if (!dy) {
        break;
      }
      int k = get<2>(sols[i]);
      while (dy <= y) {
        y -= dy;
        z += k * dz;
        x += (-b * dy + c * k * dz) / a;
        tuple<int, int, int> now = make_tuple(x, y, z);
        while(basis.size() && isLess(now, basis.back())) basis.pop_back();
        if (!basis.size() || !isLess(basis.back(), now)) {
          basis.emplace_back(x, y, z);
        }
      }
    }
  }
  return basis;
}

void LDESlopesAlg::lexEnum(int poz, int diff, int suma, int sumb) {
  if (poz + 1 == (int)a.size()) {
    lexEnum(poz + 1, diff, suma, sumb);
    return;
  }
  if (poz + 2 == (int)partialSol.size()) {
    vector<tuple<int, int, int>> sols;
    if (!diff && suma + sumb) {
      sols.emplace_back(0, 0, 0);
    } else {
      sols = solve(a.back(), b[b.size() - 2], b.back(), diff);
    }
    for (auto it : sols) {
      int x, y, z;
      tie(x, y, z) = it;
      partialSol[a.size() - 1] = x;
      partialSol[vab.size() - 2] = y;
      partialSol[vab.size() - 1] = z;
      if (isMinimal(partialSol)) {
        addSolution(partialSol);
      }
    }
    return;
  }
  if (poz < (int)a.size()) {
    for (int val = 0; suma + val <= maxb; ++val) {
      partialSol[poz] = val;
      lexEnum(poz + 1, diff - val * vab[poz], suma + val, sumb);
    }
  } else {
    for (int val = 0; sumb + val <= maxa; ++val) {
      partialSol[poz] = val;
      lexEnum(poz + 1, diff - val * vab[poz], suma, sumb + val);
    }
  }
}

vector<pair<vector<int>, vector<int>>> LDESlopesAlg::solve() {
  this->basis.clear();
  lexEnum(0, c);
  return this->basis;
}
#ifndef KIWI_MATH_PRIMES_H
#define KIWI_MATH_PRIMES_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iterator>

class PrimeGenerator {
 public:
  static int nthPrime(int n);
  static std::vector<int> listPrimes(int n);

 private:
  static void simpleSieve(int limit, std::vector<int>& primes);
  static std::vector<int> segmentedSieve(int n);
};

void PrimeGenerator::simpleSieve(int limit, std::vector<int>& primes) {
  std::vector<bool> mark(limit + 1, true);

  for (int p = 2; p * p < limit; p++) {
    if (mark[p]) {
      for (int i = p * p; i < limit; i += p) {
        mark[i] = false;
      }
    }
  }

  for (int p = 2; p < limit; p++) {
    if (mark[p]) {
      primes.push_back(p);
    }
  }
}

std::vector<int> PrimeGenerator::segmentedSieve(int n) {
  int limit = std::floor(std::sqrt(n)) + 1;
  std::vector<int> primes;
  std::vector<int> segPrimes;
  simpleSieve(limit, primes);

  int low = limit;
  int high = 2 * limit;
  int size = primes.size();

  while (low < n) {
    if (high >= n) {
      high = n;
    }

    std::vector<bool> mark(limit + 1, true);

    for (int i = 0; i < size; i++) {
      int loLim = std::floor(low / primes[i]) * primes[i];

      if (loLim < low) {
        loLim += primes[i];
      }

      for (int j = loLim; j < high; j += primes[i]) {
        mark[j - low] = false;
      }
    }

    for (int i = low; i < high; i++) {
      if (mark[i - low]) {
        segPrimes.push_back(i);
      }
    }

    low = low + limit;
    high = high + limit;
  }

  return segPrimes;
}

int PrimeGenerator::nthPrime(int n) {
  if (n == 0) {
    return 2;
  }
  int limit = std::floor(sqrt(n) * log(n)) + 1;
  std::vector<int> primes;
  simpleSieve(limit, primes);

  int count = primes.size();
  int low = limit;
  int high = 2 * limit;

  while (count < n + 1) {
    std::vector<bool> mark(limit + 1, true);

    for (int i = 0; i < static_cast<int>(primes.size()); i++) {
      int loLim = std::floor(low / primes[i]) * primes[i];

      if (loLim < low) {
        loLim += primes[i];
      }

      for (int j = loLim; j < high; j += primes[i]) {
        mark[j - low] = false;
      }
    }

    for (int i = low; i < high && count < n + 1; i++) {
      if (mark[i - low]) {
        if (count == n) {
          return i;
        }

        count++;
      }
    }

    low += limit;
    high += limit;
  }

  return -1;
}

std::vector<int> PrimeGenerator::listPrimes(int n) {
  return segmentedSieve(n);
}

#endif
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
  static std::vector<int> listPrimes(int limit);

 private:
  static void simpleSieve(int limit, std::vector<int>& primes);
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

std::vector<int> PrimeGenerator::listPrimes(int limit) {
  std::vector<bool> is_prime(limit + 1, true);
  std::vector<int> primes;

  is_prime[0] = is_prime[1] = false;  // 0 and 1 are not prime numbers

  for (int num = 2; num <= sqrt(limit); ++num) {
    if (is_prime[num]) {
      for (int multiple = num * num; multiple <= limit; multiple += num) {
        is_prime[multiple] = false;
      }
    }
  }

  for (int num = 2; num <= limit; ++num) {
    if (is_prime[num]) {
      primes.push_back(num);
    }
  }

  return primes;
}

#endif
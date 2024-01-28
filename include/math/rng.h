#ifndef KIWI_MATH_RNG_H
#define KIWI_MATH_RNG_H

#include <ctime>
#include <iostream>
#include <random>

class RNG {
 public:
  static RNG& getInstance();
  double random(double from, double to);
  std::string random16();

 private:
  RNG();
  std::mt19937 generator;  // Mersenne Twister engine
};

RNG& RNG::getInstance() {
  static RNG instance;
  return instance;
}

RNG::RNG() {
  std::seed_seq seed{static_cast<unsigned>(std::time(0))};
  generator.seed(seed);
}

double RNG::random(double from, double to) {
  std::uniform_real_distribution<double> distribution(from, to);
  return distribution(generator);
}

std::string RNG::random16() {
  const size_t LENGTH = 16;
  // Might trim it down a bit.
  const std::string chars =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::uniform_int_distribution<> distribution(0, chars.size() - 1);

  std::string randomString;
  for (size_t i = 0; i < LENGTH; ++i) {
    randomString += chars[distribution(generator)];
  }

  return randomString;
}

#endif
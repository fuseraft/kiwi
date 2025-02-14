#ifndef KIWI_MATH_RNG_H
#define KIWI_MATH_RNG_H

#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include "typing/value.h"

class RNG {
 public:
  static RNG& getInstance();
  double random(double from, double to);
  k_int random(k_int from, k_int to);
  k_string random8();
  k_string random16();
  k_string randomString(const k_string& input, size_t length);
  k_value randomList(k_list list, size_t length);

 private:
  RNG();
  std::mt19937 generator;  // Mersenne Twister engine
};

RNG& RNG::getInstance() {
  static RNG instance;
  return instance;
}

RNG::RNG() {
  std::random_device rd;
  std::seed_seq seed{rd(), static_cast<unsigned>(std::time(nullptr))};
  generator.seed(seed);
}

double RNG::random(double from, double to) {
  std::uniform_real_distribution<double> distribution(from, to);
  return distribution(generator);
}

k_int RNG::random(k_int from, k_int to) {
  std::uniform_int_distribution<k_int> distribution(from, to);
  return distribution(generator);
}

k_string RNG::random8() {
  const size_t LENGTH = 8;
  const k_string chars =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  return randomString(chars, LENGTH);
}

k_string RNG::random16() {
  const size_t LENGTH = 16;
  // Might trim it down a bit.
  const k_string chars =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  return randomString(chars, LENGTH);
}

k_string RNG::randomString(const k_string& chars, size_t length) {
  if (chars.empty()) {
    return chars;
  }

  std::uniform_int_distribution<> distribution(0, chars.size() - 1);
  std::ostringstream randomString;

  for (size_t i = 0; i < length; ++i) {
    randomString << chars[distribution(generator)];
  }

  return randomString.str();
}

k_value RNG::randomList(k_list list, size_t length) {
  const auto& elements = list->elements;
  if (elements.empty()) {
    return std::make_shared<List>();
  }

  std::uniform_int_distribution<> distribution(0, elements.size() - 1);
  auto randomList = std::make_shared<List>();
  auto& randomElements = randomList->elements;
  randomElements.reserve(length);

  for (size_t i = 0; i < length; ++i) {
    randomElements.emplace_back(elements.at(distribution(generator)));
  }

  return randomList;
}

#endif
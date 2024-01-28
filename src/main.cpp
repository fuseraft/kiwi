/**
 *  kiwi: an unorthodox scripting language
 *  hybrid-typed, object-oriented, interpreted
 **/

#include <vector>
#include "kiwi.h"
#include "globals.h"

bool DEBUG = false;

int main(int c, char** v) {
  std::vector<std::string> args;
  for (int i = 0; i < c; ++i) {
    args.push_back(v[i]);
  }

  return kiwi(args);
}
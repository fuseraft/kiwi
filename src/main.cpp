/**
 *  uslang: an unorthodox scripting language
 *  hybrid-typed, object-oriented, interpreted
 **/

#include <vector>
#include "include/uslang.h"
#include "include/globals.h"

bool DEBUG = false;

int main(int c, char** v) {
  int exit = 0;

  try {
    std::vector<std::string> args;
    for (int i = 0; i < c; ++i) {
      args.push_back(v[i]);
    }

    if (DEBUG) {
      args.push_back("-C");
      args.push_back("/home/scott/work/usl/uslang/usl.conf");
    }

    unsigned int size = args.size();
    exit = uslang(size, args);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    exit = 1;
  }

  return exit;
}
/**
 * =============================
 * The Kiwi Programming Language
 * =============================
 * 
 * A light-weight general-purpose scripting language.
 *
 * @author Scott Stauffer
 * @date 2024-01-31
 * @version 1.1.4
 *
 * GitHub Repository: https://github.com/fuseraft/kiwi
 * For more information, issues, or to contribute, please visit the repository.
 *
 * License: MIT License
 * SPDX-License-Identifier: MIT
 * The full license text is available in the LICENSE file in the repository.
 */

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
/**
 * =============================
 * The Kiwi Programming Language
 * =============================
 * 
 * A light-weight general-purpose scripting language.
 *
 * @author Scott Stauffer
 * @date 2024-01-31
 * @version 1.2.1
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

int main(int argc, char** argv) {
  return Kiwi::run(argc, argv);
}
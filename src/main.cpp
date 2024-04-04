/**
 * ===============================
 * The Astral Programming Language
 * ===============================
 * 
 * A light-weight general-purpose scripting language without a syntax tree.
 *
 * @author Scott Stauffer
 * @date 2024-01-31
 * @version 1.3.4
 *
 * GitHub Repository: https://github.com/fuseraft/astral
 * For more information, issues, or to contribute, please visit the repository.
 *
 * License: MIT License
 * SPDX-License-Identifier: MIT
 * The full license text is available in the LICENSE file in the repository.
 */

#include "astral.h"
#include "globals.h"

bool SILENCE = false;

int main(int argc, char** argv) {
  return Astral::run(argc, argv);
}
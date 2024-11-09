/**
 * ===============================
 * The Kiwi Programming Language
 * ===============================
 * 
 * A light-weight general-purpose scripting language with an interesting flavor.
 *
 * @author Scott Stauffer
 * @date 2024-11-07
 * @version 2.0.8
 *
 * GitHub Repository: https://github.com/fuseraft/kiwi
 * For more information, issues, or to contribute, please visit the repository.
 *
 * License: MIT License
 * SPDX-License-Identifier: MIT
 * The full license text is available in the LICENSE file in the repository.
 */

#include "cli.h"

int main(int argc, char** argv) {
  return KiwiCLI::run(argc, argv);
}
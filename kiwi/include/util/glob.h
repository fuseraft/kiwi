#ifndef KIWI_UTIL_GLOB_H
#define KIWI_UTIL_GLOB_H

#include <string>
#include "util/string.h"

struct Glob;

Glob parseGlob(k_string input);

/// @brief A glob.
struct Glob {
  /// @brief The path to search.
  k_string path;
  /// @brief The pattern string.
  k_string pattern;
  /// @brief The regex pattern string.
  k_string regexPattern;
  /// @brief Use recursion.
  bool recursiveTraversal = false;
};

/// @brief Create a regex pattern from a pattern string.
/// @param pattern The pattern string.
/// @return String containing regex pattern.
k_string createRegexPattern(const k_string& pattern) {
  k_string regexPattern;
  // Escape dots for regex
  for (char c : pattern) {
    if (c == '.') {
      regexPattern += "\\.";
    } else if (c == '*') {
      regexPattern += ".*";
    } else {
      regexPattern += c;
    }
  }

  if (!pattern.empty() && pattern.back() != '*') {
    regexPattern += "$";
  }

  return regexPattern;
}

Glob parseGlob(k_string input) {
  Glob glob;
  char sep = '/';

  // Check if the pattern includes the recursive "**"
  size_t doubleStarPos = input.find("**");
  if (doubleStarPos != k_string::npos) {
    glob.recursiveTraversal = true;
    if (doubleStarPos > 0 && input[doubleStarPos - 1] == sep) {
      // Exclude the '/' before '**'
      glob.path = input.substr(0, doubleStarPos - 1);
    } else {
      glob.path = "./";
    }

    if (doubleStarPos + 2 < input.size() && input[doubleStarPos + 2] == sep) {
      // Exclude the '/' after '**'
      glob.pattern = input.substr(doubleStarPos + 3);
    } else {
      glob.pattern = input.substr(doubleStarPos + 2);
    }
  } else {
    // Handle non-recursive patterns
    size_t lastSlashPos = input.rfind(sep);
    if (lastSlashPos != k_string::npos) {
      glob.path = input.substr(0, lastSlashPos);
      if (lastSlashPos + 1 < input.size()) {
        glob.pattern = input.substr(lastSlashPos + 1);
      }
    } else {
      glob.path = "./";
      glob.pattern = input;
    }
  }

  // Ensure that the path is not empty
  if (glob.path.empty()) {
    glob.path = "./";
  }

  // Create the regex pattern from the glob pattern
  glob.regexPattern = createRegexPattern(glob.pattern);

  return glob;
}

#endif
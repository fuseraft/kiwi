#ifndef KIWI_UTIL_GLOB_H
#define KIWI_UTIL_GLOB_H

#include <string>
#include "util/string.h"

struct Glob;

Glob parseGlob(std::string input);

/// @brief A glob.
struct Glob {
  /// @brief The path to search.
  std::string path;
  /// @brief The pattern string.
  std::string pattern;
  /// @brief The regex pattern string.
  std::string regexPattern;
  /// @brief Use recursion.
  bool recursiveTraversal = false;
};

/// @brief Create a regex pattern from a pattern string.
/// @param pattern The pattern string.
/// @return String containing regex pattern.
std::string createRegexPattern(const std::string& pattern) {
  std::string regexPattern;
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

Glob parseGlob(std::string input) {
  Glob glob;
  char sep = '/';

#ifdef _WIN64
  sep = '\\';
  if (String::contains(input, "/")) {
    input = String::replace(input, "/", "\\");
  }
#endif

  // Check if the pattern includes the recursive "**"
  size_t doubleStarPos = input.find("**");
  if (doubleStarPos != std::string::npos) {
    glob.recursiveTraversal = true;
    if (doubleStarPos > 0 && input[doubleStarPos - 1] == sep) {
      // Exclude the '/' before '**'
      glob.path = input.substr(0, doubleStarPos - 1);
    } else {
#ifdef _WIN64
      glob.path = ".\\";
#else
      glob.path = "./";
#endif
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
    if (lastSlashPos != std::string::npos) {
      glob.path = input.substr(0, lastSlashPos);
      if (lastSlashPos + 1 < input.size()) {
        glob.pattern = input.substr(lastSlashPos + 1);
      }
    } else {
#ifdef _WIN64
      glob.path = ".\\";
#else
      glob.path = "./";
#endif
      glob.pattern = input;
    }
  }

  // Ensure that the path is not empty
  if (glob.path.empty()) {
#ifdef _WIN64
    glob.path = ".\\";
#else
    glob.path = "./";
#endif
  }

  // Create the regex pattern from the glob pattern
  glob.regexPattern = createRegexPattern(glob.pattern);

  return glob;
}

#endif
#ifndef KIWI_UTIL_GLOB_H
#define KIWI_UTIL_GLOB_H

#include <string>

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

/// @brief Parse a glob.
/// @param input The glob string.
/// @return A glob.
Glob parseGlob(std::string input) {
  Glob glob;
  size_t pos = 0, length = input.size();
  std::string builder, path;
  bool canPeek = false;
  char c = '\0', next = '\0';

  while (pos < length) {
    c = input[pos++];
    canPeek = pos + 1 < length;
    if (canPeek) {
      next = input[pos];
    }

    if (c == '/' && glob.path.empty()) {
      builder += c;
      path += c;
      glob.path = builder;
      builder.clear();
      continue;
    } else if (c == '*') {
      if (canPeek && next == '*') {
        builder.clear();
        glob.recursiveTraversal = true;
        pos += 2;
        glob.path = path;
        continue;
      }
    }

    builder += c;
    path += c;
    next = '\0';
  }

  if (glob.path.empty()) {
    glob.path = "./";  // Default to current directory.
  }

  if (!builder.empty()) {
    glob.pattern = builder;
    builder.clear();
  }

  if (!glob.pattern.empty()) {
    glob.regexPattern = createRegexPattern(glob.pattern);
  }

  return glob;
}

#endif
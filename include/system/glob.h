#ifndef KIWI_SYSTEM_GLOB_H
#define KIWI_SYSTEM_GLOB_H

#include <string>

struct Glob;

Glob parseGlob(std::string input);

struct Glob {
  std::string path;
  std::string pattern;
  std::string regexPattern;
  bool recursiveTraversal = false;
};

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
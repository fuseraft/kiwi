#ifndef STRINGS_H
#define STRINGS_H

#include <string>

bool ends_with(std::string s, std::string end) {
  return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
}

bool begins_with(std::string s, std::string beginning) {
  return s.size() > beginning.size() &&
         s.substr(0, beginning.size()) == beginning;
}

bool is_flag(const std::string& s, const std::string& shortFlag,
             const std::string& longFlag) {
  std::string flags[] = {shortFlag, longFlag};

  // For funsies.
  for (const std::string& flag : flags) {
    if (s == ("-" + flag) || s == ("--" + flag) || s == ("/" + flag)) {
      return true;
    }
  }

  return false;
}

bool contains(std::string s, std::string search) {
  if (search == "") {
    return false;
  }

  return s.find(search) != std::string::npos;
}

#endif
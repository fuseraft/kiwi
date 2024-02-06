#ifndef KIWI_PARSING_STRINGS_H
#define KIWI_PARSING_STRINGS_H

#include <regex>
#include <string>

bool begins_with(std::string s, std::string beginning) {
  return s.size() > beginning.size() &&
         s.substr(0, beginning.size()) == beginning;
}

bool contains(std::string s, std::string search) {
  if (search == "") {
    return false;
  }

  return s.find(search) != std::string::npos;
}

bool ends_with(std::string s, std::string end) {
  return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
}

std::string replace(std::string s, std::string search,
                    std::string replacement) {
  std::regex pattern(search);
  return std::regex_replace(s, pattern, replacement);
}

std::string upcase(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

std::string downcase(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

int index_of(std::string s, std::string search) {
  size_t index = s.find(search);
  if (index != std::string::npos) {
    return index;
  }

  return -1;
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

#endif
#ifndef KIWI_PARSING_STRINGS_H
#define KIWI_PARSING_STRINGS_H

#include <algorithm>
#include <cctype>
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

std::string ltrim(const std::string& input) {
  std::string s = input;
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  return s;
}

std::string rtrim(const std::string& input) {
  std::string s = input;
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return s;
}

std::string trim(const std::string& input) {
  return ltrim(rtrim(input));
}

std::vector<std::string> split(const std::string& input,
                               const std::string& delimiter) {
  std::vector<std::string> result;

  if (delimiter.empty())
    return {input};

  std::string::size_type startPos = 0;
  std::string::size_type endPos = 0;

  while ((endPos = input.find(delimiter, startPos)) != std::string::npos) {
    result.push_back(input.substr(startPos, endPos - startPos));
    startPos = endPos + delimiter.size();
  }

  result.push_back(input.substr(startPos));

  return result;
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
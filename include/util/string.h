#ifndef KIWI_UTIL_STRING_H
#define KIWI_UTIL_STRING_H

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>

/// @brief A string utility.
class String {
 public:
  /// @brief Check if a string begins with another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool beginsWith(const std::string& s, const std::string& beginning) {
    return s.size() > beginning.size() &&
           s.substr(0, beginning.size()) == beginning;
  }

  /// @brief Check if a string contains another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool contains(const std::string& s, const std::string& search) {
    if (search == "") {
      return false;
    }

    return s.find(search) != std::string::npos;
  }

  /// @brief Check if a string ends with another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool endsWith(const std::string& s, const std::string& end) {
    return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
  }

  /// @brief Replace a string within a string.
  /// @param s The string to work on.
  /// @param search The string to find.
  /// @param replacement The replacement string.
  /// @return String containing replacement.
  static std::string replace(std::string s, std::string search,
                             std::string replacement) {
    std::regex pattern(search);
    return std::regex_replace(s, pattern, replacement);
  }

  /// @brief Convert the string to uppercase.
  /// @param s The string to convert.
  /// @return String containing uppercase text.
  static std::string toUppercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  }

  /// @brief Convert the string to lowercase.
  /// @param s The string to convert.
  /// @return String containing lowercase text.
  static std::string toLowercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
  }

  /// @brief Trim whitespace from the left of a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static std::string trimLeft(const std::string& input) {
    std::string s = input;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
              return !std::isspace(ch);
            }));
    return s;
  }

  /// @brief Trim whitespace from the right of a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static std::string trimRight(const std::string& input) {
    std::string s = input;
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
    return s;
  }

  /// @brief Trim whitespace from a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static std::string trim(const std::string& input) {
    return trimLeft(trimRight(input));
  }

  /// @brief Split a string into a vector of strings.
  /// @param input The string to split.
  /// @param delimiter The string to split on.
  /// @return A vector of strings.
  static std::vector<std::string> split(const std::string& input,
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

  /// @brief Get the index of a string within another string.
  /// @param s The string to search.
  /// @param search The string to find.
  /// @return Integer containing the index of a substring. Returns -1 if not found.
  static int indexOf(const std::string& s, const std::string& search) {
    size_t index = s.find(search);
    if (index != std::string::npos) {
      return index;
    }

    return -1;
  }

  /// @brief Get the last index of a string within another string.
  /// @param s The string to search.
  /// @param search The string to find.
  /// @return Integer containing the last index of a substring. Returns -1 if not found.
  static int lastIndexOf(const std::string& s, const std::string& search) {
    size_t index = s.rfind(search);
    if (index != std::string::npos) {
      return static_cast<int>(index);
    }

    return -1;
  }

  /// @brief Check if a string is a command-line flag.
  /// @param s The string to search.
  /// @param shortFlag The short flag name.
  /// @param longFlag The long flag name.
  /// @return Boolean indicating success.
  static bool isCLIFlag(const std::string& s, const std::string& shortFlag,
                        const std::string& longFlag) {
    auto flags = {shortFlag, longFlag};

    // For funsies.
    for (const auto& flag : flags) {
      if (s == ("-" + flag) || s == ("--" + flag) || s == ("/" + flag)) {
        return true;
      }
    }

    return false;
  }

  /// @brief Check if a string is an Xargument.
  /// @param s The string to check.
  /// @return Boolean indicating success.
  static bool isXArg(const std::string& s) {
    return beginsWith(s, "-X") && contains(s, "=");
  }
};

#endif
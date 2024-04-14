#ifndef ASTRAL_UTIL_REGEXP_H
#define ASTRAL_UTIL_REGEXP_H

#include <regex>
#include <vector>
#include "typing/value.h"

/// @brief A utility class for regexp support.
class RegularExpression {
 public:
  /// @brief Searches for the first occurrence of a pattern described by a regex and returns the substring.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A string.
  static k_string find(const k_string& text, const k_string& pattern) {
    std::regex reg(pattern);
    std::smatch match;

    if (std::regex_search(text, match, reg) && match.size() > 0) {
      return match.str(0);
    }

    return "";
  }

  /// @brief Returns a list of capture groups.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A list.
  static k_list match(const k_string& text, const k_string& pattern) {
    std::regex reg(pattern);
    std::smatch match;
    std::vector<k_string> results;

    if (std::regex_search(text, match, reg)) {
      for (size_t i = 1; i < match.size(); ++i) {
        results.push_back(match[i].str());
      }
    }

    return std::make_shared<List>(results);
  }

  /// @brief Tests whether the entire string conforms to a regular expression pattern.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A boolean.
  static bool matches(const k_string& text, const k_string& pattern) {
    std::regex reg(pattern);
    return std::regex_match(text, reg);
  }

  /// @brief Tests whether the entire string conforms to a regular expression pattern.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A boolean.
  static bool matches_all(const k_string& text, const k_string& pattern) {
    std::regex reg(pattern);
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), reg);
    auto words_end = std::sregex_iterator();

    size_t matches_length = 0;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
      std::smatch match = *i;
      matches_length += match.str().length();
    }

    return matches_length == text.length();
  }

  /// @brief Replaces every part of the string that matches the regex with the replacement string.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @param replacement The replacement text.
  /// @return A string.
  static k_string replace(const k_string& text, const k_string& pattern,
                          const k_string& replacement) {
    std::regex reg(pattern);
    return std::regex_replace(text, reg, replacement);
  }

  /// @brief Globally substitutes all occurrences of the regex in the string with the replacement, similar to `replace` but typically used for more complex replacements and supporting back-references.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @param replacement The replacement text.
  /// @return A string.
  static k_string replace_all(const k_string& text, const k_string& pattern,
                              const k_string& replacement) {
    std::regex reg(pattern);
    return std::regex_replace(text, reg, replacement);
  }

  /// @brief Finds every occurrence of the regex in the string and returns a list of matches.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A list.
  static k_list scan(const k_string& text, const k_string& pattern) {
    std::regex reg(pattern);
    std::sregex_iterator begin(text.begin(), text.end(), reg);
    std::sregex_iterator end;

    std::vector<k_string> matches;
    for (std::sregex_iterator i = begin; i != end; ++i) {
      std::smatch match = *i;
      matches.push_back(match.str());
    }

    return std::make_shared<List>(matches);
  }

  /// @brief Splits the string around matches of the given regex. An optional limit can be set for the number of splits.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @param limit The number of splits.
  /// @return A list.
  static k_list split(const k_string& text, const k_string& pattern,
                      int limit = -1) {
    std::regex reg(pattern);
    std::sregex_token_iterator iter(text.begin(), text.end(), reg, -1);
    std::sregex_token_iterator end;

    std::vector<k_string> result;
    while (iter != end && limit != 0) {
      result.push_back(*iter++);
      if (limit > 0) {
        limit--;
      }
    }

    return std::make_shared<List>(result);
  }
};

#endif
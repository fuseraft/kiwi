#ifndef KIWI_UTIL_STRING_H
#define KIWI_UTIL_STRING_H

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <regex>
#include "typing/value.h"

static const k_string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/// @brief A string utility.
class String {
 public:
  /// @brief Check if a string begins with another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool beginsWith(const k_string& s, const k_string& beginning) {
    return s.size() > beginning.size() &&
           s.substr(0, beginning.size()) == beginning;
  }

  /// @brief Check if a string contains another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool contains(const k_string& s, const k_string& search) {
    if (search == "") {
      return false;
    }

    return s.find(search) != k_string::npos;
  }

  /// @brief Check if a string ends with another string.
  /// @param s The string to search.
  /// @param beginning The string to find.
  /// @return Boolean indicating existence.
  static bool endsWith(const k_string& s, const k_string& end) {
    return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
  }

  /// @brief Searches for the first occurrence of a pattern described by a regex and returns the substring.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @return A string.
  static k_string find(const k_string& text, const k_string& pattern) {
    std::cout << "find(\"" << text << "\", '" << pattern << "')" << std::endl;
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
    std::vector<k_value> results;

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
  static bool matchesAll(const k_string& text, const k_string& pattern) {
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
    if (text.empty()) {
      return text;
    }
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

    std::vector<k_value> matches;
    for (std::sregex_iterator i = begin; i != end; ++i) {
      std::smatch match = *i;
      matches.emplace_back(match.str());
    }

    return std::make_shared<List>(matches);
  }

  /// @brief Splits the string around matches of the given regex. An optional limit can be set for the number of splits.
  /// @param text The string to check.
  /// @param pattern The regular expression.
  /// @param limit The number of splits.
  /// @return A list.
  static std::vector<k_string> split(const k_string& text,
                                     const k_string& pattern,
                                     k_int limit = -1) {
    std::regex reg(pattern);
    std::sregex_token_iterator iter(text.begin(), text.end(), reg, -1);
    std::sregex_token_iterator end;

    std::vector<k_string> result;
    int nlimit = static_cast<int>(limit);
    while (iter != end && nlimit != 0) {
      result.emplace_back(*iter++);
      if (nlimit > 0) {
        nlimit--;
      }
    }

    if (!result.empty() && iter != end && nlimit == 0) {
      k_string remaining;
      while (iter != end) {
        remaining += pattern + iter->str();
        ++iter;
      }

      result.back() += remaining;
    }

    if (limit < 0) {
      result.erase(
          std::remove_if(result.begin(), result.end(),
                         [](const std::string& s) { return s.empty(); }),
          result.end());
    }

    return result;
  }

  /// @brief Convert the string to uppercase.
  /// @param s The string to convert.
  /// @return String containing uppercase text.
  static k_string toUppercase(k_string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  }

  /// @brief Convert the string to lowercase.
  /// @param s The string to convert.
  /// @return String containing lowercase text.
  static k_string toLowercase(k_string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
  }

  /// @brief Trim whitespace from the left of a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static k_string trimLeft(const k_string& input) {
    k_string s = input;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
              return !std::isspace(ch);
            }));
    return s;
  }

  /// @brief Trim whitespace from the right of a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static k_string trimRight(const k_string& input) {
    k_string s = input;
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
    return s;
  }

  /// @brief Trim whitespace from a string.
  /// @param s The string to trim.
  /// @return String containing trimmed text.
  static k_string trim(const k_string& input) {
    return trimLeft(trimRight(input));
  }

  /// @brief Extract a substring.
  /// @param s The string to extract from.
  /// @param pos The position in the string to start.
  /// @param size The size of the substring to extract.
  /// @return The substring.
  static k_string substring(const k_string& s, size_t pos, size_t size) {
    if (pos >= s.size()) {
      return "";
    }

    return s.substr(pos, std::min(size, s.size() - pos));
  }

  /// @brief Get the index of a string within another string.
  /// @param s The string to search.
  /// @param search The string to find.
  /// @return Integer containing the index of a substring. Returns -1 if not found.
  static int indexOf(const k_string& s, const k_string& search) {
    size_t index = s.find(search);
    if (index != k_string::npos) {
      return index;
    }

    return -1;
  }

  /// @brief Get the last index of a string within another string.
  /// @param s The string to search.
  /// @param search The string to find.
  /// @return Integer containing the last index of a substring. Returns -1 if not found.
  static int lastIndexOf(const k_string& s, const k_string& search) {
    size_t index = s.rfind(search);
    if (index != k_string::npos) {
      return static_cast<int>(index);
    }

    return -1;
  }

  /// @brief Check if a string is a command-line flag.
  /// @param s The string to search.
  /// @param shortFlag The short flag name.
  /// @param longFlag The long flag name.
  /// @return Boolean indicating success.
  static bool isCLIFlag(const k_string& s, const k_string& shortFlag,
                        const k_string& longFlag) {
    std::array<k_string, 3> prefixes = {"-", "--", "/"};

    return std::any_of(
        prefixes.begin(), prefixes.end(), [&](const k_string& prefix) {
          return s == prefix + shortFlag || s == prefix + longFlag;
        });
  }

  /// @brief Check if a string is a KVP option.
  /// @param s The string to check.
  /// @return Boolean indicating success.
  static bool isOptionKVP(const k_string& s) {
    std::array<k_string, 3> prefixes = {"-", "--", "/"};
    return std::any_of(prefixes.begin(), prefixes.end(),
                       [&](const k_string& prefix) {
                         return beginsWith(s, prefix) && contains(s, "=");
                       });
  }

  static char hexToChar(const k_string& hex) {
    int ch = std::stoi(hex, nullptr, 16);
    if (ch >= 0 && ch <= 0xFF) {
      return static_cast<char>(ch);
    } else {
      throw std::invalid_argument("Invalid hex character");
    }
  }

  static k_string urlEncode(const k_string& value) {
    std::ostringstream encoded;
    encoded.fill('0');

    for (unsigned char c : value) {
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        encoded << c;
      } else {
        encoded << std::uppercase;
        encoded << '%' << std::setw(2) << std::hex << static_cast<int>(c);
        encoded << std::nouppercase;
      }
    }

    return encoded.str();
  }

  static k_string urlDecode(const k_string& encoded) {
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
      if (encoded[i] == '%' && i + 2 < encoded.length()) {
        k_string hexStr = encoded.substr(i + 1, 2);

        try {
          char decodedChar = hexToChar(hexStr);
          decoded << decodedChar;
        } catch (const std::invalid_argument& e) {
          std::cout << "Error decoding: " << e.what() << std::endl;
          return "";
        }

        i += 2;
      } else if (encoded[i] == '+') {
        decoded << ' ';
      } else {
        decoded << encoded[i];
      }
    }
    return decoded.str();
  }

  static int base64CharValue(char base64_char) {
    if (base64_char >= 'A' && base64_char <= 'Z')
      return base64_char - 'A';
    if (base64_char >= 'a' && base64_char <= 'z')
      return base64_char - 'a' + 26;
    if (base64_char >= '0' && base64_char <= '9')
      return base64_char - '0' + 52;
    if (base64_char == '+')
      return 62;
    if (base64_char == '/')
      return 63;
    return -1;
  }

  static k_string base64Encode(const k_string& input) {
    k_string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0) {
        output.push_back(base64_chars[(val >> valb) & 0x3F]);
        valb -= 6;
      }
    }
    if (valb > -6)
      output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (output.size() % 4)
      output.push_back('=');
    return output;
  }

  static k_string base64Decode(const k_string& input) {
    k_string output;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
      T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : input) {
      if (T[c] == -1)
        break;
      val = (val << 6) + T[c];
      valb += 6;
      if (valb >= 0) {
        output.push_back(char((val >> valb) & 0xFF));
        valb -= 8;
      }
    }
    return output;
  }
};

#endif
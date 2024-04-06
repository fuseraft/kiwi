#ifndef ASTRAL_UTIL_STRING_H
#define ASTRAL_UTIL_STRING_H

#include <algorithm>
#include <array>
#include <cctype>
#include <regex>
#include <string>

static const std::string base64_chars =
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

  /// @brief Extract a substring.
  /// @param s The string to extract from.
  /// @param pos The position in the string to start.
  /// @param size The size of the substring to extract.
  /// @return The substring.
  static std::string substring(const std::string& s, size_t pos, size_t size) {
    if (pos >= s.size()) {
      return "";
    }

    return s.substr(pos, std::min(size, s.size() - pos));
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
      result.emplace_back(input.substr(startPos, endPos - startPos));
      startPos = endPos + delimiter.size();
    }

    result.emplace_back(input.substr(startPos));

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
    std::array<std::string, 3> prefixes = {"-", "--", "/"};

    return std::any_of(
        prefixes.begin(), prefixes.end(), [&](const std::string& prefix) {
          return s == prefix + shortFlag || s == prefix + longFlag;
        });
  }

  /// @brief Check if a string is a KVP option.
  /// @param s The string to check.
  /// @return Boolean indicating success.
  static bool isOptionKVP(const std::string& s) {
    std::array<std::string, 3> prefixes = {"-", "--", "/"};
    return std::any_of(prefixes.begin(), prefixes.end(),
                       [&](const std::string& prefix) {
                         return beginsWith(s, prefix) && contains(s, "=");
                       });
  }

  static char hexToChar(const std::string& hex) {
    int ch = std::stoi(hex, nullptr, 16);
    if (ch >= 0 && ch <= 0xFF) {
      return static_cast<char>(ch);
    } else {
      throw std::invalid_argument("Invalid hex character");
    }
  }

  static std::string urlEncode(const std::string& value) {
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

  static std::string urlDecode(const std::string& encoded) {
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
      if (encoded[i] == '%' && i + 2 < encoded.length()) {
        std::string hexStr = encoded.substr(i + 1, 2);

        try {
          char decodedChar = hexToChar(hexStr);
          decoded << decodedChar;
        } catch (const std::invalid_argument& e) {
          std::cerr << "Error decoding: " << e.what() << std::endl;
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

  static std::string base64Encode(const std::string& input) {
    std::string output;
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

  static std::string base64Decode(const std::string& input) {
    std::string output;
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
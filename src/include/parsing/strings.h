#ifndef STRINGS_H
#define STRINGS_H

#include <string>

bool ends_with(std::string s, std::string end) {
    return s.size() > end.size() && s.substr(s.size() - end.size()) == end;
}

bool begins_with(std::string s, std::string start) {
    return s.size() > start.size() && s.substr(0, start.size()) == start;
}

bool is_flag(
    const std::string &s, const std::string &shortFlag,
    const std::string &longFlag) {
    std::string flags[] = {shortFlag, longFlag};
    for (const std::string &flag : flags) {
        if (s == ("-" + flag) || s == ("--" + flag) || s == ("/" + flag)) {
            return true;
        }
    }

    return false;
}

bool is_script(std::string path) {
    return ends_with(path, ".usl") || ends_with(path, ".uslang");
}

bool contains(std::string s1, std::string s2) {
    if (s2 == "") {
        return false;
    }

    return s1.find(s2) != std::string::npos;
}

#endif
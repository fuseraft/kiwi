#ifndef METHOD_H
#define METHOD_H

#include <vector>
#include "../parsing/tokens.h"

class Method {
public:
    void addToken(Token& t) {
        code.push_back(t);
    }

    std::vector<Token> getCode() {
        return code;
    }
private:
    std::vector<Token> parameters;
    std::vector<Token> code;
};

#endif
#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include <vector>
#include "../parsing/tokens.h"

class IfStatement {
public:
    void addToken(Token& t) {
        code.push_back(t);
    }

    void setEvaluation(bool evaluation) {
        _evaluation = evaluation;
    }

    bool isExecutable() {
        return _evaluation;
    }

    std::vector<Token> getCode() {
        return code;
    }
private:
    std::vector<Token> code;
    bool _evaluation;
};

class ElseIfStatement : public IfStatement {
public:
private:
    std::vector<Token> code;
    bool _evaluation;
};

class ElseStatement {
public:
    void addToken(Token& t) {
        code.push_back(t);
    }

    std::vector<Token> getCode() {
        return code;
    }

private:
    std::vector<Token> code;
};

class Conditional {
public:
    IfStatement& getIfStatement() {
        return ifStatement;
    }

    ElseStatement& getElseStatement() {
        return elseStatement;
    }

    std::vector<ElseIfStatement>& getElseIfStatements() {
        return elseIfStatements;
    }

    ElseIfStatement& getElseIfStatement() {
        if (elseIfStatements.empty()) {
            addElseIfStatement();
        }
        return elseIfStatements.at(elseIfStatements.size() - 1);
    }

    bool canExecuteElseIf() {
        for (ElseIfStatement elseIf : elseIfStatements) {
            if (elseIf.isExecutable()) {
                return true;
            }
        }
        return false;
    }

    void addElseIfStatement() {
        ElseIfStatement elseIf;
        elseIfStatements.push_back(elseIf);
    }

private:
    IfStatement ifStatement;
    std::vector<ElseIfStatement> elseIfStatements;
    ElseStatement elseStatement;
};

#endif
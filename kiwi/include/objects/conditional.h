#ifndef KIWI_OBJECTS_CONDITIONAL_H
#define KIWI_OBJECTS_CONDITIONAL_H

#include <vector>
#include "parsing/tokens.h"

class IfStatement {
 public:
  void addToken(const Token& t) { code.emplace_back(t); }

  void setEvaluation(bool evaluation) { _evaluation = evaluation; }

  bool isExecutable() const { return _evaluation; }

  std::vector<Token>& getCode() { return code; }

 private:
  std::vector<Token> code;
  bool _evaluation = false;
};

class ElseIfStatement : public IfStatement {
 public:
 private:
  std::vector<Token> code;
  bool _evaluation = false;
};

class ElseStatement {
 public:
  void addToken(const Token& t) { code.emplace_back(t); }

  std::vector<Token>& getCode() { return code; }

 private:
  std::vector<Token> code;
};

class Conditional {
 public:
  IfStatement& getIfStatement() { return ifStatement; }

  ElseStatement& getElseStatement() { return elseStatement; }

  std::vector<ElseIfStatement>& getElseIfStatements() {
    return elseIfStatements;
  }

  ElseIfStatement& getElseIfStatement() {
    if (elseIfStatements.empty()) {
      addElseIfStatement();
    }
    return elseIfStatements.at(elseIfStatements.size() - 1);
  }

  bool canExecuteElseIf() const {
    for (const auto& elseIf : elseIfStatements) {
      if (elseIf.isExecutable()) {
        return true;
      }
    }
    return false;
  }

  void addElseIfStatement() {
    ElseIfStatement elseIf;
    elseIfStatements.emplace_back(elseIf);
  }

 private:
  IfStatement ifStatement;
  std::vector<ElseIfStatement> elseIfStatements;
  ElseStatement elseStatement;
};

#endif
#ifndef KIWI_PARSING_AST_H
#define KIWI_PARSING_AST_H

#include "typing/serializer.h"
#include "typing/value.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class ASTNodeType {
  PROGRAM,
  FUNCTION_DECLARATION,
  FUNCTION_CALL,
  VARIABLE_DECLARATION,
  BINARY_OPERATION,
  UNARY_OPERATION,
  LITERAL,
  IDENTIFIER,
  EXPRESSION_STATEMENT,
  RETURN_STATEMENT,
  ASSIGNMENT,
  IF_STATEMENT,
  WHILE_LOOP,
  FOR_LOOP,
  BLOCK_STATEMENT,
  PRINT_STATEMENT,
};

class ASTNode {
 public:
  ASTNodeType type;

  ASTNode(ASTNodeType type) : type(type) {}
  virtual ~ASTNode() = default;

  virtual void print() const = 0;
};

class ProgramNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> statements;

  ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}
  ProgramNode(std::vector<std::unique_ptr<ASTNode>> statements)
      : ASTNode(ASTNodeType::PROGRAM), statements(std::move(statements)) {}

  void print() const override {
    std::cout << "ProgramNode: " << std::endl;
    for (const auto& statement : statements) {
      statement->print();
    }
  }
};

class LiteralNode : public ASTNode {
 public:
  k_value value;

  LiteralNode() : ASTNode(ASTNodeType::LITERAL) {}
  LiteralNode(const k_value& value)
      : ASTNode(ASTNodeType::LITERAL), value(value) {}

  void print() const override {
    std::cout << "Literal: " << Serializer::serialize(value) << std::endl;
  }
};

class IdentifierNode : public ASTNode {
 public:
  std::string name;

  IdentifierNode() : ASTNode(ASTNodeType::IDENTIFIER) {}
  IdentifierNode(const std::string& name)
      : ASTNode(ASTNodeType::IDENTIFIER), name(name) {}

  void print() const override {
    std::cout << "Identifier: " << name << std::endl;
  }
};

class BinaryOperationNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> left;
  std::string op;
  std::unique_ptr<ASTNode> right;

  BinaryOperationNode() : ASTNode(ASTNodeType::BINARY_OPERATION) {}
  BinaryOperationNode(std::unique_ptr<ASTNode> left, const std::string& op,
                      std::unique_ptr<ASTNode> right)
      : ASTNode(ASTNodeType::BINARY_OPERATION),
        left(std::move(left)),
        op(op),
        right(std::move(right)) {}

  void print() const override {
    std::cout << "BinaryOperation: " << op << std::endl;
    left->print();
    right->print();
  }
};

class UnaryOperationNode : public ASTNode {
 public:
  std::string op;
  std::unique_ptr<ASTNode> operand;

  UnaryOperationNode() : ASTNode(ASTNodeType::UNARY_OPERATION) {}
  UnaryOperationNode(const std::string& op, std::unique_ptr<ASTNode> operand)
      : ASTNode(ASTNodeType::UNARY_OPERATION),
        op(op),
        operand(std::move(operand)) {}

  void print() const override {
    std::cout << "UnaryOperation: " << op << std::endl;
    operand->print();
  }
};

class PrintNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> expression;  // Expression to print
  bool printNewline;                    // Flag for printing a newline

  PrintNode() : ASTNode(ASTNodeType::PRINT_STATEMENT) {}
  PrintNode(std::unique_ptr<ASTNode> expression, bool printNewline)
      : ASTNode(ASTNodeType::PRINT_STATEMENT),
        expression(std::move(expression)),
        printNewline(printNewline) {}

  void print() const override {
    std::cout << "PrintNode: "
              << (printNewline ? "with newline" : "without newline")
              << std::endl;
    expression->print();
  }
};

class FunctionDeclarationNode : public ASTNode {
 public:
  std::string name;
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  std::vector<std::unique_ptr<ASTNode>> body;

  FunctionDeclarationNode() : ASTNode(ASTNodeType::FUNCTION_DECLARATION) {}

  void print() const override {
    std::cout << "FunctionDeclaration: " << name << std::endl;
    std::cout << "Parameters: " << std::endl;
    for (const auto& param : parameters) {
      std::cout << "  " << param.first;
      if (param.second) {
        std::cout << " (default: ";
        param.second->print();
        std::cout << ")";
      }
      std::cout << std::endl;
    }
    for (const auto& stmt : body) {
      stmt->print();
    }
  }
};

class FunctionCallNode : public ASTNode {
 public:
  std::string functionName;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  FunctionCallNode() : ASTNode(ASTNodeType::FUNCTION_CALL) {}
  FunctionCallNode(const std::string& functionName,
                   std::vector<std::unique_ptr<ASTNode>> arguments)
      : ASTNode(ASTNodeType::FUNCTION_CALL),
        functionName(functionName),
        arguments(std::move(arguments)) {}

  void print() const override {
    std::cout << "FunctionCall: " << functionName << std::endl;
    std::cout << "Arguments: " << std::endl;
    for (const auto& arg : arguments) {
      arg->print();
    }
  }
};

class VariableDeclarationNode : public ASTNode {
 public:
  std::string name;
  std::unique_ptr<ASTNode> initializer;

  VariableDeclarationNode() : ASTNode(ASTNodeType::VARIABLE_DECLARATION) {}
  VariableDeclarationNode(const std::string& name,
                          std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::VARIABLE_DECLARATION),
        name(name),
        initializer(std::move(initializer)) {}

  void print() const override {
    std::cout << "VariableDeclaration: " << name << std::endl;
    if (initializer) {
      std::cout << "Initializer: " << std::endl;
      initializer->print();
    }
  }
};

class BlockStatementNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> statements;

  BlockStatementNode() : ASTNode(ASTNodeType::BLOCK_STATEMENT) {}
  BlockStatementNode(std::vector<std::unique_ptr<ASTNode>> statements)
      : ASTNode(ASTNodeType::BLOCK_STATEMENT),
        statements(std::move(statements)) {}

  void print() const override {
    std::cout << "BlockStatement: " << std::endl;
    for (const auto& statement : statements) {
      statement->print();
    }
  }
};

#endif
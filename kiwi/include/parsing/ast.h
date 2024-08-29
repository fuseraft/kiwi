#ifndef KIWI_PARSING_AST_H
#define KIWI_PARSING_AST_H

#include "tokens.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class ASTNodeType {
  ASSIGNMENT,            // done
  BINARY_OPERATION,      // done
  BREAK_STATEMENT,       // done
  CASE_STATEMENT,        // done
  CASE_WHEN,             // done
  EXIT_STATEMENT,        // done
  EXPORT_STATEMENT,      // obsolete
  FOR_LOOP,              // done
  FUNCTION_CALL,         // done
  FUNCTION_DECLARATION,  // done
  HASH_LITERAL,          // done
  IDENTIFIER,            // done
  IF_STATEMENT,          // done
  IMPORT_STATEMENT,      // done
  INDEX_EXPRESSION,      // done
  LAMBDA,                // done
  LIST_LITERAL,          // done
  LITERAL,               // done
  MEMBER_ACCESS,
  MEMBER_ASSIGNMENT,  // done
  METHOD_CALL,        // wip
  NEXT_STATEMENT,     // done
  NO_OP,              // done
  PACKAGE,            // done
  PARSE_STATEMENT,
  PRINT_STATEMENT,    // done
  PROGRAM,            // done
  RANGE_LITERAL,      // done
  REPEAT_LOOP,        // done
  RETURN_STATEMENT,   // done
  SLICE_EXPRESSION,   // done
  TERNARY_OPERATION,  // done
  THROW_STATEMENT,    // done
  TRY,                // done
  UNARY_OPERATION,    // done
  WHILE_LOOP,         // done
};

class ASTNode {
 public:
  ASTNodeType type;
  Token token = Token::createEmpty();

  ASTNode(ASTNodeType type) : type(type) {}
  virtual ~ASTNode() = default;

  virtual void print(int depth = 0) const = 0;
};

void print_depth(int depth) {
  for (int i = 0; i < depth; ++i) {
    std::cout << " ";
  }
}

class ProgramNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> statements;

  ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}
  ProgramNode(std::vector<std::unique_ptr<ASTNode>> statements)
      : ASTNode(ASTNodeType::PROGRAM), statements(std::move(statements)) {}

  void print(int depth = 0) const override {
    print_depth(depth);
    std::cout << "Program:" << std::endl;
    for (const auto& statement : statements) {
      statement->print(1 + depth);
    }
  }
};

class LiteralNode : public ASTNode {
 public:
  k_value value;

  LiteralNode() : ASTNode(ASTNodeType::LITERAL) {}
  LiteralNode(const k_value& value)
      : ASTNode(ASTNodeType::LITERAL), value(value) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Literal: " << Serializer::serialize(value) << std::endl;
  }
};

class HashLiteralNode : public ASTNode {
 public:
  std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> elements;

  HashLiteralNode(
      std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> elements)
      : ASTNode(ASTNodeType::HASH_LITERAL), elements(std::move(elements)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "HashLiteral:" << std::endl;
    for (const auto& element : elements) {
      element.first->print(1 + depth);
      element.second->print(1 + depth);
    }
  }
};

class ListLiteralNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> elements;

  ListLiteralNode() : ASTNode(ASTNodeType::LIST_LITERAL) {}
  ListLiteralNode(std::vector<std::unique_ptr<ASTNode>> elements)
      : ASTNode(ASTNodeType::LIST_LITERAL), elements(std::move(elements)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "ListLiteral:" << std::endl;
    for (const auto& element : elements) {
      element->print(1 + depth);
    }
  }
};

class RangeLiteralNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> rangeStart;
  std::unique_ptr<ASTNode> rangeEnd;

  RangeLiteralNode() : ASTNode(ASTNodeType::RANGE_LITERAL) {}
  RangeLiteralNode(std::unique_ptr<ASTNode> rangeStart,
                   std::unique_ptr<ASTNode> rangeEnd)
      : ASTNode(ASTNodeType::RANGE_LITERAL),
        rangeStart(std::move(rangeStart)),
        rangeEnd(std::move(rangeEnd)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "RangeLiteral:" << std::endl;
    rangeStart->print(1 + depth);
    rangeEnd->print(1 + depth);
  }
};

class IndexingNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> indexedObject;
  std::string name;
  std::unique_ptr<ASTNode> indexExpression;

  IndexingNode() : ASTNode(ASTNodeType::INDEX_EXPRESSION) {}

  IndexingNode(const std::string& name,
               std::unique_ptr<ASTNode> indexExpression)
      : ASTNode(ASTNodeType::INDEX_EXPRESSION),
        name(name),
        indexExpression(std::move(indexExpression)) {}

  IndexingNode(std::unique_ptr<ASTNode> indexedObject,
               std::unique_ptr<ASTNode> indexExpression)
      : ASTNode(ASTNodeType::INDEX_EXPRESSION),
        indexedObject(std::move(indexedObject)),
        indexExpression(std::move(indexExpression)) {}

  void print(int depth) const override {
    print_depth(depth);
    if (indexedObject) {
      std::cout << "Index on object:" << std::endl;
      indexedObject->print(1 + depth);
    } else {
      std::cout << "Index: `" << name << "`" << std::endl;
    }
    indexExpression->print(1 + depth);
  }
};

class ReturnNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> returnValue;
  std::unique_ptr<ASTNode> condition;

  ReturnNode() : ASTNode(ASTNodeType::RETURN_STATEMENT) {}
  ReturnNode(std::unique_ptr<ASTNode> returnValue,
             std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::RETURN_STATEMENT),
        returnValue(std::move(returnValue)),
        condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Return:" << std::endl;
    if (returnValue) {
      returnValue->print(1 + depth);
    }

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }
};

class ThrowNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> errorValue;
  std::unique_ptr<ASTNode> condition;

  ThrowNode() : ASTNode(ASTNodeType::THROW_STATEMENT) {}
  ThrowNode(std::unique_ptr<ASTNode> errorValue,
            std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::THROW_STATEMENT),
        errorValue(std::move(errorValue)),
        condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Throw:" << std::endl;
    if (errorValue) {
      errorValue->print(1 + depth);
    }

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }
};

class ExitNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> exitValue;
  std::unique_ptr<ASTNode> condition;

  ExitNode() : ASTNode(ASTNodeType::EXIT_STATEMENT) {}
  ExitNode(std::unique_ptr<ASTNode> exitValue,
           std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::EXIT_STATEMENT),
        exitValue(std::move(exitValue)),
        condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Exit:" << std::endl;
    if (exitValue) {
      exitValue->print(1 + depth);
    }

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }
};

class ParseNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> parseValue;

  ParseNode() : ASTNode(ASTNodeType::PARSE_STATEMENT) {}
  ParseNode(std::unique_ptr<ASTNode> parseValue)
      : ASTNode(ASTNodeType::PARSE_STATEMENT),
        parseValue(std::move(parseValue)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Parse:" << std::endl;
    if (parseValue) {
      parseValue->print(1 + depth);
    }
  }
};

class NextNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;

  NextNode() : ASTNode(ASTNodeType::NEXT_STATEMENT) {}
  NextNode(std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::NEXT_STATEMENT), condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Next:" << std::endl;

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }
};

class BreakNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;

  BreakNode() : ASTNode(ASTNodeType::BREAK_STATEMENT) {}
  BreakNode(std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::BREAK_STATEMENT),
        condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Break:" << std::endl;

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }
};

class NoOpNode : public ASTNode {
 public:
  NoOpNode() : ASTNode(ASTNodeType::NO_OP) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "No operation:" << std::endl;
  }
};

class ImportNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> packageName;

  ImportNode() : ASTNode(ASTNodeType::IMPORT_STATEMENT) {}
  ImportNode(std::unique_ptr<ASTNode> packageName)
      : ASTNode(ASTNodeType::IMPORT_STATEMENT),
        packageName(std::move(packageName)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Import:" << std::endl;
    packageName->print(1 + depth);
  }
};

class ExportNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> packageName;

  ExportNode() : ASTNode(ASTNodeType::EXPORT_STATEMENT) {}
  ExportNode(std::unique_ptr<ASTNode> packageName)
      : ASTNode(ASTNodeType::EXPORT_STATEMENT),
        packageName(std::move(packageName)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Export:" << std::endl;
    packageName->print(1 + depth);
  }
};

class PackageNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> packageName;
  std::vector<std::unique_ptr<ASTNode>> body;

  PackageNode() : ASTNode(ASTNodeType::PACKAGE) {}
  PackageNode(std::unique_ptr<ASTNode> packageName)
      : ASTNode(ASTNodeType::PACKAGE), packageName(std::move(packageName)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Package:" << std::endl;
    packageName->print(1 + depth);

    print_depth(depth);
    std::cout << "Content:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class SliceNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> slicedObject;
  std::unique_ptr<ASTNode> startExpression;
  std::unique_ptr<ASTNode> stopExpression;
  std::unique_ptr<ASTNode> stepExpression;

  SliceNode() : ASTNode(ASTNodeType::SLICE_EXPRESSION) {}
  SliceNode(std::unique_ptr<ASTNode> slicedObject,
            std::unique_ptr<ASTNode> startExpression = nullptr,
            std::unique_ptr<ASTNode> stopExpression = nullptr,
            std::unique_ptr<ASTNode> stepExpression = nullptr)
      : ASTNode(ASTNodeType::SLICE_EXPRESSION),
        slicedObject(std::move(slicedObject)),
        startExpression(std::move(startExpression)),
        stopExpression(std::move(stopExpression)),
        stepExpression(std::move(stepExpression)) {}

  void print(int depth) const override {
    print_depth(depth);

    if (slicedObject) {
      std::cout << "Slice on object:" << std::endl;
      slicedObject->print(1 + depth);
    }

    if (startExpression) {
      print_depth(depth);
      std::cout << "Slice start:" << std::endl;
      startExpression->print(1 + depth);
    }

    if (stopExpression) {
      print_depth(depth);
      std::cout << "Slice stop:" << std::endl;
      stopExpression->print(1 + depth);
    }

    if (stepExpression) {
      print_depth(depth);
      std::cout << "Slice step:" << std::endl;
      stepExpression->print(1 + depth);
    }
  }
};

class IdentifierNode : public ASTNode {
 public:
  std::string name;
  std::string package;

  IdentifierNode() : ASTNode(ASTNodeType::IDENTIFIER) {}
  IdentifierNode(const std::string& name)
      : ASTNode(ASTNodeType::IDENTIFIER), name(name) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Identifier: `" << name << "`" << std::endl;
  }
};

class TernaryOperationNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> evalExpression;
  std::unique_ptr<ASTNode> trueExpression;
  std::unique_ptr<ASTNode> falseExpression;

  TernaryOperationNode() : ASTNode(ASTNodeType::TERNARY_OPERATION) {}
  TernaryOperationNode(std::unique_ptr<ASTNode> evalExpression,
                       std::unique_ptr<ASTNode> trueExpression,
                       std::unique_ptr<ASTNode> falseExpression)
      : ASTNode(ASTNodeType::TERNARY_OPERATION),
        evalExpression(std::move(evalExpression)),
        trueExpression(std::move(trueExpression)),
        falseExpression(std::move(falseExpression)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "TernaryOperation:" << std::endl;
    print_depth(depth);
    std::cout << "Evaluate expression:" << std::endl;
    evalExpression->print(1 + depth);
    print_depth(depth);
    std::cout << "True expression:" << std::endl;
    trueExpression->print(1 + depth);
    print_depth(depth);
    std::cout << "False expression:" << std::endl;
    falseExpression->print(1 + depth);
  }
};

class BinaryOperationNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> left;
  KName op;
  std::unique_ptr<ASTNode> right;

  BinaryOperationNode() : ASTNode(ASTNodeType::BINARY_OPERATION) {}
  BinaryOperationNode(std::unique_ptr<ASTNode> left, const KName& op,
                      std::unique_ptr<ASTNode> right)
      : ASTNode(ASTNodeType::BINARY_OPERATION),
        left(std::move(left)),
        op(op),
        right(std::move(right)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "BinaryOperation: " << Operators.get_op_string(op)
              << std::endl;
    left->print(1 + depth);
    right->print(1 + depth);
  }
};

class UnaryOperationNode : public ASTNode {
 public:
  KName op;
  std::unique_ptr<ASTNode> operand;

  UnaryOperationNode() : ASTNode(ASTNodeType::UNARY_OPERATION) {}
  UnaryOperationNode(const KName& op, std::unique_ptr<ASTNode> operand)
      : ASTNode(ASTNodeType::UNARY_OPERATION),
        op(op),
        operand(std::move(operand)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "UnaryOperation: " << Operators.get_op_string(op) << std::endl;
    operand->print(1 + depth);
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

  void print(int depth) const override {
    print_depth(depth);
    std::cout << (printNewline ? "Print line:" : "Print:") << std::endl;
    expression->print(1 + depth);
  }
};

class FunctionDeclarationNode : public ASTNode {
 public:
  std::string name;
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  std::vector<std::unique_ptr<ASTNode>> body;

  FunctionDeclarationNode() : ASTNode(ASTNodeType::FUNCTION_DECLARATION) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "FunctionDeclaration: `" << name << "`" << std::endl;
    print_depth(depth);
    std::cout << "Parameters: " << std::endl;
    for (const auto& param : parameters) {
      print_depth(1 + depth);
      std::cout << param.first;
      if (param.second) {
        print_depth(1 + depth);
        std::cout << "Default: ";
        param.second->print(1 + depth);
      } else {
        std::cout << std::endl;
      }
    }

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class LambdaNode : public ASTNode {
 public:
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  std::vector<std::unique_ptr<ASTNode>> body;

  LambdaNode() : ASTNode(ASTNodeType::LAMBDA) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Lambda: " << std::endl;
    print_depth(depth);
    std::cout << "Parameters: " << std::endl;
    for (const auto& param : parameters) {
      print_depth(1 + depth);
      std::cout << param.first;
      if (param.second) {
        print_depth(1 + depth);
        std::cout << "Default: ";
        param.second->print(1 + depth);
      } else {
        std::cout << std::endl;
      }
    }

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class ForLoopNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> dataSet;
  std::unique_ptr<ASTNode> valueIterator;
  std::unique_ptr<ASTNode> indexIterator;
  std::vector<std::unique_ptr<ASTNode>> body;

  ForLoopNode() : ASTNode(ASTNodeType::FOR_LOOP) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "For loop: " << std::endl;

    print_depth(depth);
    std::cout << "Data set: " << std::endl;
    dataSet->print(1 + depth);

    print_depth(depth);
    std::cout << "Value iterator: " << std::endl;
    valueIterator->print(1 + depth);

    if (indexIterator) {
      print_depth(depth);
      std::cout << "Index iterator: " << std::endl;
      indexIterator->print(1 + depth);
    }

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class WhileLoopNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;

  WhileLoopNode() : ASTNode(ASTNodeType::WHILE_LOOP) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "While loop: " << std::endl;

    print_depth(depth);
    std::cout << "Condition: " << std::endl;
    condition->print(1 + depth);

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class RepeatLoopNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> count;
  std::unique_ptr<ASTNode> alias;
  std::vector<std::unique_ptr<ASTNode>> body;

  RepeatLoopNode() : ASTNode(ASTNodeType::REPEAT_LOOP) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Repeat loop: " << std::endl;

    print_depth(depth);
    std::cout << "Count: " << std::endl;
    count->print(1 + depth);

    if (alias) {
      print_depth(depth);
      std::cout << "Alias: " << std::endl;
      alias->print(1 + depth);
    }

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class CaseWhenNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;

  CaseWhenNode() : ASTNode(ASTNodeType::CASE_WHEN) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Case when:" << std::endl;
    condition->print(1 + depth);

    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }
  }
};

class CaseNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> testValue;
  std::vector<std::unique_ptr<ASTNode>> elseBody;
  std::vector<std::unique_ptr<CaseWhenNode>> whenNodes;

  CaseNode() : ASTNode(ASTNodeType::CASE_STATEMENT) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Case:" << std::endl;

    if (testValue) {
      print_depth(depth);
      std::cout << "Test:" << std::endl;
      testValue->print(1 + depth);
    }

    if (!whenNodes.empty()) {
      for (const auto& when : whenNodes) {
        when->print(1 + depth);
      }
    }

    if (!elseBody.empty()) {
      print_depth(1 + depth);
      std::cout << "Case else:" << std::endl;
      for (const auto& stmt : elseBody) {
        stmt->print(2 + depth);
      }
    }
  }
};

class IfNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;
  std::vector<std::unique_ptr<ASTNode>> elseBody;
  std::vector<std::unique_ptr<IfNode>> elseifNodes;

  IfNode() : ASTNode(ASTNodeType::IF_STATEMENT) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "If:" << std::endl;
    condition->print(1 + depth);
    print_depth(depth);
    std::cout << "Statements:" << std::endl;
    for (const auto& stmt : body) {
      stmt->print(1 + depth);
    }

    if (!elseifNodes.empty()) {
      for (const auto& elsif : elseifNodes) {
        print_depth(depth);
        std::cout << "Else-If:" << std::endl;
        elsif->print(1 + depth);
      }
    }

    if (!elseBody.empty()) {
      print_depth(depth);
      std::cout << "Else:" << std::endl;
      for (const auto& stmt : elseBody) {
        stmt->print(1 + depth);
      }
    }
  }
};

class TryNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> tryBody;
  std::vector<std::unique_ptr<ASTNode>> catchBody;
  std::vector<std::unique_ptr<ASTNode>> finallyBody;
  std::unique_ptr<ASTNode> errorType;
  std::unique_ptr<ASTNode> errorMessage;

  TryNode() : ASTNode(ASTNodeType::TRY) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Try: " << std::endl;

    print_depth(depth);
    std::cout << "Try statements:" << std::endl;
    for (const auto& stmt : tryBody) {
      stmt->print(1 + depth);
    }

    if (!catchBody.empty()) {
      print_depth(depth);
      std::cout << "Catch:" << std::endl;

      if (errorType) {
        print_depth(1 + depth);
        std::cout << "Error type:" << std::endl;
        errorType->print(2 + depth);
      }

      if (errorMessage) {
        print_depth(1 + depth);
        std::cout << "Error message:" << std::endl;
        errorMessage->print(2 + depth);
      }

      print_depth(depth);
      std::cout << "Catch statements:" << std::endl;
      for (const auto& stmt : catchBody) {
        stmt->print(1 + depth);
      }
    }

    if (!finallyBody.empty()) {
      print_depth(depth);
      std::cout << "Finally statements:" << std::endl;
      for (const auto& stmt : finallyBody) {
        stmt->print(1 + depth);
      }
    }
  }
};

class FunctionCallNode : public ASTNode {
 public:
  std::string functionName;
  KName op;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  FunctionCallNode() : ASTNode(ASTNodeType::FUNCTION_CALL) {}
  FunctionCallNode(const std::string& functionName, const KName& op,
                   std::vector<std::unique_ptr<ASTNode>> arguments)
      : ASTNode(ASTNodeType::FUNCTION_CALL),
        functionName(functionName),
        op(op),
        arguments(std::move(arguments)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "FunctionCall: `" << functionName << "`" << std::endl;
    print_depth(depth);
    std::cout << "Arguments: " << std::endl;
    for (const auto& arg : arguments) {
      arg->print(1 + depth);
    }
  }
};

class MethodCallNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  std::string methodName;
  KName op;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  MethodCallNode(std::unique_ptr<ASTNode> object, const std::string& methodName,
                 const KName& op,
                 std::vector<std::unique_ptr<ASTNode>> arguments)
      : ASTNode(ASTNodeType::METHOD_CALL),
        object(std::move(object)),
        methodName(methodName),
        op(op),
        arguments(std::move(arguments)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "MethodCall: `" << methodName << "` on object: " << std::endl;
    object->print(1 + depth);
    print_depth(depth);
    std::cout << "Arguments:" << std::endl;
    for (const auto& arg : arguments) {
      arg->print(1 + depth);
    }
  }
};

class MemberAccessNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  std::string memberName;

  MemberAccessNode(std::unique_ptr<ASTNode> object,
                   const std::string& memberName)
      : ASTNode(ASTNodeType::MEMBER_ACCESS),
        object(std::move(object)),
        memberName(memberName) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "MemberAccess: `" << memberName
              << "` on object: " << std::endl;
    object->print(1 + depth);
  }
};

class AssignmentNode : public ASTNode {
 public:
  std::string name;
  KName op;
  std::unique_ptr<ASTNode> initializer;

  AssignmentNode() : ASTNode(ASTNodeType::ASSIGNMENT) {}
  AssignmentNode(const std::string& name, const KName& op,
                 std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::ASSIGNMENT),
        name(name),
        op(op),
        initializer(std::move(initializer)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Assignment: `" << name << "` " << Operators.get_op_string(op)
              << std::endl;
    print_depth(depth);
    std::cout << "Initializer:" << std::endl;
    initializer->print(1 + depth);
  }
};

class MemberAssignmentNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  std::string memberName;
  KName op;
  std::unique_ptr<ASTNode> initializer;

  MemberAssignmentNode(std::unique_ptr<ASTNode> object,
                       const std::string& memberName, KName op,
                       std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::MEMBER_ASSIGNMENT),
        object(std::move(object)),
        memberName(memberName),
        op(op),
        initializer(std::move(initializer)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "MemberAssignment: `" << memberName << "` "
              << Operators.get_op_string(op) << " on object: " << std::endl;
    print_depth(depth);
    object->print(1 + depth);
    print_depth(depth);
    std::cout << "Initializer:" << std::endl;
    initializer->print(1 + depth);
  }
};

#endif
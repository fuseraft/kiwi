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
  LAMBDA,
  FUNCTION_CALL,
  METHOD_CALL,
  MEMBER_ACCESS,
  TERNARY_OPERATION,
  BINARY_OPERATION,
  UNARY_OPERATION,
  LITERAL,
  HASH_LITERAL,
  LIST_LITERAL,
  RANGE_LITERAL,
  IDENTIFIER,
  INDEX_EXPRESSION,
  SLICE_EXPRESSION,
  EXPRESSION_STATEMENT,
  RETURN_STATEMENT,
  ASSIGNMENT,
  IF_STATEMENT,
  WHILE_LOOP,
  FOR_LOOP,
  REPEAT_LOOP,
  BLOCK_STATEMENT,
  PRINT_STATEMENT,
};

class ASTNode {
 public:
  ASTNodeType type;

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

class SliceNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> slicedObject;
  std::string name;
  std::unique_ptr<ASTNode> startExpression;
  std::unique_ptr<ASTNode> stopExpression;
  std::unique_ptr<ASTNode> stepExpression;

  SliceNode() : ASTNode(ASTNodeType::SLICE_EXPRESSION) {}

  SliceNode(const std::string& name,
            std::unique_ptr<ASTNode> startExpression = nullptr,
            std::unique_ptr<ASTNode> stopExpression = nullptr,
            std::unique_ptr<ASTNode> stepExpression = nullptr)
      : ASTNode(ASTNodeType::SLICE_EXPRESSION),
        name(name),
        startExpression(std::move(startExpression)),
        stopExpression(std::move(stopExpression)),
        stepExpression(std::move(stepExpression)) {}

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
    } else {
      std::cout << "Slice: `" << name << "`" << std::endl;
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
  std::vector<std::unique_ptr<ASTNode>> arguments;

  MethodCallNode(std::unique_ptr<ASTNode> object, const std::string& methodName,
                 std::vector<std::unique_ptr<ASTNode>> arguments)
      : ASTNode(ASTNodeType::METHOD_CALL),
        object(std::move(object)),
        methodName(methodName),
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
  KName type;
  std::unique_ptr<ASTNode> initializer;

  AssignmentNode() : ASTNode(ASTNodeType::ASSIGNMENT) {}
  AssignmentNode(const std::string& name, const KName& type,
                 std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::ASSIGNMENT),
        name(name),
        type(type),
        initializer(std::move(initializer)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Assignment: `" << name << "` "
              << Operators.get_op_string(type) << std::endl;
    print_depth(depth);
    std::cout << "Initializer:" << std::endl;
    initializer->print(1 + depth);
  }
};

class MemberAssignmentNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  std::string memberName;
  KName type;
  std::unique_ptr<ASTNode> initializer;

  MemberAssignmentNode(std::unique_ptr<ASTNode> object,
                       const std::string& memberName, KName type,
                       std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::ASSIGNMENT),
        object(std::move(object)),
        memberName(memberName),
        type(type),
        initializer(std::move(initializer)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "MemberAssignment: `" << memberName << "` "
              << Operators.get_op_string(type) << " on object: " << std::endl;
    print_depth(depth);
    object->print(1 + depth);
    print_depth(depth);
    std::cout << "Initializer:" << std::endl;
    initializer->print(1 + depth);
  }
};

class BlockStatementNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> statements;

  BlockStatementNode() : ASTNode(ASTNodeType::BLOCK_STATEMENT) {}
  BlockStatementNode(std::vector<std::unique_ptr<ASTNode>> statements)
      : ASTNode(ASTNodeType::BLOCK_STATEMENT),
        statements(std::move(statements)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "BlockStatement: " << std::endl;
    for (const auto& statement : statements) {
      statement->print(1 + depth);
    }
  }
};

#endif
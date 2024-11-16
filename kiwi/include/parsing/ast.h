#ifndef KIWI_PARSING_AST_H
#define KIWI_PARSING_AST_H

#include "tokens.h"
#include "tokentype.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

const Token astToken = Token::createExternal();

enum class ASTNodeType {
  ASSIGNMENT,
  BINARY_OPERATION,
  BREAK,
  CASE,
  CASE_WHEN,
  STRUCT,
  EXIT,
  EXPORT,  // obsolete
  FOR_LOOP,
  SPAWN,
  FUNCTION_CALL,
  FUNCTION,
  HASH_LITERAL,
  IDENTIFIER,
  IF,
  IMPORT,
  INDEX_ASSIGNMENT,
  INDEX,
  INTERFACE,  // obsolete
  LAMBDA,
  LAMBDA_CALL,
  LIST_LITERAL,
  LITERAL,
  MEMBER_ACCESS,
  MEMBER_ASSIGNMENT,
  METHOD_CALL,
  NEXT,
  NO_OP,
  PACK_ASSIGNMENT,
  PACKAGE,
  PARSE,  // WIP
  PRINT,
  PRINTXY,
  PROGRAM,
  RANGE_LITERAL,
  REPEAT_LOOP,
  RETURN,
  SELF,
  SLICE,
  TERNARY_OPERATION,
  THROW,
  TRY,
  UNARY_OPERATION,
  WHILE_LOOP,
};

class ASTNode {
 public:
  ASTNodeType type;
  Token token = astToken;

  ASTNode(ASTNodeType type) : type(type) {}
  virtual ~ASTNode() = default;

  virtual void print(int depth = 0) const = 0;
  virtual std::unique_ptr<ASTNode> clone() const = 0;
};

void print_depth(int depth) {
  for (int i = 0; i < depth; ++i) {
    std::cout << " ";
  }
}

class ProgramNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> statements;
  bool isScript = false;

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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedStatements;
    clonedStatements.reserve(statements.size());

    for (const auto& statement : statements) {
      clonedStatements.push_back(statement->clone());
    }

    return std::make_unique<ProgramNode>(std::move(clonedStatements));
  }
};

class StructNode : public ASTNode {
 public:
  k_string name;
  k_string baseClass;
  std::vector<k_string> interfaces;
  std::vector<std::unique_ptr<ASTNode>> methods;

  StructNode() : ASTNode(ASTNodeType::STRUCT) {}
  StructNode(const k_string& name, const k_string& baseClass,
             std::vector<k_string> interfaces,
             std::vector<std::unique_ptr<ASTNode>> methods)
      : ASTNode(ASTNodeType::STRUCT),
        name(name),
        baseClass(baseClass),
        interfaces(std::move(interfaces)),
        methods(std::move(methods)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Struct: " << name << std::endl;
    if (!baseClass.empty()) {
      print_depth(1 + depth);
      std::cout << "Base: " << baseClass << std::endl;
    }
    if (!interfaces.empty()) {
      print_depth(1 + depth);
      std::cout << "Interfaces:" << std::endl;
      for (const auto& iface : interfaces) {
        print_depth(2 + depth);
        std::cout << iface << std::endl;
      }
    }
    if (!methods.empty()) {
      print_depth(1 + depth);
      std::cout << "Methods:" << std::endl;
      for (const auto& def : methods) {
        def->print(2 + depth);
      }
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedMethods;
    clonedMethods.reserve(methods.size());
    for (const auto& method : methods) {
      clonedMethods.push_back(method->clone());
    }

    return std::make_unique<StructNode>(name, baseClass, interfaces,
                                        std::move(clonedMethods));
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<LiteralNode>(value);
  }
};

class HashLiteralNode : public ASTNode {
 public:
  std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> elements;
  std::vector<k_string> keys;

  HashLiteralNode(
      std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> elements,
      std::vector<k_string> keys)
      : ASTNode(ASTNodeType::HASH_LITERAL),
        elements(std::move(elements)),
        keys(std::move(keys)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "HashLiteral:" << std::endl;
    for (const auto& element : elements) {
      element.first->print(1 + depth);
      element.second->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> clonedElements;
    for (const auto& [key, value] : elements) {
      clonedElements.emplace(key->clone(), value->clone());
    }

    return std::make_unique<HashLiteralNode>(std::move(clonedElements), keys);
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedElements;
    clonedElements.reserve(elements.size());
    for (const auto& element : elements) {
      clonedElements.push_back(element->clone());
    }

    return std::make_unique<ListLiteralNode>(std::move(clonedElements));
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<RangeLiteralNode>(rangeStart->clone(),
                                              rangeEnd->clone());
  }
};

class IndexingNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> indexedObject;
  k_string name;
  std::unique_ptr<ASTNode> indexExpression;

  IndexingNode() : ASTNode(ASTNodeType::INDEX) {}

  IndexingNode(const k_string& name, std::unique_ptr<ASTNode> indexExpression)
      : ASTNode(ASTNodeType::INDEX),
        name(name),
        indexExpression(std::move(indexExpression)) {}

  IndexingNode(std::unique_ptr<ASTNode> indexedObject,
               std::unique_ptr<ASTNode> indexExpression)
      : ASTNode(ASTNodeType::INDEX),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<IndexingNode>(
        indexedObject ? indexedObject->clone() : nullptr,
        indexExpression->clone());
  }
};

class ReturnNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> returnValue;
  std::unique_ptr<ASTNode> condition;

  ReturnNode() : ASTNode(ASTNodeType::RETURN) {}
  ReturnNode(std::unique_ptr<ASTNode> returnValue,
             std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::RETURN),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ReturnNode>(
        returnValue ? returnValue->clone() : nullptr,
        condition ? condition->clone() : nullptr);
  }
};

class ThrowNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> errorValue;
  std::unique_ptr<ASTNode> condition;

  ThrowNode() : ASTNode(ASTNodeType::THROW) {}
  ThrowNode(std::unique_ptr<ASTNode> errorValue,
            std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::THROW),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ThrowNode>(
        errorValue ? errorValue->clone() : nullptr,
        condition ? condition->clone() : nullptr);
  }
};

class ExitNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> exitValue;
  std::unique_ptr<ASTNode> condition;

  ExitNode() : ASTNode(ASTNodeType::EXIT) {}
  ExitNode(std::unique_ptr<ASTNode> exitValue,
           std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::EXIT),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ExitNode>(exitValue ? exitValue->clone() : nullptr,
                                      condition ? condition->clone() : nullptr);
  }
};

class ParseNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> parseValue;

  ParseNode() : ASTNode(ASTNodeType::PARSE) {}
  ParseNode(std::unique_ptr<ASTNode> parseValue)
      : ASTNode(ASTNodeType::PARSE), parseValue(std::move(parseValue)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Parse:" << std::endl;
    if (parseValue) {
      parseValue->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ParseNode>(parseValue ? parseValue->clone()
                                                  : nullptr);
  }
};

class NextNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;

  NextNode() : ASTNode(ASTNodeType::NEXT) {}
  NextNode(std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::NEXT), condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Next:" << std::endl;

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<NextNode>(condition ? condition->clone() : nullptr);
  }
};

class BreakNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;

  BreakNode() : ASTNode(ASTNodeType::BREAK) {}
  BreakNode(std::unique_ptr<ASTNode> condition)
      : ASTNode(ASTNodeType::BREAK), condition(std::move(condition)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Break:" << std::endl;

    if (condition) {
      print_depth(1 + depth);
      std::cout << "When:" << std::endl;
      condition->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<BreakNode>(condition ? condition->clone()
                                                 : nullptr);
  }
};

class NoOpNode : public ASTNode {
 public:
  NoOpNode() : ASTNode(ASTNodeType::NO_OP) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "No operation:" << std::endl;
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<NoOpNode>(*this);
  }
};

class ImportNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> packageName;

  ImportNode() : ASTNode(ASTNodeType::IMPORT) {}
  ImportNode(std::unique_ptr<ASTNode> packageName)
      : ASTNode(ASTNodeType::IMPORT), packageName(std::move(packageName)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Import:" << std::endl;
    packageName->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ImportNode>(packageName->clone());
  }
};

class ExportNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> packageName;

  ExportNode() : ASTNode(ASTNodeType::EXPORT) {}
  ExportNode(std::unique_ptr<ASTNode> packageName)
      : ASTNode(ASTNodeType::EXPORT), packageName(std::move(packageName)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Export:" << std::endl;
    packageName->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<ExportNode>(packageName->clone());
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& statement : body) {
      clonedBody.push_back(statement->clone());
    }

    auto node = std::make_unique<PackageNode>(packageName->clone());
    node->body = std::move(clonedBody);
    return node;
  }
};

class SliceNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> slicedObject;
  std::unique_ptr<ASTNode> startExpression;
  std::unique_ptr<ASTNode> stopExpression;
  std::unique_ptr<ASTNode> stepExpression;

  SliceNode() : ASTNode(ASTNodeType::SLICE) {}
  SliceNode(std::unique_ptr<ASTNode> slicedObject,
            std::unique_ptr<ASTNode> startExpression = nullptr,
            std::unique_ptr<ASTNode> stopExpression = nullptr,
            std::unique_ptr<ASTNode> stepExpression = nullptr)
      : ASTNode(ASTNodeType::SLICE),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<SliceNode>(
        slicedObject->clone(),
        startExpression ? startExpression->clone() : nullptr,
        stopExpression ? stopExpression->clone() : nullptr,
        stepExpression ? stepExpression->clone() : nullptr);
  }
};

class IdentifierNode : public ASTNode {
 public:
  k_string name;
  k_string package;

  IdentifierNode() : ASTNode(ASTNodeType::IDENTIFIER) {}
  IdentifierNode(const k_string& name)
      : ASTNode(ASTNodeType::IDENTIFIER), name(name) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Identifier: `" << name << "`" << std::endl;
  }

  std::unique_ptr<ASTNode> clone() const override {
    auto node = std::make_unique<IdentifierNode>(name);
    node->package = package;
    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<TernaryOperationNode>(evalExpression->clone(),
                                                  trueExpression->clone(),
                                                  falseExpression->clone());
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<BinaryOperationNode>(left->clone(), op,
                                                 right->clone());
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<UnaryOperationNode>(op, operand->clone());
  }
};

class PrintNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> expression;  // Expression to print
  bool printNewline;                    // Flag for printing a newline
  bool printStdError;                   // Flag for printing to stderr

  PrintNode() : ASTNode(ASTNodeType::PRINT) {}
  PrintNode(std::unique_ptr<ASTNode> expression, bool printNewline,
            bool printStdError)
      : ASTNode(ASTNodeType::PRINT),
        expression(std::move(expression)),
        printNewline(printNewline),
        printStdError(printStdError) {}

  void print(int depth) const override {
    print_depth(depth);
    if (printStdError) {
      std::cout << (printNewline ? "Print error line:" : "Print error:")
                << std::endl;
    } else {
      std::cout << (printNewline ? "Print line:" : "Print:") << std::endl;
    }
    expression->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<PrintNode>(expression->clone(), printNewline,
                                       printStdError);
  }
};

class PrintXyNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> expression;  // Expression to print
  std::unique_ptr<ASTNode> x;
  std::unique_ptr<ASTNode> y;

  PrintXyNode() : ASTNode(ASTNodeType::PRINTXY) {}
  PrintXyNode(std::unique_ptr<ASTNode> expression, std::unique_ptr<ASTNode> x,
              std::unique_ptr<ASTNode> y)
      : ASTNode(ASTNodeType::PRINTXY),
        expression(std::move(expression)),
        x(std::move(x)),
        y(std::move(y)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "PrintXy:" << std::endl;
    expression->print(1 + depth);
    x->print(1 + depth);
    y->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<PrintXyNode>(expression->clone(), x->clone(),
                                         y->clone());
  }
};

class FunctionDeclarationNode : public ASTNode {
 public:
  k_string name;
  std::vector<std::pair<k_string, std::unique_ptr<ASTNode>>> parameters;
  std::vector<std::unique_ptr<ASTNode>> body;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;
  bool isStatic = false;
  bool isPrivate = false;

  FunctionDeclarationNode() : ASTNode(ASTNodeType::FUNCTION) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "FunctionDeclaration: `" << name << "`" << std::endl;

    if (isStatic || isPrivate) {
      print_depth(1 + depth);
      std::cout << "Modifiers:" << std::endl;
      print_depth(2 + depth);
      std::cout << "Private:" << std::boolalpha << isPrivate;
      std::cout << ", Static:" << std::boolalpha << isStatic << std::endl;
    }

    print_depth(1 + depth);
    std::cout << "Return Type: "
              << Serializer::get_typename_string(returnTypeHint) << std::endl;

    print_depth(1 + depth);
    std::cout << "Parameters: " << std::endl;
    for (const auto& param : parameters) {
      print_depth(2 + depth);
      std::cout << param.first;

      if (typeHints.find(param.first) != typeHints.end()) {
        std::cout << std::endl;
        print_depth(2 + depth);
        auto typeHint = typeHints.at(param.first);
        std::cout << "Parameter Type: "
                  << Serializer::get_typename_string(typeHint);
      }

      if (param.second) {
        std::cout << std::endl;
        print_depth(2 + depth);
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

  std::unique_ptr<ASTNode> clone() const override {
    // Deep copy parameters
    std::vector<std::pair<k_string, std::unique_ptr<ASTNode>>> clonedParameters;
    clonedParameters.reserve(parameters.size());
    for (const auto& param : parameters) {
      clonedParameters.emplace_back(
          param.first, param.second ? param.second->clone() : nullptr);
    }

    // Deep copy body
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<FunctionDeclarationNode>();
    node->name = name;
    node->parameters = std::move(clonedParameters);
    node->body = std::move(clonedBody);
    node->isStatic = isStatic;
    node->isPrivate = isPrivate;
    node->typeHints = typeHints;
    node->returnTypeHint = returnTypeHint;
    return node;
  }
};

class LambdaNode : public ASTNode {
 public:
  std::vector<std::pair<k_string, std::unique_ptr<ASTNode>>> parameters;
  std::vector<std::unique_ptr<ASTNode>> body;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;

  LambdaNode() : ASTNode(ASTNodeType::LAMBDA) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Lambda: " << std::endl;

    print_depth(1 + depth);
    std::cout << "Return Type: "
              << Serializer::get_typename_string(returnTypeHint) << std::endl;

    print_depth(depth);
    std::cout << "Parameters: " << std::endl;
    for (const auto& param : parameters) {
      print_depth(1 + depth);
      std::cout << param.first;

      if (typeHints.find(param.first) != typeHints.end()) {
        std::cout << std::endl;
        print_depth(1 + depth);
        auto typeHint = typeHints.at(param.first);
        std::cout << "Parameter Type: "
                  << Serializer::get_typename_string(typeHint);
      }

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

  std::unique_ptr<ASTNode> clone() const override {
    // Deep copy parameters
    std::vector<std::pair<k_string, std::unique_ptr<ASTNode>>> clonedParameters;
    clonedParameters.reserve(parameters.size());
    for (const auto& param : parameters) {
      clonedParameters.emplace_back(
          param.first, param.second ? param.second->clone() : nullptr);
    }

    // Deep copy body
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<LambdaNode>();
    node->parameters = std::move(clonedParameters);
    node->body = std::move(clonedBody);
    node->typeHints = typeHints;
    node->returnTypeHint = returnTypeHint;
    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    // Deep copy body
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<ForLoopNode>();
    node->dataSet = dataSet->clone();
    node->valueIterator = valueIterator->clone();
    node->indexIterator = indexIterator ? indexIterator->clone() : nullptr;
    node->body = std::move(clonedBody);

    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    // Deep copy body
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<WhileLoopNode>();
    node->condition = condition->clone();
    node->body = std::move(clonedBody);
    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    // Deep copy body
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<RepeatLoopNode>();
    node->count = count->clone();
    node->alias = alias ? alias->clone() : nullptr;
    node->body = std::move(clonedBody);
    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<CaseWhenNode>();
    node->condition = condition->clone();
    node->body = std::move(clonedBody);
    return node;
  }
};

class CaseNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> testValue;
  std::vector<std::unique_ptr<ASTNode>> elseBody;
  std::vector<std::unique_ptr<CaseWhenNode>> whenNodes;

  CaseNode() : ASTNode(ASTNodeType::CASE) {}

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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedElseBody;
    clonedElseBody.reserve(elseBody.size());
    for (const auto& stmt : elseBody) {
      clonedElseBody.push_back(stmt->clone());
    }

    std::vector<std::unique_ptr<CaseWhenNode>> clonedWhenNodes;
    clonedWhenNodes.reserve(whenNodes.size());
    for (const auto& when : whenNodes) {
      clonedWhenNodes.push_back(std::unique_ptr<CaseWhenNode>(
          static_cast<CaseWhenNode*>(when->clone().release())));
    }

    auto node = std::make_unique<CaseNode>();
    node->testValue = testValue ? testValue->clone() : nullptr;
    node->elseBody = std::move(clonedElseBody);
    node->whenNodes = std::move(clonedWhenNodes);
    return node;
  }
};

class IfNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;
  std::vector<std::unique_ptr<ASTNode>> elseBody;
  std::vector<std::unique_ptr<IfNode>> elseifNodes;

  IfNode() : ASTNode(ASTNodeType::IF) {}

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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedBody;
    clonedBody.reserve(body.size());
    for (const auto& stmt : body) {
      clonedBody.push_back(stmt->clone());
    }

    std::vector<std::unique_ptr<ASTNode>> clonedElseBody;
    clonedElseBody.reserve(elseBody.size());
    for (const auto& stmt : elseBody) {
      clonedElseBody.push_back(stmt->clone());
    }

    std::vector<std::unique_ptr<IfNode>> clonedElseifNodes;
    clonedElseifNodes.reserve(elseifNodes.size());
    for (const auto& elif : elseifNodes) {
      clonedElseifNodes.push_back(std::unique_ptr<IfNode>(
          static_cast<IfNode*>(elif->clone().release())));
    }

    auto node = std::make_unique<IfNode>();
    node->condition = condition->clone();
    node->body = std::move(clonedBody);
    node->elseBody = std::move(clonedElseBody);
    node->elseifNodes = std::move(clonedElseifNodes);
    return node;
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedTryBody;
    clonedTryBody.reserve(tryBody.size());
    for (const auto& stmt : tryBody) {
      clonedTryBody.push_back(stmt->clone());
    }

    std::vector<std::unique_ptr<ASTNode>> clonedCatchBody;
    clonedCatchBody.reserve(catchBody.size());
    for (const auto& stmt : catchBody) {
      clonedCatchBody.push_back(stmt->clone());
    }

    std::vector<std::unique_ptr<ASTNode>> clonedFinallyBody;
    clonedFinallyBody.reserve(finallyBody.size());
    for (const auto& stmt : finallyBody) {
      clonedFinallyBody.push_back(stmt->clone());
    }

    auto node = std::make_unique<TryNode>();
    node->tryBody = std::move(clonedTryBody);
    node->catchBody = std::move(clonedCatchBody);
    node->finallyBody = std::move(clonedFinallyBody);
    node->errorType = errorType ? errorType->clone() : nullptr;
    node->errorMessage = errorMessage ? errorMessage->clone() : nullptr;
    return node;
  }
};

class FunctionCallNode : public ASTNode {
 public:
  k_string functionName;
  KName op;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  FunctionCallNode() : ASTNode(ASTNodeType::FUNCTION_CALL) {}
  FunctionCallNode(const k_string& functionName, const KName& op,
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedArguments;
    clonedArguments.reserve(arguments.size());
    for (const auto& arg : arguments) {
      clonedArguments.push_back(arg->clone());
    }

    return std::make_unique<FunctionCallNode>(functionName, op,
                                              std::move(clonedArguments));
  }
};

class LambdaCallNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> lambdaNode;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  LambdaCallNode() : ASTNode(ASTNodeType::LAMBDA_CALL) {}
  LambdaCallNode(std::unique_ptr<ASTNode> lambdaNode,
                 std::vector<std::unique_ptr<ASTNode>> arguments)
      : ASTNode(ASTNodeType::LAMBDA_CALL),
        lambdaNode(std::move(lambdaNode)),
        arguments(std::move(arguments)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "LambdaCall:" << std::endl;
    print_depth(depth);
    std::cout << "Arguments:" << std::endl;
    for (const auto& arg : arguments) {
      arg->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedArguments;
    clonedArguments.reserve(arguments.size());
    for (const auto& arg : arguments) {
      clonedArguments.push_back(arg->clone());
    }

    return std::make_unique<LambdaCallNode>(lambdaNode->clone(),
                                            std::move(clonedArguments));
  }
};

class MethodCallNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  k_string methodName;
  KName op;
  std::vector<std::unique_ptr<ASTNode>> arguments;

  MethodCallNode(std::unique_ptr<ASTNode> object, const k_string& methodName,
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

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedArguments;
    clonedArguments.reserve(arguments.size());
    for (const auto& arg : arguments) {
      clonedArguments.push_back(arg->clone());
    }

    return std::make_unique<MethodCallNode>(object->clone(), methodName, op,
                                            std::move(clonedArguments));
  }
};

class MemberAccessNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  k_string memberName;

  MemberAccessNode(std::unique_ptr<ASTNode> object, const k_string& memberName)
      : ASTNode(ASTNodeType::MEMBER_ACCESS),
        object(std::move(object)),
        memberName(memberName) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "MemberAccess: `" << memberName
              << "` on object: " << std::endl;
    object->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<MemberAccessNode>(object->clone(), memberName);
  }
};

class SelfNode : public ASTNode {
 public:
  k_string name;
  SelfNode() : ASTNode(ASTNodeType::SELF) {}
  SelfNode(const k_string& name) : ASTNode(ASTNodeType::SELF), name(name) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Self:" << std::endl;

    if (!name.empty()) {
      print_depth(1 + depth);
      std::cout << "Name: " << name << std::endl;
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<SelfNode>(name);
  }
};

class IndexAssignmentNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  KName op;
  std::unique_ptr<ASTNode> initializer;

  IndexAssignmentNode() : ASTNode(ASTNodeType::INDEX_ASSIGNMENT) {}
  IndexAssignmentNode(std::unique_ptr<ASTNode> object, const KName& op,
                      std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::INDEX_ASSIGNMENT),
        object(std::move(object)),
        op(op),
        initializer(std::move(initializer)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "IndexAssignment:" << std::endl;
    object->print(1 + depth);
    print_depth(1 + depth);
    std::cout << Operators.get_op_string(op) << std::endl;
    print_depth(1 + depth);
    std::cout << "Initializer:" << std::endl;
    initializer->print(2 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<IndexAssignmentNode>(object->clone(), op,
                                                 initializer->clone());
  }
};

class PackAssignmentNode : public ASTNode {
 public:
  std::vector<std::unique_ptr<ASTNode>> left;
  std::vector<std::unique_ptr<ASTNode>> right;
  KName op;

  PackAssignmentNode() : ASTNode(ASTNodeType::PACK_ASSIGNMENT) {}
  PackAssignmentNode(std::vector<std::unique_ptr<ASTNode>> left,
                     std::vector<std::unique_ptr<ASTNode>> right,
                     const KName& op)
      : ASTNode(ASTNodeType::PACK_ASSIGNMENT),
        left(std::move(left)),
        right(std::move(right)),
        op(op) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "PackAssignment: " << Operators.get_op_string(op) << std::endl;
    print_depth(depth);
    std::cout << "Left-hand side:" << std::endl;
    for (const auto& lhs : left) {
      lhs->print(1 + depth);
    }
    print_depth(depth);
    std::cout << "Right-hand side:" << std::endl;
    for (const auto& rhs : right) {
      rhs->print(1 + depth);
    }
  }

  std::unique_ptr<ASTNode> clone() const override {
    std::vector<std::unique_ptr<ASTNode>> clonedLeft;
    clonedLeft.reserve(left.size());
    for (const auto& lhs : left) {
      clonedLeft.push_back(lhs->clone());
    }

    std::vector<std::unique_ptr<ASTNode>> clonedRight;
    clonedRight.reserve(right.size());
    for (const auto& rhs : right) {
      clonedRight.push_back(rhs->clone());
    }

    return std::make_unique<PackAssignmentNode>(std::move(clonedLeft),
                                                std::move(clonedRight), op);
  }
};

class AssignmentNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> left;
  k_string name;
  KName op;
  std::unique_ptr<ASTNode> initializer;

  AssignmentNode() : ASTNode(ASTNodeType::ASSIGNMENT) {}
  AssignmentNode(std::unique_ptr<ASTNode> left, const k_string& name,
                 const KName& op, std::unique_ptr<ASTNode> initializer)
      : ASTNode(ASTNodeType::ASSIGNMENT),
        left(std::move(left)),
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<AssignmentNode>(left->clone(), name, op,
                                            initializer->clone());
  }
};

class MemberAssignmentNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> object;
  k_string memberName;
  KName op;
  std::unique_ptr<ASTNode> initializer;

  MemberAssignmentNode(std::unique_ptr<ASTNode> object,
                       const k_string& memberName, KName op,
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

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<MemberAssignmentNode>(object->clone(), memberName,
                                                  op, initializer->clone());
  }
};

class SpawnNode : public ASTNode {
 public:
  std::unique_ptr<ASTNode> expression;

  SpawnNode() : ASTNode(ASTNodeType::SPAWN) {}
  SpawnNode(std::unique_ptr<ASTNode> expression)
      : ASTNode(ASTNodeType::SPAWN), expression(std::move(expression)) {}

  void print(int depth) const override {
    print_depth(depth);
    std::cout << "Spawn:" << std::endl;
    expression->print(1 + depth);
  }

  std::unique_ptr<ASTNode> clone() const override {
    return std::make_unique<SpawnNode>(expression->clone());
  }
};

enum class KCallableType {
  Builtin,
  Function,
  Method,
  Lambda,
};

class KCallable {
 public:
  KCallableType type;
  std::vector<std::pair<k_string, k_value>> parameters;
  std::unordered_set<k_string> defaultParameters;
  KCallable(KCallableType type) : type(type) {}
  virtual ~KCallable() = default;
  virtual const std::vector<std::unique_ptr<ASTNode>>& getBody() const = 0;
};

class KBuiltin : public KCallable {
 public:
  k_string name;
  Token token;

  KBuiltin(const Token& token, const k_string& name)
      : KCallable(KCallableType::Builtin), name(name), token(token) {}

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return body;
  }

  std::unique_ptr<KCallable> clone() {
    auto cloned = std::make_unique<KBuiltin>(token, name);
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    return cloned;
  }

 private:
  std::vector<std::unique_ptr<ASTNode>> body;
};

class KFunction : public KCallable {
 public:
  k_string name;
  std::unique_ptr<FunctionDeclarationNode> decl;
  bool isStatic = false;
  bool isPrivate = false;
  bool isCtor = false;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;

  KFunction(std::unique_ptr<ASTNode> node)
      : KCallable(KCallableType::Function) {
    std::unique_ptr<FunctionDeclarationNode> nodeptr(
        dynamic_cast<FunctionDeclarationNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return decl->body;
  }

  std::unique_ptr<KFunction> clone() {
    std::unique_ptr<FunctionDeclarationNode> nodeptr(
        dynamic_cast<FunctionDeclarationNode*>(decl->clone().release()));
    auto cloned = std::make_unique<KFunction>(std::move(nodeptr));
    cloned->name = name;
    cloned->isStatic = isStatic;
    cloned->isPrivate = isPrivate;
    cloned->isCtor = isCtor;
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    cloned->typeHints = typeHints;
    cloned->returnTypeHint = returnTypeHint;
    return cloned;
  }
};

class KLambda : public KCallable {
 public:
  std::unique_ptr<LambdaNode> decl;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;

  KLambda(std::unique_ptr<ASTNode> node) : KCallable(KCallableType::Lambda) {
    std::unique_ptr<LambdaNode> nodeptr(
        dynamic_cast<LambdaNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return decl->body;
  }

  std::unique_ptr<KLambda> clone() {
    std::unique_ptr<LambdaNode> nodeptr(
        dynamic_cast<LambdaNode*>(decl->clone().release()));
    auto cloned = std::make_unique<KLambda>(std::move(nodeptr));
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    cloned->typeHints = typeHints;
    cloned->returnTypeHint = returnTypeHint;
    return cloned;
  }
};

class KClass {
 public:
  k_string name;
  k_string baseClass;
  std::unordered_map<k_string, std::unique_ptr<KFunction>> methods;

  std::unique_ptr<KClass> clone() const {
    auto cloned = std::make_unique<KClass>();
    cloned->name = name;
    cloned->baseClass = baseClass;

    for (const auto& [methodName, methodPtr] : methods) {
      cloned->methods[methodName] = std::unique_ptr<KFunction>(
          static_cast<KFunction*>(methodPtr->clone().release()));
    }

    return cloned;
  }
};

class KPackage {
 public:
  std::unique_ptr<PackageNode> decl;

  KPackage(std::unique_ptr<ASTNode> node) {
    std::unique_ptr<PackageNode> nodeptr(
        dynamic_cast<PackageNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  std::unique_ptr<KPackage> clone() const {
    std::unique_ptr<PackageNode> nodeptr(
        dynamic_cast<PackageNode*>(decl->clone().release()));
    return std::make_unique<KPackage>(std::move(nodeptr));
  }
};

#endif
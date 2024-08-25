#ifndef KIWI_INTERPRETER_H
#define KIWI_INTERPRETER_H

#include <memory>
#include <iostream>  // Assuming you need to include this for std::cout

#include "globals.h"
#include "math/functions.h"
#include "parsing/ast.h"
#include "typing/value.h"

class KInterpreter {
 public:
  KInterpreter() {}

  k_value interpret(const ASTNode* node);
  k_value visit(const ProgramNode* node);
  k_value visit(const AssignmentNode* node);
  k_value visit(const IdentifierNode* node);
  k_value visit(const LiteralNode* node);
  k_value visit(const ListLiteralNode* node);
  k_value visit(const HashLiteralNode* node);
  k_value visit(const PrintNode* node);
  k_value visit(const UnaryOperationNode* node);
  k_value visit(const BinaryOperationNode* node);
};

k_value KInterpreter::interpret(const ASTNode* node) {
  if (node->type == ASTNodeType::PROGRAM) {
    // No need to cast, just pass the raw pointer
    const auto* program = static_cast<const ProgramNode*>(node);
    return visit(program);
  } else if (node->type == ASTNodeType::ASSIGNMENT) {
    const auto* assignment = static_cast<const AssignmentNode*>(node);
    return visit(assignment);
  } else if (node->type == ASTNodeType::LITERAL) {
    const auto* literal = static_cast<const LiteralNode*>(node);
    return visit(literal);
  } else if (node->type == ASTNodeType::LIST_LITERAL) {
    const auto* literal = static_cast<const ListLiteralNode*>(node);
    return visit(literal);
  } else if (node->type == ASTNodeType::HASH_LITERAL) {
    const auto* literal = static_cast<const HashLiteralNode*>(node);
    return visit(literal);
  } else if (node->type == ASTNodeType::IDENTIFIER) {
    const auto* identifier = static_cast<const IdentifierNode*>(node);
    return visit(identifier);
  } else if (node->type == ASTNodeType::PRINT_STATEMENT) {
    const auto* print = static_cast<const PrintNode*>(node);
    return visit(print);
  } else if (node->type == ASTNodeType::BINARY_OPERATION) {
    const auto* binaryOp = static_cast<const BinaryOperationNode*>(node);
    return visit(binaryOp);
  } else {
    node->print();
  }
  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ProgramNode* node) {
  auto frame = std::make_shared<CallStackFrame>();
  callStack.push(frame);

  for (const auto& stmt : node->statements) {
    interpret(stmt.get());
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const AssignmentNode* node) {
  auto frame = callStack.top();
  auto value = interpret(node->initializer.get());
  auto type = node->type;
  auto name = node->name;

  if (type == KName::Ops_Assign) {
    if (name == Keywords.Global) {
      throw IllegalNameError(node->token, name);
    }
    frame->variables[name] = value;
  } else {
    if (!frame->hasVariable(name)) {
      throw VariableUndefinedError(node->token, name);
    }

    auto oldValue = frame->variables[name];

    if (type == KName::Ops_BitwiseNotAssign) {
      frame->variables[name] = MathImpl.do_bitwise_not(node->token, oldValue);
    } else {
      frame->variables[name] = MathImpl.do_binary_op(node->token, type, oldValue, value);
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const IdentifierNode* node) {
  auto frame = callStack.top();

  if (frame->hasVariable(node->name)) {
    return frame->variables[node->name];
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const LiteralNode* node) {
  return node->value;
}

k_value KInterpreter::visit(const ListLiteralNode* node) {
  std::vector<k_value> elements;
  elements.reserve(node->elements.size());
  for (const auto& element : node->elements) {
    const auto& value = interpret(element.get());
    elements.emplace_back(value);
  }
  return std::make_shared<List>(elements);
}

k_value KInterpreter::visit(const HashLiteralNode* node) {
  auto hash = std::make_shared<Hash>();
  for (const auto& pair : node->elements) {
    auto key = interpret(pair.first.get());
    auto value = interpret(pair.second.get());

    if (!std::holds_alternative<k_string>(key)) {
      throw SyntaxError(node->token, "Hash key must be a string value.");
    }

    hash->add(std::get<k_string>(key), value);
  }
  return hash;
}

k_value KInterpreter::visit(const PrintNode* node) {
  auto value = interpret(node->expression.get());
  if (node->printNewline) {
    std::cout << Serializer::serialize(value) << std::endl;
  } else {
    std::cout << Serializer::serialize(value);
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const UnaryOperationNode* node) {
  auto right = interpret(node->operand.get());
  return MathImpl.do_unary_op(node->token, node->op, right);
}

k_value KInterpreter::visit(const BinaryOperationNode* node) {
  auto left = interpret(node->left.get());
  auto right = interpret(node->right.get());
  return MathImpl.do_binary_op(node->token, node->op, left, right);
}

#endif  // KIWI_INTERPRETER_H

#ifndef KIWI_INTERPRETER_H
#define KIWI_INTERPRETER_H

#include <memory>
#include <iostream>

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
  k_value visit(const RangeLiteralNode* node);
  k_value visit(const HashLiteralNode* node);
  k_value visit(const PrintNode* node);
  k_value visit(const UnaryOperationNode* node);
  k_value visit(const BinaryOperationNode* node);
  k_value visit(const IfNode* node);
  k_value visit(const CaseNode* node);
  k_value visit(const ForLoopNode* node);
};

k_value KInterpreter::interpret(const ASTNode* node) {
  switch (node->type) {
    case ASTNodeType::PROGRAM:
      return visit(static_cast<const ProgramNode*>(node));

    case ASTNodeType::ASSIGNMENT:
      return visit(static_cast<const AssignmentNode*>(node));

    case ASTNodeType::LITERAL:
      return visit(static_cast<const LiteralNode*>(node));

    case ASTNodeType::LIST_LITERAL:
      return visit(static_cast<const ListLiteralNode*>(node));

    case ASTNodeType::RANGE_LITERAL:
      return visit(static_cast<const RangeLiteralNode*>(node));

    case ASTNodeType::HASH_LITERAL:
      return visit(static_cast<const HashLiteralNode*>(node));

    case ASTNodeType::IDENTIFIER:
      return visit(static_cast<const IdentifierNode*>(node));

    case ASTNodeType::PRINT_STATEMENT:
      return visit(static_cast<const PrintNode*>(node));

    case ASTNodeType::BINARY_OPERATION:
      return visit(static_cast<const BinaryOperationNode*>(node));

    case ASTNodeType::IF_STATEMENT:
      return visit(static_cast<const IfNode*>(node));

    case ASTNodeType::CASE_STATEMENT:
      return visit(static_cast<const CaseNode*>(node));

    case ASTNodeType::FOR_LOOP:
      return visit(static_cast<const ForLoopNode*>(node));

    default:
      node->print();
      break;
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
      frame->variables[name] =
          MathImpl.do_binary_op(node->token, type, oldValue, value);
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

k_value KInterpreter::visit(const RangeLiteralNode* node) {
  auto startValue = interpret(node->rangeStart.get());
  auto stopValue = interpret(node->rangeEnd.get());

  if (!std::holds_alternative<k_int>(startValue) ||
      !std::holds_alternative<k_int>(stopValue)) {
    throw RangeError(node->token, "Range value must be an integer.");
  }

  auto start = std::get<k_int>(startValue);
  auto stop = std::get<k_int>(stopValue);
  auto step = (stop < start) ? -1 : 1;
  size_t numElements = static_cast<size_t>(std::abs(stop - start)) + 1;

  auto list = std::make_shared<List>();
  auto& elements = list->elements;
  elements.reserve(numElements);

  for (auto i = start; i != stop; i += step) {
    elements.emplace_back(i);
  }

  elements.emplace_back(stop);

  return list;
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

k_value KInterpreter::visit(const IfNode* node) {
  auto conditionValue = interpret(node->condition.get());

  if (MathImpl.is_truthy(conditionValue)) {
    for (const auto& stmt : node->body) {
      interpret(stmt.get());
    }
  } else {
    bool executed = false;
    for (const auto& elseifNode : node->elseifNodes) {
      auto elseifConditionValue = interpret(elseifNode->condition.get());
      if (MathImpl.is_truthy(elseifConditionValue)) {
        for (const auto& stmt : elseifNode->body) {
          interpret(stmt.get());
        }
        executed = true;
        break;
      }
    }

    if (!executed && !node->elseBody.empty()) {
      for (const auto& stmt : node->elseBody) {
        interpret(stmt.get());
      }
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const CaseNode* node) {
  k_value testValue = interpret(node->testValue.get());

  for (const auto& whenNode : node->whenNodes) {
    k_value whenCondition = interpret(whenNode->condition.get());

    if (std::get<bool>(MathImpl.do_eq_comparison(testValue, whenCondition))) {
      for (const auto& stmt : whenNode->body) {
        interpret(stmt.get());
      }
      return static_cast<k_int>(0);
    }
  }

  if (!node->elseBody.empty()) {
    for (const auto& stmt : node->elseBody) {
      interpret(stmt.get());
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ForLoopNode* node) {
  k_value dataSetValue = interpret(node->dataSet.get());

  if (std::holds_alternative<std::shared_ptr<List>>(dataSetValue)) {
    auto list = std::get<std::shared_ptr<List>>(dataSetValue);
    auto frame = callStack.top();
    k_string valueIteratorName;
    k_string indexIteratorName;
    bool hasIndexIterator = false;

    const auto* valueIterator =
        static_cast<const IdentifierNode*>(node->valueIterator.get());
    valueIteratorName = valueIterator->name;

    if (node->indexIterator) {
      const auto* indexIterator =
          static_cast<const IdentifierNode*>(node->indexIterator.get());
      indexIteratorName = indexIterator->name;
      hasIndexIterator = true;
    }

    const auto& elements = list->elements;

    for (size_t i = 0; i < elements.size(); ++i) {
      frame->variables[valueIteratorName] = elements.at(i);

      if (hasIndexIterator) {
        frame->variables[indexIteratorName] = static_cast<k_int>(i);
      }

      for (const auto& stmt : node->body) {
        k_value result = interpret(stmt.get());

        if (stmt->type == ASTNodeType::NEXT_STATEMENT) {
          const auto* nextNode = static_cast<const NextNode*>(stmt.get());
          if (!nextNode->condition ||
              MathImpl.is_truthy(interpret(nextNode->condition.get()))) {
            break;
          }
        } else if (stmt->type == ASTNodeType::BREAK_STATEMENT) {
          const auto* breakNode = static_cast<const BreakNode*>(stmt.get());
          if (!breakNode->condition ||
              MathImpl.is_truthy(interpret(breakNode->condition.get()))) {
            return static_cast<k_int>(0);
          }
        }
      }
    }
  }

  return static_cast<k_int>(0);
}

#endif
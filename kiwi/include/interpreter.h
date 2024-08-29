#ifndef KIWI_INTERPRETER_H
#define KIWI_INTERPRETER_H

#include <algorithm>
#include <memory>
#include <iostream>

#include "globals.h"
#include "interp_helper.h"
#include "math/functions.h"
#include "parsing/ast.h"
#include "parsing/builtins.h"
#include "tracing/error.h"
#include "typing/value.h"

class KFunction {
 public:
  std::string name;
  std::vector<std::pair<std::string, k_value>> parameters;
  const FunctionDeclarationNode* decl;
  std::unordered_set<std::string> defaultParameters;

  KFunction(const FunctionDeclarationNode* decl) : decl(decl) {}
};

class KPackage {
 public:
  const PackageNode* decl;
  KPackage(const PackageNode* decl) : decl(decl) {}
};

class KLambda {
 public:
  std::vector<std::pair<std::string, k_value>> parameters;
  const LambdaNode* decl;
  std::unordered_set<std::string> defaultParameters;

  KLambda(const LambdaNode* decl) : decl(decl) {}
};

class KInterpreter {
 public:
  KInterpreter() {}

  k_value interpret(const ASTNode* node);

 private:
  std::unordered_map<k_string, std::shared_ptr<KPackage>> packages;
  std::unordered_map<k_string, std::shared_ptr<KFunction>> functions;
  std::unordered_map<k_string, std::shared_ptr<KLambda>> lambdas;

  std::shared_ptr<CallStackFrame> createFrame(bool isMethodInvocation);
  k_value dropFrame();
  void importPackage(const k_value& packageName, const Token& token);
  k_string id(const ASTNode* node);
  k_value visit(const ProgramNode* node);
  k_value visit(const AssignmentNode* node);
  k_value visit(const MemberAssignmentNode* node);
  k_value visit(const IdentifierNode* node);
  k_value visit(const LiteralNode* node);
  k_value visit(const ListLiteralNode* node);
  k_value visit(const RangeLiteralNode* node);
  k_value visit(const HashLiteralNode* node);
  k_value visit(const PrintNode* node);
  k_value visit(const UnaryOperationNode* node);
  k_value visit(const BinaryOperationNode* node);
  k_value visit(const TernaryOperationNode* node);
  k_value visit(const IfNode* node);
  k_value visit(const CaseNode* node);
  k_value visit(const ForLoopNode* node);
  k_value visit(const WhileLoopNode* node);
  k_value visit(const RepeatLoopNode* node);
  k_value visit(const TryNode* node);
  k_value visit(const LambdaNode* node);
  k_value visit(const FunctionDeclarationNode* node);
  k_value visit(const FunctionCallNode* node);
  k_value visit(const MethodCallNode* node);
  k_value visit(const PackageNode* node);
  k_value visit(const ImportNode* node);
  k_value visit(const ExportNode* node);
  k_value visit(const ExitNode* node);
  k_value visit(const ThrowNode* node);
  k_value visit(const ReturnNode* node);
  k_value visit(const IndexingNode* node);
  k_value visit(const SliceNode* node);
  SliceIndex getSlice(const SliceNode* node, k_value object);

  k_value listSum(const k_list& list);
  k_value listMin(const Token& token, const k_list& list);
  k_value listMax(const Token& token, const k_list& list);
  k_value listSort(const k_list& list);
  k_value lambdaEach(std::shared_ptr<KLambda> lambda, const k_list& list);
  k_value lambdaNone(std::shared_ptr<KLambda> lambda, const k_list& list);
  k_value lambdaMap(std::shared_ptr<KLambda> lambda, const k_list& list);
  k_value lambdaReduce(std::shared_ptr<KLambda> lambda, k_value accumulator,
                       const k_list& list);
  k_value lambdaSelect(std::shared_ptr<KLambda> lambda, const k_list& list);
  k_value interpretListBuiltin(const Token& token, k_value& object,
                               const KName& op, std::vector<k_value> arguments);
};

k_value KInterpreter::interpret(const ASTNode* node) {
  switch (node->type) {
    case ASTNodeType::PROGRAM:
      return visit(static_cast<const ProgramNode*>(node));

    case ASTNodeType::PACKAGE:
      return visit(static_cast<const PackageNode*>(node));

    case ASTNodeType::IMPORT_STATEMENT:
      return visit(static_cast<const ImportNode*>(node));

    case ASTNodeType::EXPORT_STATEMENT:
      return visit(static_cast<const ExportNode*>(node));

    case ASTNodeType::EXIT_STATEMENT:
      return visit(static_cast<const ExitNode*>(node));

    case ASTNodeType::THROW_STATEMENT:
      return visit(static_cast<const ThrowNode*>(node));

    case ASTNodeType::ASSIGNMENT:
      return visit(static_cast<const AssignmentNode*>(node));

    case ASTNodeType::MEMBER_ASSIGNMENT:
      return visit(static_cast<const MemberAssignmentNode*>(node));

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

    case ASTNodeType::TERNARY_OPERATION:
      return visit(static_cast<const TernaryOperationNode*>(node));

    case ASTNodeType::BINARY_OPERATION:
      return visit(static_cast<const BinaryOperationNode*>(node));

    case ASTNodeType::UNARY_OPERATION:
      return visit(static_cast<const UnaryOperationNode*>(node));

    case ASTNodeType::IF_STATEMENT:
      return visit(static_cast<const IfNode*>(node));

    case ASTNodeType::CASE_STATEMENT:
      return visit(static_cast<const CaseNode*>(node));

    case ASTNodeType::FOR_LOOP:
      return visit(static_cast<const ForLoopNode*>(node));

    case ASTNodeType::WHILE_LOOP:
      return visit(static_cast<const WhileLoopNode*>(node));

    case ASTNodeType::REPEAT_LOOP:
      return visit(static_cast<const RepeatLoopNode*>(node));

    case ASTNodeType::TRY:
      return visit(static_cast<const TryNode*>(node));

    case ASTNodeType::LAMBDA:
      return visit(static_cast<const LambdaNode*>(node));

    case ASTNodeType::FUNCTION_DECLARATION:
      return visit(static_cast<const FunctionDeclarationNode*>(node));

    case ASTNodeType::FUNCTION_CALL:
      return visit(static_cast<const FunctionCallNode*>(node));

    case ASTNodeType::METHOD_CALL:
      return visit(static_cast<const MethodCallNode*>(node));

    case ASTNodeType::RETURN_STATEMENT:
      return visit(static_cast<const ReturnNode*>(node));

    case ASTNodeType::INDEX_EXPRESSION:
      return visit(static_cast<const IndexingNode*>(node));

    case ASTNodeType::SLICE_EXPRESSION:
      return visit(static_cast<const SliceNode*>(node));

    case ASTNodeType::NO_OP:
      break;

    default:
      node->print();
      break;
  }

  return static_cast<k_int>(0);
}

std::shared_ptr<CallStackFrame> KInterpreter::createFrame(
    bool isMethodInvocation = false) {
  std::shared_ptr<CallStackFrame> frame = callStack.top();
  auto subFrame = std::make_shared<CallStackFrame>();
  auto& subFrameVariables = subFrame->variables;
  auto& subFrameLambdas = subFrame->lambdas;

  if (!isMethodInvocation) {
    const auto& frameVariables = frame->variables;
    const auto& frameLambdas = frame->lambdas;
    for (const auto& pair : frameVariables) {
      subFrameVariables[pair.first] = pair.second;
    }
    for (const auto& pair : frameLambdas) {
      subFrameLambdas[pair.first] = pair.second;
    }
  }

  if (frame->inObjectContext()) {
    const auto& objectContext = frame->getObjectContext();
    subFrame->setObjectContext(objectContext);
  }

  if (frame->isFlagSet(FrameFlags::InTry)) {
    subFrame->setFlag(FrameFlags::InTry);
  }

  if (frame->isFlagSet(FrameFlags::SubFrame)) {
    subFrame->setFlag(FrameFlags::SubFrame);
  }

  return subFrame;
}

k_value KInterpreter::dropFrame() {
  auto frame = callStack.top();
  auto returnValue = std::move(frame->returnValue);
  auto topVariables = std::move(frame->variables);
  callStack.pop();
  auto callerFrame = callStack.top();

  callerFrame->returnValue = returnValue;

  if (callerFrame->isFlagSet(FrameFlags::SubFrame)) {
    callerFrame->setFlag(FrameFlags::ReturnFlag);
  }

  InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
  return static_cast<k_int>(0);
}

k_string KInterpreter::id(const ASTNode* node) {
  return static_cast<const IdentifierNode*>(node)->name;
}

k_value KInterpreter::visit(const ProgramNode* node) {
  auto programFrame = std::make_shared<CallStackFrame>();
  callStack.push(programFrame);

  for (const auto& stmt : node->statements) {
    interpret(stmt.get());
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ExitNode* node) {
  auto exitValue = interpret(node->exitValue.get());
  auto exitCode = static_cast<k_int>(0);

  if (std::holds_alternative<k_int>(exitValue)) {
    exitCode = std::get<k_int>(exitValue);
  } else {
    exitCode = static_cast<k_int>(1);
  }

  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    exit(static_cast<int>(exitCode));
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ReturnNode* node) {
  auto frame = callStack.top();
  k_value returnValue = static_cast<k_int>(0);

  if (node->returnValue) {
    returnValue = interpret(node->returnValue.get());
  }

  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    frame->setFlag(FrameFlags::ReturnFlag);
    frame->returnValue = returnValue;
    return returnValue;
  }

  return returnValue;
}

k_value KInterpreter::visit(const ThrowNode* node) {
  k_string errorType = "KiwiError";
  k_string errorMessage;

  if (node->errorValue) {
    auto errorValue = interpret(node->errorValue.get());

    if (std::holds_alternative<k_hash>(errorValue)) {
      auto errorHash = std::get<k_hash>(errorValue);
      if (errorHash->hasKey("error") &&
          std::holds_alternative<k_string>(errorHash->kvp["error"])) {
        errorType = std::get<k_string>(errorHash->kvp["error"]);
      }
      if (errorHash->hasKey("message") &&
          std::holds_alternative<k_string>(errorHash->kvp["message"])) {
        errorMessage = std::get<k_string>(errorHash->kvp["message"]);
      }
    } else if (std::holds_alternative<k_string>(errorValue)) {
      errorMessage = std::get<k_string>(errorValue);
    }
  }

  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    throw KiwiError(node->token, errorType, errorMessage);
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const PackageNode* node) {
  auto packageName = id(node->packageName.get());
  packages[packageName] = std::make_shared<KPackage>(node);

  return static_cast<k_int>(0);
}

void KInterpreter::importPackage(const k_value& packageName,
                                 const Token& token) {
  if (!std::holds_alternative<k_string>(packageName)) {
    throw InvalidOperationError(token,
                                "Expected the name of a package to import.");
  }

  auto packageNameValue = std::get<k_string>(packageName);

  if (packages.find(packageNameValue) == packages.end()) {
    throw PackageUndefinedError(token, packageNameValue);
  }

  packageStack.push(packageNameValue);
  auto package = packages[packageNameValue];
  const auto& decl = *package->decl;

  for (const auto& stmt : decl.body) {
    interpret(stmt.get());
  }

  packageStack.pop();
}

k_value KInterpreter::visit(const ExportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ImportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);
  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const MemberAssignmentNode* node) {
  auto frame = callStack.top();
  auto object = interpret(node->object.get());
  auto memberName = node->memberName;
  auto op = node->op;
  auto initializer = interpret(node->initializer.get());

  if (std::holds_alternative<k_hash>(object)) {
    auto hash = std::get<k_hash>(object);

    if (op == KName::Ops_Assign) {
      hash->add(memberName, initializer);
    } else if (hash->hasKey(memberName)) {
      auto value = hash->get(memberName);
      auto newValue =
          MathImpl.do_binary_op(node->token, op, value, initializer);
      hash->add(memberName, newValue);
    } else {
      throw HashKeyError(node->token, memberName);
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const AssignmentNode* node) {
  auto frame = callStack.top();
  auto value = interpret(node->initializer.get());
  auto type = node->op;
  auto name = node->name;

  if (type == KName::Ops_Assign) {
    if (name == Keywords.Global) {
      throw IllegalNameError(node->token, name);
    }

    if (std::holds_alternative<k_lambda>(value)) {
      // WIP: need to work on this.
      const auto& lambdaId = std::get<k_lambda>(value)->identifier;
      lambdas[name] = lambdas[lambdaId];
    } else {
      frame->variables[name] = value;
    }
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

  return frame->variables[name];
}

SliceIndex KInterpreter::getSlice(const SliceNode* node, k_value object) {
  SliceIndex slice;
  slice.isSlice = true;

  slice.indexOrStart = static_cast<k_int>(0);

  if (std::holds_alternative<k_list>(object)) {
    slice.stopIndex =
        static_cast<k_int>(std::get<k_list>(object)->elements.size());
  } else if (std::holds_alternative<k_string>(object)) {
    slice.stopIndex = static_cast<k_int>(std::get<k_string>(object).size());
  }

  slice.stepValue = static_cast<k_int>(1);

  if (node->startExpression) {
    slice.indexOrStart = interpret(node->startExpression.get());
  }

  if (node->stopExpression) {
    slice.stopIndex = interpret(node->stopExpression.get());
  }

  if (node->stepExpression) {
    slice.stepValue = interpret(node->stepExpression.get());
  }

  return slice;
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

k_value KInterpreter::visit(const TernaryOperationNode* node) {
  auto eval = interpret(node->evalExpression.get());

  if (MathImpl.is_truthy(eval)) {
    return interpret(node->trueExpression.get());
  }

  return interpret(node->falseExpression.get());
}

k_value KInterpreter::visit(const SliceNode* node) {
  if (!node->slicedObject) {
    throw InvalidOperationError(node->token, "Nothing to slice.");
  }

  auto object = interpret(node->slicedObject.get());
  auto slice = getSlice(node, object);

  if (std::holds_alternative<k_string>(object)) {
    return InterpHelper::stringSlice(node->token, slice, object);
  } else if (std::holds_alternative<k_list>(object)) {
    return InterpHelper::listSlice(node->token, slice, object);
  }

  throw InvalidOperationError(node->token,
                              "You can only slice lists and strings.");
}

k_value KInterpreter::visit(const IndexingNode* node) {
  if (!node->indexedObject) {
    throw InvalidOperationError(node->token, "Nothing to index.");
  }

  auto object = interpret(node->indexedObject.get());
  auto indexValue = interpret(node->indexExpression.get());

  if (std::holds_alternative<k_list>(object)) {
    auto index = get_integer(node->token, indexValue);
    auto list = std::get<k_list>(object);

    if (index < 0 || static_cast<size_t>(index) > list->elements.size()) {
      throw RangeError(node->token,
                       "The index was outside the bounds of the list.");
    }

    return list->elements.at(index);
  } else if (std::holds_alternative<k_hash>(object)) {
    auto key = get_string(node->token, indexValue);
    auto hash = std::get<k_hash>(object);

    if (!hash->hasKey(key)) {
      throw HashKeyError(node->token,
                         "The key '" + key + "' was not found in the hash.");
    }

    return hash->get(key);
  } else if (std::holds_alternative<k_string>(object)) {
    auto string = std::get<k_string>(object);
    auto index = get_integer(node->token, indexValue);

    if (index < 0 || static_cast<size_t>(index) > string.size()) {
      throw RangeError(node->token,
                       "The index was outside the bounds of the string.");
    }

    return k_string(1, string.at(index));
  }

  throw IndexError(node->token, "Invalid indexing operation.");
}

k_value KInterpreter::visit(const IfNode* node) {
  auto conditionValue = interpret(node->condition.get());
  auto frame = callStack.top();

  if (MathImpl.is_truthy(conditionValue)) {
    for (const auto& stmt : node->body) {
      interpret(stmt.get());
      if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
        break;
      }
    }
  } else {
    bool executed = false;
    for (const auto& elseifNode : node->elseifNodes) {
      auto elseifConditionValue = interpret(elseifNode->condition.get());
      if (MathImpl.is_truthy(elseifConditionValue)) {
        for (const auto& stmt : elseifNode->body) {
          interpret(stmt.get());
          if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
            break;
          }
        }
        executed = true;
        break;
      }
    }

    if (!executed && !node->elseBody.empty()) {
      for (const auto& stmt : node->elseBody) {
        interpret(stmt.get());
        if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
          break;
        }
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

  if (!std::holds_alternative<std::shared_ptr<List>>(dataSetValue)) {
    throw InvalidOperationError(node->token,
                                "Expected a list value in for-loop.");
  }

  auto frame = callStack.top();
  auto list = std::get<std::shared_ptr<List>>(dataSetValue);
  k_string valueIteratorName;
  k_string indexIteratorName;
  bool hasIndexIterator = false;

  valueIteratorName = id(node->valueIterator.get());

  if (node->indexIterator) {
    indexIteratorName = id(node->indexIterator.get());
    hasIndexIterator = true;
  }

  const auto& elements = list->elements;
  bool fallOut = false;

  for (size_t i = 0; i < elements.size(); ++i) {
    if (fallOut) {
      break;
    }

    frame->variables[valueIteratorName] = elements.at(i);

    if (hasIndexIterator) {
      frame->variables[indexIteratorName] = static_cast<k_int>(i);
    }

    for (const auto& stmt : node->body) {
      k_value result = interpret(stmt.get());

      if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
        break;
      }

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
          fallOut = true;
          break;
        }
      }
    }
  }

  frame->variables.erase(valueIteratorName);
  if (hasIndexIterator) {
    frame->variables.erase(indexIteratorName);
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const WhileLoopNode* node) {
  while (MathImpl.is_truthy(interpret(node->condition.get()))) {
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

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const RepeatLoopNode* node) {
  k_value countValue = interpret(node->count.get());
  if (!std::holds_alternative<k_int>(countValue)) {
    throw InvalidOperationError(node->token,
                                "Repeat loop count must be an integer.");
  }
  k_int count = std::get<k_int>(countValue);
  k_string aliasName;
  bool hasAlias = false;

  if (node->alias) {
    aliasName = id(node->alias.get());
    hasAlias = true;
  }

  auto frame = callStack.top();
  auto fallOut = false;
  for (k_int i = 1; i <= count; ++i) {
    if (fallOut) {
      break;
    }

    if (hasAlias) {
      frame->variables[aliasName] = i;
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
          fallOut = true;
          break;
        }
      }
    }
  }

  if (hasAlias && frame->hasVariable(aliasName)) {
    frame->variables.erase(aliasName);
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const TryNode* node) {
  auto frame = callStack.top();

  try {
    for (const auto& stmt : node->tryBody) {
      interpret(stmt.get());
    }
  } catch (const KiwiError& e) {
    if (!node->catchBody.empty()) {
      k_string errorTypeName;
      k_string errorMessageName;
      if (node->errorType) {
        errorTypeName = id(node->errorType.get());
        frame->variables[errorTypeName] = e.getError();
      }

      if (node->errorMessage) {
        errorMessageName = id(node->errorMessage.get());
        frame->variables[errorMessageName] = e.getMessage();
      }

      for (const auto& stmt : node->catchBody) {
        interpret(stmt.get());
      }

      if (node->errorType) {
        frame->variables.erase(errorTypeName);
      }

      if (node->errorMessage) {
        frame->variables.erase(errorMessageName);
      }
    }
  }

  if (!node->finallyBody.empty()) {
    for (const auto& stmt : node->finallyBody) {
      interpret(stmt.get());
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const LambdaNode* node) {
  std::vector<std::pair<std::string, k_value>> parameters;
  std::unordered_set<std::string> defaultParameters;
  auto tmpId = InterpHelper::getTemporaryId();

  parameters.reserve(node->parameters.size());
  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    k_value paramValue = static_cast<k_int>(0);
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }

    std::pair<std::string, k_value> param(paramName, paramValue);
    parameters.emplace_back(param);
  }

  auto lambda = std::make_shared<KLambda>(node);
  lambda->parameters = parameters;
  lambda->defaultParameters = defaultParameters;
  lambdas[tmpId] = lambda;

  return std::make_shared<LambdaRef>(tmpId);
}

k_value KInterpreter::visit(const FunctionDeclarationNode* node) {
  auto name = node->name;

  if (!packageStack.empty()) {
    name = packageStack.top() + "::" + name;
  }

  std::vector<std::pair<std::string, k_value>> parameters;
  std::unordered_set<std::string> defaultParameters;

  parameters.reserve(node->parameters.size());

  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    k_value paramValue = static_cast<k_int>(0);
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }

    std::pair<std::string, k_value> param(paramName, paramValue);
    parameters.emplace_back(param);
  }

  auto function = std::make_shared<KFunction>(node);
  function->name = name;
  function->parameters = parameters;
  function->defaultParameters = defaultParameters;
  functions[name] = std::move(function);

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const FunctionCallNode* node) {
  auto function = functions[node->functionName];

  // TODO: refactoring
  if (function) {
    auto defaultParameters = function->defaultParameters;
    auto functionFrame = createFrame();
    std::vector<k_string> lambdaNames;

    for (size_t i = 0; i < function->parameters.size(); ++i) {
      const auto& param = function->parameters[i];
      k_value argValue = static_cast<k_int>(0);
      if (i < node->arguments.size()) {
        const auto& arg = node->arguments[i];
        argValue = interpret(arg.get());
      } else if (defaultParameters.find(param.first) !=
                 defaultParameters.end()) {
        // Use the default value.
        argValue = param.second;
      } else {
        throw ParameterCountMismatchError(node->token, node->functionName);
      }

      if (std::holds_alternative<k_lambda>(argValue)) {
        auto lambdaId = std::get<k_lambda>(argValue)->identifier;
        lambdas[param.first] = lambdas[lambdaId];
        lambdaNames.push_back(lambdaId);
      } else {
        functionFrame->variables[param.first] = argValue;
      }
    }

    callStack.push(functionFrame);

    k_value result;
    const auto& decl = *function->decl;
    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());
      if (functionFrame->isFlagSet(FrameFlags::ReturnFlag)) {
        result = functionFrame->returnValue;
        break;
      }
    }

    dropFrame();

    for (const auto& lambdaName : lambdaNames) {
      lambdas.erase(lambdaName);
    }

    return result;
  } else if (KiwiBuiltins.is_builtin_method(node->op)) {
    std::vector<k_value> args;
    args.reserve(node->arguments.size());
    for (const auto& arg : node->arguments) {
      args.emplace_back(interpret(arg.get()));
    }

    return BuiltinDispatch::execute(node->token, node->op, args, kiwiArgs);
  } else if (lambdas.find(node->functionName) != lambdas.end()) {
    auto lambda = lambdas[node->functionName];
    auto defaultParameters = lambda->defaultParameters;
    auto lambdaFrame = createFrame();

    for (size_t i = 0; i < lambda->parameters.size(); ++i) {
      const auto& param = lambda->parameters[i];
      k_value argValue = static_cast<k_int>(0);
      if (i < node->arguments.size()) {
        const auto& arg = node->arguments[i];
        argValue = interpret(arg.get());
      } else if (defaultParameters.find(param.first) !=
                 defaultParameters.end()) {
        // Use the default value.
        argValue = param.second;
      } else {
        throw ParameterCountMismatchError(node->token, node->functionName);
      }

      lambdaFrame->variables[param.first] = argValue;
    }

    callStack.push(lambdaFrame);

    k_value result;
    const auto& decl = *lambda->decl;
    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());
    }

    dropFrame();

    return result;
  }

  throw FunctionUndefinedError(node->token, node->functionName);
}

k_value KInterpreter::visit(const MethodCallNode* node) {
  auto op = node->op;
  auto object = interpret(node->object.get());
  std::vector<k_value> arguments;
  arguments.reserve(node->arguments.size());

  for (const auto& arg : node->arguments) {
    arguments.emplace_back(interpret(arg.get()));
  }

  if (ListBuiltins.is_builtin(op)) {
    return interpretListBuiltin(node->token, object, op, arguments);
  } else if (KiwiBuiltins.is_builtin(op)) {
    return BuiltinDispatch::execute(node->token, op, object, arguments);
  }

  throw UnknownBuiltinError(node->token, node->methodName);
}

k_value KInterpreter::interpretListBuiltin(const Token& token, k_value& object,
                                           const KName& op,
                                           std::vector<k_value> arguments) {
  if (!std::holds_alternative<k_list>(object)) {
    throw InvalidOperationError(
        token, "Expected a list for specialized list builtin.");
  }

  auto list = std::get<k_list>(object);

  switch (op) {
    case KName::Builtin_List_Max:
      return listMax(token, list);

    case KName::Builtin_List_Min:
      return listMin(token, list);

    case KName::Builtin_List_Sort:
      return listSort(list);

    case KName::Builtin_List_Sum:
      return listSum(list);

    default:
      break;
  }

  if (arguments.size() == 1) {
    auto arg = arguments.at(0);
    if (!std::holds_alternative<k_lambda>(arg)) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }
    auto lambdaRef = std::get<k_lambda>(arg);

    if (lambdas.find(lambdaRef->identifier) == lambdas.end()) {
      throw InvalidOperationError(
          token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
    }

    auto lambda = lambdas[lambdaRef->identifier];

    switch (op) {
      case KName::Builtin_List_Each:
        return lambdaEach(lambda, list);

      case KName::Builtin_List_Map:
        return lambdaMap(lambda, list);

      case KName::Builtin_List_None:
        return lambdaNone(lambda, list);

      case KName::Builtin_List_Select:
        return lambdaSelect(lambda, list);

      default:
        break;
    }
  } else if (arguments.size() == 2 && op == KName::Builtin_List_Reduce) {
    auto arg = arguments.at(1);

    if (!std::holds_alternative<k_lambda>(arg)) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }
    auto lambdaRef = std::get<k_lambda>(arg);

    if (lambdas.find(lambdaRef->identifier) == lambdas.end()) {
      throw InvalidOperationError(
          token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
    }

    auto lambda = lambdas[lambdaRef->identifier];

    return lambdaReduce(lambda, arguments.at(0), list);
  }

  throw InvalidOperationError(token,
                              "Invalid specialized list builtin invocation.");
}

k_value KInterpreter::listSum(const k_list& list) {
  return sum_listvalue(list);
}

k_value KInterpreter::listMin(const Token& token, const k_list& list) {
  if (list->elements.empty()) {
    throw EmptyListError(token);
  }

  return min_listvalue(list);
}

k_value KInterpreter::listMax(const Token& token, const k_list& list) {
  if (list->elements.empty()) {
    throw EmptyListError(token);
  }

  return max_listvalue(list);
}

k_value KInterpreter::listSort(const k_list& list) {
  sort_list(*list);
  return list;
}

k_value KInterpreter::lambdaEach(std::shared_ptr<KLambda> lambda,
                                 const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string valueVariable;
  k_string indexVariable;
  bool hasIndexVariable = false;

  if (lambda->parameters.empty()) {
    return static_cast<k_int>(0);
  }

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      valueVariable = param.first;
      frame->variables[valueVariable] = static_cast<k_int>(0);
    } else if (i == 1) {
      indexVariable = param.first;
      hasIndexVariable = true;
      frame->variables[indexVariable] = static_cast<k_int>(0);
    }
  }

  k_value result;
  const auto& decl = *lambda->decl;
  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      frame->variables[indexVariable] = static_cast<k_int>(i);
    }

    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());
    }
  }

  frame->variables.erase(valueVariable);
  if (hasIndexVariable) {
    frame->variables.erase(indexVariable);
  }

  return result;
}

k_value KInterpreter::lambdaNone(std::shared_ptr<KLambda> lambda,
                                 const k_list& list) {
  auto selected = lambdaSelect(lambda, list);
  if (std::holds_alternative<k_list>(selected)) {
    return std::get<k_list>(selected)->elements.empty();
  }
  return false;
}

k_value KInterpreter::lambdaMap(std::shared_ptr<KLambda> lambda,
                                const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string mapVariable;

  if (lambda->parameters.empty()) {
    return list;
  }

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      mapVariable = param.first;
      frame->variables[mapVariable] = static_cast<k_int>(0);
    }
  }

  const auto& decl = *lambda->decl;
  const auto& elements = list->elements;
  std::vector<k_value> resultList;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[mapVariable] = elements.at(i);

    for (const auto& stmt : decl.body) {
      resultList.emplace_back(interpret(stmt.get()));
    }
  }

  frame->variables.erase(mapVariable);

  return std::make_shared<List>(resultList);
}

k_value KInterpreter::lambdaReduce(std::shared_ptr<KLambda> lambda,
                                   k_value accumulator, const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string accumVariable;
  k_string valueVariable;

  if (lambda->parameters.size() != 2) {
    return accumulator;
  }

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      accumVariable = param.first;
      frame->variables[accumVariable] = accumulator;
    } else if (i == 1) {
      valueVariable = param.first;
      frame->variables[valueVariable] = static_cast<k_int>(0);
    }
  }

  const auto& elements = list->elements;
  const auto& decl = *lambda->decl;
  k_value result;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());
    }
  }

  result = frame->variables[accumVariable];

  frame->variables.erase(accumVariable);
  frame->variables.erase(valueVariable);

  return result;
}

k_value KInterpreter::lambdaSelect(std::shared_ptr<KLambda> lambda,
                                   const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string valueVariable;
  k_string indexVariable;
  bool hasIndexVariable = false;

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      valueVariable = param.first;
      frame->variables[valueVariable] = static_cast<k_int>(0);
    } else if (i == 1) {
      indexVariable = param.first;
      hasIndexVariable = true;
      frame->variables[indexVariable] = static_cast<k_int>(0);
    }
  }

  k_value result;
  const auto& decl = *lambda->decl;
  const auto& elements = list->elements;
  std::vector<k_value> resultList;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      frame->variables[indexVariable] = static_cast<k_int>(i);
    }

    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());

      if (MathImpl.is_truthy(result)) {
        resultList.emplace_back(elements.at(i));
      }
    }
  }

  frame->variables.erase(valueVariable);
  if (hasIndexVariable) {
    frame->variables.erase(indexVariable);
  }

  return std::make_shared<List>(resultList);
}

#endif
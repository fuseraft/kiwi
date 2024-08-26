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
#include "typing/value.h"

class Function {
 public:
  std::string name;
  std::vector<std::pair<std::string, k_value>> parameters;
  const FunctionDeclarationNode* decl;
  std::unordered_set<std::string> defaultParameters;

  Function(const FunctionDeclarationNode* decl) : decl(decl) {}
};

class KInterpreter {
 public:
  KInterpreter() {}

  std::shared_ptr<CallStackFrame> createFrame(bool isMethodInvocation);
  k_value dropFrame();
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
  k_value visit(const WhileLoopNode* node);
  k_value visit(const RepeatLoopNode* node);
  k_value visit(const TryNode* node);
  k_value visit(const FunctionDeclarationNode* node);
  k_value visit(const FunctionCallNode* node);
  k_value visit(const MethodCallNode* node);

 private:
  std::unordered_map<std::string, std::shared_ptr<Function>> functions;
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

    case ASTNodeType::FUNCTION_DECLARATION:
      return visit(static_cast<const FunctionDeclarationNode*>(node));

    case ASTNodeType::FUNCTION_CALL:
      return visit(static_cast<const FunctionCallNode*>(node));

    case ASTNodeType::METHOD_CALL:
      return visit(static_cast<const MethodCallNode*>(node));

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

k_value KInterpreter::visit(const ProgramNode* node) {
  auto programFrame = std::make_shared<CallStackFrame>();
  callStack.push(programFrame);

  for (const auto& stmt : node->statements) {
    interpret(stmt.get());
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
    auto ifFrame = createFrame();
    callStack.push(ifFrame);
    for (const auto& stmt : node->body) {
      interpret(stmt.get());
    }
    dropFrame();
  } else {
    bool executed = false;
    for (const auto& elseifNode : node->elseifNodes) {
      auto elseifConditionValue = interpret(elseifNode->condition.get());
      if (MathImpl.is_truthy(elseifConditionValue)) {
        auto elseifFrame = createFrame();
        callStack.push(elseifFrame);
        for (const auto& stmt : elseifNode->body) {
          interpret(stmt.get());
        }
        dropFrame();
        executed = true;
        break;
      }
    }

    if (!executed && !node->elseBody.empty()) {
      auto elseFrame = createFrame();
      callStack.push(elseFrame);
      for (const auto& stmt : node->elseBody) {
        interpret(stmt.get());
      }
      dropFrame();
    }
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const CaseNode* node) {
  k_value testValue = interpret(node->testValue.get());

  for (const auto& whenNode : node->whenNodes) {
    k_value whenCondition = interpret(whenNode->condition.get());

    if (std::get<bool>(MathImpl.do_eq_comparison(testValue, whenCondition))) {
      auto caseFrame = createFrame();
      callStack.push(caseFrame);

      for (const auto& stmt : whenNode->body) {
        interpret(stmt.get());
      }

      return dropFrame();
    }
  }

  if (!node->elseBody.empty()) {
    auto elseFrame = createFrame();
    for (const auto& stmt : node->elseBody) {
      interpret(stmt.get());
    }
    dropFrame();
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const ForLoopNode* node) {
  k_value dataSetValue = interpret(node->dataSet.get());

  if (!std::holds_alternative<std::shared_ptr<List>>(dataSetValue)) {
    throw InvalidOperationError(node->token,
                                "Expected a list value in for-loop.");
  }

  auto list = std::get<std::shared_ptr<List>>(dataSetValue);
  k_string valueIteratorName;
  k_string indexIteratorName;
  bool hasIndexIterator = false;

  valueIteratorName =
      static_cast<const IdentifierNode*>(node->valueIterator.get())->name;

  if (node->indexIterator) {
    indexIteratorName =
        static_cast<const IdentifierNode*>(node->indexIterator.get())->name;
    hasIndexIterator = true;
  }

  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    auto frame = createFrame();
    frame->variables[valueIteratorName] = elements.at(i);

    if (hasIndexIterator) {
      frame->variables[indexIteratorName] = static_cast<k_int>(i);
    }
    callStack.push(frame);

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
          return dropFrame();
        }
      }
    }

    dropFrame();
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const WhileLoopNode* node) {
  createFrame();

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
          return dropFrame();
        }
      }
    }
  }

  return dropFrame();
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
    aliasName = static_cast<const IdentifierNode*>(node->alias.get())->name;
    hasAlias = true;
  }

  auto frame = createFrame();
  for (k_int i = 1; i <= count; ++i) {

    if (hasAlias) {
      frame->variables[aliasName] = i;
    }

    callStack.push(frame);

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
          return dropFrame();
        }
      }
    }
  }

  dropFrame();

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const TryNode* node) {
  auto tryFrame = createFrame();
  callStack.push(tryFrame);

  try {
    for (const auto& stmt : node->tryBody) {
      interpret(stmt.get());
    }
  } catch (const KiwiError& e) {
    if (!node->catchBody.empty()) {
      auto catchFrame = createFrame();

      if (node->errorType) {
        auto errorTypeName =
            static_cast<const IdentifierNode*>(node->errorType.get())->name;
        catchFrame->variables[errorTypeName] = e.getError();
      }

      if (node->errorMessage) {
        auto errorMessageName =
            static_cast<const IdentifierNode*>(node->errorMessage.get())->name;
        catchFrame->variables[errorMessageName] = e.getMessage();
      }

      callStack.push(catchFrame);

      for (const auto& stmt : node->catchBody) {
        interpret(stmt.get());
      }

      dropFrame();
    }
  }

  dropFrame();

  if (!node->finallyBody.empty()) {
    auto finallyFrame = createFrame();
    callStack.push(finallyFrame);
    for (const auto& stmt : node->finallyBody) {
      interpret(stmt.get());
    }
    dropFrame();
  }

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const FunctionDeclarationNode* node) {
  auto name = node->name;
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

  auto function = std::make_shared<Function>(node);

  function->name = name;
  function->parameters = parameters;
  function->defaultParameters = defaultParameters;
  functions[node->name] = std::move(function);

  return static_cast<k_int>(0);
}

k_value KInterpreter::visit(const FunctionCallNode* node) {
  auto function = functions[node->functionName];

  // TODO: refactoring
  if (function) {
    auto defaultParameters = function->defaultParameters;
    auto functionFrame = createFrame();

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

      functionFrame->variables[param.first] = argValue;
    }

    callStack.push(functionFrame);

    k_value result;
    const auto& decl = *function->decl;
    for (const auto& stmt : decl.body) {
      result = interpret(stmt.get());
    }

    dropFrame();

    return result;
  } else if (KiwiBuiltins.is_builtin_method(node->op)) {
    std::vector<k_value> args;
    args.reserve(node->arguments.size());
    for (const auto& arg : node->arguments) {
      args.emplace_back(interpret(arg.get()));
    }

    return BuiltinDispatch::execute(node->token, node->op, args, kiwiArgs);
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

  if (KiwiBuiltins.is_builtin(op)) {
    return BuiltinDispatch::execute(node->token, op, object, arguments);
  }

  throw UnknownBuiltinError(node->token, node->methodName);
}

#endif
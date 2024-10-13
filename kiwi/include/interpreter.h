#ifndef KIWI_INTERPRETER_H
#define KIWI_INTERPRETER_H

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "globals.h"
#include "builtin.h"
#include "interp_helper.h"
#include "math/functions.h"
#include "parsing/ast.h"
#include "parsing/builtins.h"
#include "tracing/error.h"
#include "typing/value.h"
#include "util/file.h"

std::unordered_map<k_string, std::unique_ptr<KPackage>> packages;
std::unordered_map<k_string, std::unique_ptr<KFunction>> functions;
std::unordered_map<k_string, std::unique_ptr<KFunction>> methods;
std::unordered_map<k_string, std::unique_ptr<KLambda>> lambdas;
std::unordered_map<k_string, std::unique_ptr<KClass>> classes;
httplib::Server server;
std::unordered_map<int, k_string> serverHooks;

std::unordered_map<k_string, k_string> lambdaTable;

std::mutex interpreterMutex;

class KInterpreter {
 public:
  KInterpreter() {}

  k_value interpret(const ASTNode* node);

 private:
  std::stack<k_string> classStack;

  std::shared_ptr<CallStackFrame> createFrame(bool isMethodInvocation);
  k_value dropFrame();
  void importPackage(const k_value& packageName, const Token& token);
  void importExternal(const k_string& packageName);

  k_string id(const ASTNode* node);
  k_value visit(const ProgramNode* node);
  k_value visit(const SelfNode* node);
  k_value visit(const PackAssignmentNode* node);
  k_value visit(const AssignmentNode* node);
  k_value visit(const IndexAssignmentNode* node);
  k_value visit(const MemberAssignmentNode* node);
  k_value visit(const MemberAccessNode* node);
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
  k_value visit(const BreakNode* node);
  k_value visit(const NextNode* node);
  k_value visit(const TryNode* node);
  k_value visit(const LambdaNode* node);
  k_value visit(const LambdaCallNode* node);
  k_value visit(const ClassNode* node);
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

  k_value callBuiltinMethod(const FunctionCallNode* node);
  k_value callClassMethod(const MethodCallNode* node, const k_class& clazz);
  k_value callObjectBaseMethod(const MethodCallNode* node,
                               const std::shared_ptr<Object>& obj,
                               const k_string& baseClass,
                               const k_string& methodName);
  k_value callObjectMethod(const MethodCallNode* node,
                           const std::shared_ptr<Object>& obj);
  k_value executeFunctionBody(const std::unique_ptr<KFunction>& function);
  k_value callLambda(std::shared_ptr<CallStackFrame>& lambdaFrame,
                     const Token& token, const k_string& lambdaName,
                     const std::vector<std::unique_ptr<ASTNode>>& arguments);
  k_value callFunction(const std::unique_ptr<KFunction>& function,
                       const std::vector<std::unique_ptr<ASTNode>>& arguments,
                       const Token& token, const std::string& functionName);
  KCallableType getCallable(const Token& token, const std::string& name) const;
  std::unique_ptr<KFunction> createFunction(const FunctionDeclarationNode* node,
                                            const std::string& name);
  k_value doSliceAssignment(const Token& token, k_value& slicedObj,
                            const SliceIndex& slice, k_value& newValue);
  k_value handleNestedIndexing(const IndexingNode* indexExpr, k_value baseObj,
                               const KName& op, const k_value& newValue);
  SliceIndex getSlice(const SliceNode* node, k_value object);
  std::vector<k_value> getMethodCallArguments(
      const std::vector<std::unique_ptr<ASTNode>>& args);

  k_value listLoop(const ForLoopNode* node, const k_list& list);
  k_value hashLoop(const ForLoopNode* node, const k_hash& hash);
  k_value listSum(const k_list& list);
  k_value listMin(const Token& token, const k_list& list);
  k_value listMax(const Token& token, const k_list& list);
  k_value listSort(const k_list& list);
  k_value lambdaEach(std::unique_ptr<KLambda>& lambda, const k_list& list);
  k_value lambdaNone(std::unique_ptr<KLambda>& lambda, const k_list& list);
  k_value lambdaMap(std::unique_ptr<KLambda>& lambda, const k_list& list);
  k_value lambdaReduce(std::unique_ptr<KLambda>& lambda, k_value accumulator,
                       const k_list& list);
  k_value lambdaSelect(std::unique_ptr<KLambda>& lambda, const k_list& list);
  k_value lambdaAll(std::unique_ptr<KLambda>& lambda, const k_list& list);
  k_value interpretListBuiltin(const Token& token, k_value& object,
                               const KName& op, std::vector<k_value> arguments);

  k_value interpolateString(const Token& token, const k_string& input);
  k_value interpretSerializerDeserialize(const Token& token,
                                         std::vector<k_value>& args);
  k_value interpretSerializerSerialize(const Token& token,
                                       std::vector<k_value>& args);
  k_value interpretSerializerBuiltin(const Token& token, const KName& builtin,
                                     std::vector<k_value>& args);
  k_value interpretReflectorBuiltin(const Token& token, const KName& builtin,
                                    std::vector<k_value>& args);
  k_value interpretWebServerBuiltin(const Token& token, const KName& builtin,
                                    std::vector<k_value>& args);
  k_value interpretWebServerGet(const Token& token, std::vector<k_value>& args);
  k_value interpretWebServerPost(const Token& token,
                                 std::vector<k_value>& args);
  k_value interpretWebServerListen(const Token& token,
                                   std::vector<k_value>& args);
  k_value interpretWebServerPublic(const Token& token,
                                   std::vector<k_value>& args);
  int getNextWebServerHook(const Token& token, k_value& arg);
  void handleWebServerRequest(int webhookID, k_hash requestHash,
                              k_string& redirect, k_string& content,
                              k_string& contentType, int& status);
};

k_value KInterpreter::interpret(const ASTNode* node) {
  if (!node) {
    return {};
  }

  switch (node->type) {
    case ASTNodeType::PROGRAM:
      return visit(static_cast<const ProgramNode*>(node));

    case ASTNodeType::SELF:
      return visit(static_cast<const SelfNode*>(node));

    case ASTNodeType::PACKAGE:
      return visit(static_cast<const PackageNode*>(node));

    case ASTNodeType::CLASS:
      return visit(static_cast<const ClassNode*>(node));

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

    case ASTNodeType::INDEX_ASSIGNMENT:
      return visit(static_cast<const IndexAssignmentNode*>(node));

    case ASTNodeType::MEMBER_ASSIGNMENT:
      return visit(static_cast<const MemberAssignmentNode*>(node));

    case ASTNodeType::PACK_ASSIGNMENT:
      return visit(static_cast<const PackAssignmentNode*>(node));

    case ASTNodeType::MEMBER_ACCESS:
      return visit(static_cast<const MemberAccessNode*>(node));

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

    case ASTNodeType::BREAK_STATEMENT:
      return visit(static_cast<const BreakNode*>(node));

    case ASTNodeType::NEXT_STATEMENT:
      return visit(static_cast<const NextNode*>(node));

    case ASTNodeType::TRY:
      return visit(static_cast<const TryNode*>(node));

    case ASTNodeType::LAMBDA:
      return visit(static_cast<const LambdaNode*>(node));

    case ASTNodeType::LAMBDA_CALL:
      return visit(static_cast<const LambdaCallNode*>(node));

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

  return {};
}

std::shared_ptr<CallStackFrame> KInterpreter::createFrame(
    bool isMethodInvocation = false) {
  std::shared_ptr<CallStackFrame> frame = callStack.top();
  auto subFrame = std::make_shared<CallStackFrame>();
  auto& subFrameVariables = subFrame->variables;

  if (!isMethodInvocation) {
    const auto& frameVariables = frame->variables;
    for (const auto& pair : frameVariables) {
      subFrameVariables[pair.first] = pair.second;
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

  if (!callStack.empty()) {
    auto callerFrame = callStack.top();

    callerFrame->returnValue = returnValue;

    if (callerFrame->isFlagSet(FrameFlags::SubFrame)) {
      callerFrame->setFlag(FrameFlags::Return);
    }

    InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
  }

  return {};
}

k_string KInterpreter::id(const ASTNode* node) {
  return static_cast<const IdentifierNode*>(node)->name;
}

k_value KInterpreter::visit(const ProgramNode* node) {
  // This is the program root
  if (!node->isScript) {
    auto programFrame = std::make_shared<CallStackFrame>();
    programFrame->variables[Keywords.Global] = std::make_shared<Hash>();
    callStack.push(programFrame);
  }

  k_value result;

  for (const auto& stmt : node->statements) {
    result = interpret(stmt.get());
  }

  return result;
}

k_value KInterpreter::visit(const ExitNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto exitValue = interpret(node->exitValue.get());
    auto exitCode = static_cast<k_int>(0);

    if (std::holds_alternative<k_int>(exitValue)) {
      exitCode = std::get<k_int>(exitValue);
    } else {
      exitCode = static_cast<k_int>(1);
    }

    exit(static_cast<int>(exitCode));
  }

  return {};
}

k_value KInterpreter::visit(const ReturnNode* node) {
  k_value returnValue = {};

  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    if (node->returnValue) {
      returnValue = interpret(node->returnValue.get());
    }

    auto frame = callStack.top();
    frame->setFlag(FrameFlags::Return);
    frame->returnValue = returnValue;
    return returnValue;
  }

  return returnValue;
}

k_value KInterpreter::visit(const ThrowNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
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

    throw KiwiError(node->token, errorType, errorMessage);
  }

  return {};
}

k_value KInterpreter::visit(const PackageNode* node) {
  auto packageName = id(node->packageName.get());
  packages[packageName] = std::make_unique<KPackage>(node);

  return {};
}

void KInterpreter::importExternal(const k_string& packageName) {
  auto content = File::readFile(packageName);
  if (content.empty()) {
    return;
  }

  Lexer lexer(packageName, content);

  Parser p;
  auto tokenStream = lexer.getTokenStream();
  auto ast = p.parseTokenStream(tokenStream, true);

  interpret(ast.get());

  return;
}

void KInterpreter::importPackage(const k_value& packageName,
                                 const Token& token) {
  if (!std::holds_alternative<k_string>(packageName)) {
    throw InvalidOperationError(token,
                                "Expected the name of a package to import.");
  }

  auto packageNameValue = std::get<k_string>(packageName);

  if (packages.find(packageNameValue) == packages.end()) {
    // Check if external package.
    if (File::isScript(packageNameValue)) {
      return importExternal(packageNameValue);
    }
    throw PackageUndefinedError(token, packageNameValue);
  }

  packageStack.push(packageNameValue);
  const auto& package = packages[packageNameValue];
  const auto& decl = *package->decl;

  for (const auto& stmt : decl.body) {
    interpret(stmt.get());
  }

  packageStack.pop();
}

k_value KInterpreter::visit(const ExportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);

  return {};
}

k_value KInterpreter::visit(const ImportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);
  return {};
}

k_value KInterpreter::visit(const MemberAccessNode* node) {
  auto object = interpret(node->object.get());
  auto memberName = node->memberName;

  if (std::holds_alternative<k_hash>(object)) {
    auto hash = std::get<k_hash>(object);
    if (!hash->hasKey(memberName)) {
      throw HashKeyError(node->token, memberName);
    }

    return hash->get(memberName);
  }

  return {};
}

k_value KInterpreter::doSliceAssignment(const Token& token, k_value& slicedObj,
                                        const SliceIndex& slice,
                                        k_value& newValue) {
  if (std::holds_alternative<k_list>(slicedObj) &&
      std::holds_alternative<k_list>(newValue)) {
    auto targetList = std::get<k_list>(slicedObj);
    auto rhsValues = std::get<k_list>(newValue);
    InterpHelper::updateListSlice(token, false, targetList, slice, rhsValues);

    return targetList;
  }

  return slicedObj;
}

k_value KInterpreter::handleNestedIndexing(const IndexingNode* indexExpr,
                                           k_value baseObj, const KName& op,
                                           const k_value& newValue) {
  if (indexExpr->indexExpression->type == ASTNodeType::INDEX_EXPRESSION) {
    auto nestedIndexExpr =
        static_cast<const IndexingNode*>(indexExpr->indexExpression.get());
    auto nestedIndex = interpret(nestedIndexExpr->indexExpression.get());

    if (std::holds_alternative<k_list>(baseObj) &&
        std::holds_alternative<k_int>(nestedIndex)) {
      auto listObj = std::get<k_list>(baseObj);
      auto indexValue =
          static_cast<int>(get_integer(indexExpr->token, nestedIndex));

      if (indexValue < 0 ||
          static_cast<size_t>(indexValue) >= listObj->elements.size()) {
        throw IndexError(indexExpr->token,
                         "The index was outside the bounds of the list.");
      }

      if (nestedIndexExpr->indexExpression->type ==
          ASTNodeType::INDEX_EXPRESSION) {
        k_value nestedValue = handleNestedIndexing(
            nestedIndexExpr, listObj->elements[indexValue], op, newValue);
        listObj->elements[indexValue] = nestedValue;
      } else {
        if (op == KName::Ops_Assign) {
          listObj->elements[indexValue] = newValue;
        } else {
          auto oldValue = listObj->elements[indexValue];
          listObj->elements[indexValue] =
              MathImpl.do_binary_op(indexExpr->token, op, oldValue, newValue);
        }
      }

      return listObj;
    } else {
      throw IndexError(indexExpr->token,
                       "Nested index does not target a list.");
    }
  } else if (indexExpr->indexExpression->type == ASTNodeType::IDENTIFIER &&
             std::holds_alternative<k_hash>(baseObj)) {
    auto key = id(indexExpr->indexExpression.get());
    auto hashObj = std::get<k_hash>(baseObj);

    if (hashObj->hasKey(key)) {
      auto nestedValue = hashObj->get(key);
      if (op == KName::Ops_Assign) {
        hashObj->add(key, newValue);
      } else {
        auto oldValue = hashObj->get(key);
        hashObj->add(key, MathImpl.do_binary_op(indexExpr->token, op, oldValue,
                                                newValue));
      }
      return hashObj;
    } else {
      throw HashKeyError(indexExpr->token, key);
    }
  } else if (indexExpr->indexExpression->type == ASTNodeType::IDENTIFIER &&
             std::holds_alternative<k_list>(baseObj)) {
    auto identifier = interpret(indexExpr->indexExpression.get());
    auto list = std::get<k_list>(baseObj);
    auto listIndex = get_integer(indexExpr->token, identifier);
    if (listIndex < 0 ||
        listIndex >= static_cast<k_int>(list->elements.size())) {
      throw IndexError(indexExpr->token,
                       "The index was outside the bounds of the list.");
    }

    if (op == KName::Ops_Assign) {
      list->elements[listIndex] = newValue;
    } else {
      auto oldValue = list->elements.at(listIndex);
      list->elements[listIndex] =
          MathImpl.do_binary_op(indexExpr->token, op, oldValue, newValue);
    }
    return list;

  } else if (indexExpr->indexExpression->type == ASTNodeType::LITERAL) {
    auto literal = interpret(indexExpr->indexExpression.get());

    if (std::holds_alternative<k_list>(baseObj) &&
        std::holds_alternative<k_int>(literal)) {
      auto list = std::get<k_list>(baseObj);
      auto listIndex = get_integer(indexExpr->token, literal);
      if (listIndex < 0 ||
          listIndex >= static_cast<k_int>(list->elements.size())) {
        throw IndexError(indexExpr->token,
                         "The index was outside the bounds of the list.");
      }

      if (op == KName::Ops_Assign) {
        list->elements[listIndex] = newValue;
      } else {
        auto oldValue = list->elements.at(listIndex);
        list->elements[listIndex] =
            MathImpl.do_binary_op(indexExpr->token, op, oldValue, newValue);
      }
      return list;
    } else if (std::holds_alternative<k_hash>(baseObj) &&
               std::holds_alternative<k_string>(literal)) {
      auto hash = std::get<k_hash>(baseObj);
      auto key = get_string(indexExpr->token, literal);

      if (op == KName::Ops_Assign) {
        hash->add(key, newValue);
      } else {
        auto oldValue = hash->get(key);
        hash->add(key, MathImpl.do_binary_op(indexExpr->token, op, oldValue,
                                             newValue));
      }
      return hash;
    }
  }

  throw IndexError(indexExpr->token, "Invalid index expression.");
}

k_value KInterpreter::visit(const PackAssignmentNode* node) {
  auto frame = callStack.top();

  std::vector<k_value> rhsValues;
  rhsValues.reserve(node->right.size());
  for (const auto& rhs : node->right) {
    rhsValues.emplace_back(interpret(rhs.get()));
  }

  size_t rhsPosition = 0;
  const size_t lhsLength = node->left.size();

  // unpack
  if (rhsValues.size() == 1) {
    std::vector<k_value> unpacked;
    const auto& rhsValue = rhsValues.at(0);
    if (!std::holds_alternative<k_list>(rhsValue)) {
      throw InvalidOperationError(node->token, "Expected a list to unpack.");
    }

    const auto& rhsElements = std::get<k_list>(rhsValue)->elements;
    unpacked.reserve(rhsElements.size());

    for (const auto& rhs : rhsElements) {
      unpacked.emplace_back(rhs);
    }

    rhsValues.clear();
    rhsValues.reserve(unpacked.size());
    rhsValues = unpacked;
  }

  for (const auto& lhs : node->left) {
    const auto& identifierName = id(lhs.get());
    if (rhsValues.size() == lhsLength) {
      frame->variables[identifierName] = rhsValues[rhsPosition++];
    } else {
      frame->variables[identifierName] = std::make_shared<Null>();
    }
  }

  return {};
}

k_value KInterpreter::visit(const IndexAssignmentNode* node) {
  auto frame = callStack.top();
  auto op = node->op;
  k_string identifierName;
  auto newValue = interpret(node->initializer.get());

  if (node->object->type == ASTNodeType::SLICE_EXPRESSION) {
    auto sliceExpr = static_cast<const SliceNode*>(node->object.get());

    if (sliceExpr->slicedObject->type == ASTNodeType::IDENTIFIER) {
      identifierName = id(sliceExpr->slicedObject.get());
      auto slicedObj = frame->variables[identifierName];
      auto slice = getSlice(sliceExpr, slicedObj);

      doSliceAssignment(node->token, slicedObj, slice, newValue);
      frame->variables[identifierName] = slicedObj;
    }
  } else if (node->object->type == ASTNodeType::INDEX_EXPRESSION) {
    auto indexExpr = static_cast<const IndexingNode*>(node->object.get());

    if (indexExpr->indexedObject->type == ASTNodeType::IDENTIFIER) {
      identifierName = id(indexExpr->indexedObject.get());
      auto indexedObj = frame->variables[identifierName];
      auto index = interpret(indexExpr->indexExpression.get());

      if (std::holds_alternative<k_list>(indexedObj) &&
          std::holds_alternative<k_int>(index)) {
        auto listObj = std::get<k_list>(indexedObj);
        auto indexValue = static_cast<int>(get_integer(node->token, index));

        if (indexValue < 0 ||
            static_cast<size_t>(indexValue) >= listObj->elements.size()) {
          throw IndexError(node->token,
                           "The index was outside the bounds of the list.");
        }

        // Handle nested indexing
        if (indexExpr->indexExpression->type == ASTNodeType::INDEX_EXPRESSION) {
          k_value nestedValue = handleNestedIndexing(
              indexExpr, listObj->elements[indexValue], op, newValue);
          listObj->elements[indexValue] = nestedValue;
        } else {
          if (op == KName::Ops_Assign) {
            listObj->elements[indexValue] = newValue;
          } else {
            auto oldValue = listObj->elements[indexValue];
            listObj->elements[indexValue] =
                MathImpl.do_binary_op(node->token, op, oldValue, newValue);
          }
        }

        frame->variables[identifierName] = listObj;
      } else if (std::holds_alternative<k_hash>(indexedObj) &&
                 std::holds_alternative<k_string>(index)) {
        auto hashObj = std::get<k_hash>(indexedObj);
        auto key = get_string(node->token, index);

        if (op == KName::Ops_Assign) {
          hashObj->add(key, newValue);
        } else {
          if (!hashObj->hasKey(key)) {
            throw HashKeyError(node->token, key);
          }
          auto oldValue = hashObj->get(key);
          hashObj->add(
              key, MathImpl.do_binary_op(node->token, op, oldValue, newValue));
        }
      }
    } else if (indexExpr->indexedObject->type ==
               ASTNodeType::INDEX_EXPRESSION) {
      auto indexedObj =
          static_cast<const IndexingNode*>(indexExpr->indexedObject.get());
      k_string indexedObjId;

      if (indexedObj->indexedObject->type == ASTNodeType::IDENTIFIER) {
        indexedObjId = id(indexedObj->indexedObject.get());
      } else {
        throw IndexError(indexExpr->token,
                         "Invalid nested indexing expression.");
      }

      k_value baseObj = interpret(indexExpr->indexedObject.get());
      handleNestedIndexing(indexExpr, baseObj, op, newValue);
    }
  }

  return {};
}

k_value KInterpreter::visit(const MemberAssignmentNode* node) {
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

  return {};
}

k_value KInterpreter::visit(const AssignmentNode* node) {
  auto frame = callStack.top();
  auto left = interpret(node->left.get());
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
      lambdas[name] = std::move(lambdas[lambdaId]);
      return value;
    } else {
      if (frame->inObjectContext() &&
          (node->left->type == ASTNodeType::SELF || name.at(0) == '@')) {
        auto& obj = frame->getObjectContext();
        obj->instanceVariables[name] = value;
        return obj->instanceVariables[name];
      }

      if (std::holds_alternative<k_object>(value)) {
        auto& obj = std::get<k_object>(value);
        obj->identifier = name;
      }

      frame->variables[name] = value;
    }
  } else {
    if (frame->hasVariable(name)) {
      auto oldValue = frame->variables[name];

      if (type == KName::Ops_BitwiseNotAssign) {
        frame->variables[name] = MathImpl.do_bitwise_not(node->token, oldValue);
      } else {
        frame->variables[name] =
            MathImpl.do_binary_op(node->token, type, oldValue, value);
      }

      return frame->variables[name];
    } else if (frame->inObjectContext()) {
      auto& obj = frame->getObjectContext();

      if (!obj->hasVariable(name)) {
        throw VariableUndefinedError(node->token, name);
      }

      auto oldValue = obj->instanceVariables[name];

      if (type == KName::Ops_BitwiseNotAssign) {
        obj->instanceVariables[name] =
            MathImpl.do_bitwise_not(node->token, oldValue);
      } else {
        obj->instanceVariables[name] =
            MathImpl.do_binary_op(node->token, type, oldValue, value);
      }

      return obj->instanceVariables[name];
    }

    throw VariableUndefinedError(node->token, name);
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

k_value KInterpreter::visit(const SelfNode* node) {
  auto frame = callStack.top();

  if (!frame->inObjectContext()) {
    throw InvalidContextError(node->token);
  }

  if (!node->name.empty()) {
    auto name = node->name;
    auto obj = frame->getObjectContext();
    if (!obj->hasVariable(name)) {
      obj->instanceVariables[name] = {};
    }
    return obj->instanceVariables[name];
  }

  return frame->getObjectContext();
}

k_value KInterpreter::visit(const IdentifierNode* node) {
  auto frame = callStack.top();

  if (frame->inObjectContext() && node->name.at(0) == '@') {
    return frame->getObjectContext()->instanceVariables[node->name];
  }

  if (frame->hasVariable(node->name)) {
    return frame->variables[node->name];
  } else if (classes.find(node->name) != classes.end()) {
    return std::make_shared<ClassRef>(node->name);
  } else if (lambdas.find(node->name) != lambdas.end()) {
    return std::make_shared<LambdaRef>(node->name);
  } else if (lambdaTable.find(node->name) != lambdaTable.end()) {
    const auto& mappedId = lambdaTable[node->name];
    if (lambdas.find(mappedId) != lambdas.end()) {
      return std::make_shared<LambdaRef>(mappedId);
    }
  }

  return std::make_shared<Null>();
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
  std::unordered_map<k_value, k_value> kvps;
  std::vector<k_value> keys;
  keys.reserve(node->elements.size());

  for (const auto& pair : node->elements) {
    auto key = interpret(pair.first.get());
    auto value = interpret(pair.second.get());
    kvps[key] = value;
    keys.emplace_back(key);
  }

  for (const auto& key : keys) {
    hash->add(key, kvps[key]);
  }

  return hash;
}

k_value KInterpreter::visit(const PrintNode* node) {
  if (SILENCE) {
    return {};
  }
  auto value = interpret(node->expression.get());
  if (node->printNewline) {
    std::cout << Serializer::serialize(value) << std::endl;
  } else {
    std::cout << Serializer::serialize(value) << std::flush;
  }

  return {};
}

k_value KInterpreter::visit(const UnaryOperationNode* node) {
  auto right = interpret(node->operand.get());
  return MathImpl.do_unary_op(node->token, node->op, right);
}

k_value KInterpreter::visit(const BinaryOperationNode* node) {
  auto left = interpret(node->left.get());
  auto op = node->op;
  if (op == KName::Ops_And) {
    if (!MathImpl.is_truthy(left)) {
      return false;
    }
  } else if (op == KName::Ops_Or) {
    if (MathImpl.is_truthy(left)) {
      return true;
    }
  }
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

  if (node->indexExpression->type == ASTNodeType::INDEX_EXPRESSION) {
    auto indexExpr =
        static_cast<const IndexingNode*>(node->indexExpression.get());
    return handleNestedIndexing(indexExpr, object, KName::Ops_Assign,
                                k_value{});
  } else {
    if (std::holds_alternative<k_list>(object)) {
      auto index = get_integer(node->token, indexValue);
      auto list = std::get<k_list>(object);

      if (index < 0 || static_cast<size_t>(index) >= list->elements.size()) {
        throw RangeError(node->token,
                         "The index was outside the bounds of the list.");
      }

      return list->elements.at(index);
    } else if (std::holds_alternative<k_hash>(object)) {
      auto key = get_string(node->token, indexValue);
      auto hash = std::get<k_hash>(object);

      if (!hash->hasKey(key)) {
        throw HashKeyError(node->token, key);
      }

      return hash->get(key);
    } else if (std::holds_alternative<k_string>(object)) {
      auto string = std::get<k_string>(object);
      auto index = get_integer(node->token, indexValue);

      if (index < 0 || static_cast<size_t>(index) >= string.size()) {
        throw RangeError(node->token,
                         "The index was outside the bounds of the string.");
      }

      return k_string(1, string.at(index));
    }

    throw IndexError(node->token, "Invalid indexing operation.");
  }
}

k_value KInterpreter::visit(const IfNode* node) {
  auto conditionValue = interpret(node->condition.get());
  auto frame = callStack.top();

  if (MathImpl.is_truthy(conditionValue)) {
    for (const auto& stmt : node->body) {
      interpret(stmt.get());
      if (frame->isFlagSet(FrameFlags::Return)) {
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
          if (frame->isFlagSet(FrameFlags::Return)) {
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
        if (frame->isFlagSet(FrameFlags::Return)) {
          break;
        }
      }
    }
  }

  return {};
}

k_value KInterpreter::visit(const CaseNode* node) {
  k_value testValue = interpret(node->testValue.get());

  for (const auto& whenNode : node->whenNodes) {
    k_value whenCondition = interpret(whenNode->condition.get());

    if (std::get<bool>(MathImpl.do_eq_comparison(testValue, whenCondition))) {
      for (const auto& stmt : whenNode->body) {
        interpret(stmt.get());
      }

      return {};
    }
  }

  if (!node->elseBody.empty()) {
    for (const auto& stmt : node->elseBody) {
      interpret(stmt.get());
    }
  }

  return {};
}

k_value KInterpreter::listLoop(const ForLoopNode* node, const k_list& list) {
  auto frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);
  const auto& elements = list->elements;

  k_string valueIteratorName;
  k_string indexIteratorName;
  bool hasIndexIterator = false;

  valueIteratorName = id(node->valueIterator.get());

  if (node->indexIterator) {
    indexIteratorName = id(node->indexIterator.get());
    hasIndexIterator = true;
  }

  bool fallOut = false;
  k_value result;

  for (size_t i = 0; i < elements.size(); ++i) {
    if (fallOut) {
      break;
    }

    frame->variables[valueIteratorName] = elements.at(i);

    if (hasIndexIterator) {
      frame->variables[indexIteratorName] = static_cast<k_int>(i);
    }

    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT_STATEMENT &&
          stmt->type != ASTNodeType::BREAK_STATEMENT) {
        result = interpret(stmt.get());

        if (frame->isFlagSet(FrameFlags::Break)) {
          frame->clearFlag(FrameFlags::Break);
          fallOut = true;
          break;
        }

        if (frame->isFlagSet(FrameFlags::Next)) {
          frame->clearFlag(FrameFlags::Next);
          break;
        }
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
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

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

k_value KInterpreter::hashLoop(const ForLoopNode* node, const k_hash& hash) {
  auto frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);
  const auto& keys = hash->keys;
  const auto& kvp = hash->kvp;

  k_string valueIteratorName;
  k_string indexIteratorName;
  bool hasIndexIterator = false;

  valueIteratorName = id(node->valueIterator.get());

  if (node->indexIterator) {
    indexIteratorName = id(node->indexIterator.get());
    hasIndexIterator = true;
  }

  bool fallOut = false;
  k_value result;

  for (const auto& key : keys) {
    if (fallOut) {
      break;
    }

    frame->variables[valueIteratorName] = key;

    if (hasIndexIterator) {
      frame->variables[indexIteratorName] = kvp.at(key);
    }

    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT_STATEMENT &&
          stmt->type != ASTNodeType::BREAK_STATEMENT) {
        result = interpret(stmt.get());

        if (frame->isFlagSet(FrameFlags::Break)) {
          frame->clearFlag(FrameFlags::Break);
          fallOut = true;
          break;
        }

        if (frame->isFlagSet(FrameFlags::Next)) {
          frame->clearFlag(FrameFlags::Next);
          break;
        }
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
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

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

k_value KInterpreter::visit(const ForLoopNode* node) {
  k_value dataSetValue = interpret(node->dataSet.get());

  if (std::holds_alternative<k_list>(dataSetValue)) {
    return listLoop(node, std::get<k_list>(dataSetValue));
  }

  if (std::holds_alternative<k_hash>(dataSetValue)) {
    return hashLoop(node, std::get<k_hash>(dataSetValue));
  }

  throw InvalidOperationError(node->token,
                              "Expected a list value in for-loop.");
}

k_value KInterpreter::visit(const WhileLoopNode* node) {
  k_value result;
  auto frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);
  auto fallOut = false;

  while (MathImpl.is_truthy(interpret(node->condition.get()))) {
    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT_STATEMENT &&
          stmt->type != ASTNodeType::BREAK_STATEMENT) {
        result = interpret(stmt.get());

        if (frame->isFlagSet(FrameFlags::Break)) {
          frame->clearFlag(FrameFlags::Break);
          fallOut = true;
          break;
        }

        if (frame->isFlagSet(FrameFlags::Next)) {
          frame->clearFlag(FrameFlags::Next);
          break;
        }
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
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

    if (fallOut) {
      break;
    }
  }

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

k_value KInterpreter::visit(const BreakNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto& frame = callStack.top();
    frame->setFlag(FrameFlags::Break);
  }

  return {};
}

k_value KInterpreter::visit(const NextNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto& frame = callStack.top();
    frame->setFlag(FrameFlags::Next);
  }

  return {};
}

k_value KInterpreter::visit(const RepeatLoopNode* node) {
  k_value countValue = interpret(node->count.get());
  if (!std::holds_alternative<k_int>(countValue)) {
    throw InvalidOperationError(node->token,
                                "Repeat loop count must be an integer.");
  }
  k_int count = std::get<k_int>(countValue);
  k_string aliasName;
  k_value result;
  bool hasAlias = false;

  if (node->alias) {
    aliasName = id(node->alias.get());
    hasAlias = true;
  }

  auto frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);

  auto fallOut = false;
  for (k_int i = 1; i <= count; ++i) {
    if (fallOut) {
      break;
    }

    if (hasAlias) {
      frame->variables[aliasName] = i;
    }

    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT_STATEMENT &&
          stmt->type != ASTNodeType::BREAK_STATEMENT) {
        result = interpret(stmt.get());

        if (frame->isFlagSet(FrameFlags::Break)) {
          frame->clearFlag(FrameFlags::Break);
          fallOut = true;
          break;
        }

        if (frame->isFlagSet(FrameFlags::Next)) {
          frame->clearFlag(FrameFlags::Next);
          break;
        }
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
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

  if (hasAlias && frame->hasVariable(aliasName)) {
    frame->variables.erase(aliasName);
  }

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

k_value KInterpreter::visit(const TryNode* node) {
  try {
    for (const auto& stmt : node->tryBody) {
      interpret(stmt.get());
    }
  } catch (const KiwiError& e) {
    if (!node->catchBody.empty()) {
      auto frame = callStack.top();

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

  return {};
}

k_value KInterpreter::visit(const LambdaCallNode* node) {
  auto lambdaName =
      std::get<k_lambda>(interpret(node->lambdaNode.get()))->identifier;
  auto lambdaFrame = createFrame();
  k_value result;

  try {
    result = callLambda(lambdaFrame, node->token, lambdaName, node->arguments);
    dropFrame();
  } catch (const KiwiError& e) {
    dropFrame();
    throw;
  }

  return result;
}

k_value KInterpreter::visit(const LambdaNode* node) {
  std::vector<std::pair<std::string, k_value>> parameters;
  std::unordered_set<std::string> defaultParameters;
  auto tmpId = InterpHelper::getTemporaryId();

  parameters.reserve(node->parameters.size());
  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    k_value paramValue = {};
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }

    std::pair<std::string, k_value> param(paramName, paramValue);
    parameters.emplace_back(param);
  }

  auto lambda = std::make_unique<KLambda>(node);
  lambda->parameters = parameters;
  lambda->defaultParameters = defaultParameters;
  lambdas[tmpId] = std::move(lambda);
  lambdaTable[tmpId] = tmpId;

  return std::make_shared<LambdaRef>(tmpId);
}

k_value KInterpreter::visit(const ClassNode* node) {
  auto className = node->name;
  auto clazz = std::make_unique<KClass>();
  clazz->name = className;

  if (!node->baseClass.empty()) {
    clazz->baseClass = node->baseClass;
    if (classes.find(clazz->baseClass) == classes.end()) {
      throw ClassUndefinedError(node->token, clazz->baseClass);
    }
  }

  classStack.push(className);

  for (const auto& method : node->methods) {
    auto funcDecl = static_cast<const FunctionDeclarationNode*>(method.get());
    auto methodName = funcDecl->name;
    visit(funcDecl);

    if (methodName == Keywords.Ctor) {
      clazz->methods[Keywords.New] = std::move(methods[methodName]);
    } else {
      clazz->methods[methodName] = std::move(methods[methodName]);
    }
  }

  classes[className] = std::move(clazz);
  classStack.pop();
  methods.clear();

  return {};
}

k_value KInterpreter::visit(const FunctionDeclarationNode* node) {
  auto name = node->name;

  if (!packageStack.empty()) {
    name = packageStack.top() + "::" + name;
  }

  if (!classStack.empty()) {
    methods[name] = createFunction(node, name);
  } else {
    functions[name] = createFunction(node, name);
  }

  return {};
}

std::unique_ptr<KFunction> KInterpreter::createFunction(
    const FunctionDeclarationNode* node, const std::string& name) {
  std::vector<std::pair<std::string, k_value>> parameters;
  std::unordered_set<std::string> defaultParameters;

  parameters.reserve(node->parameters.size());

  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    k_value paramValue = {};
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }
    parameters.emplace_back(paramName, paramValue);
  }

  auto function = std::make_unique<KFunction>(node);
  function->name = name;
  function->parameters = parameters;
  function->defaultParameters = defaultParameters;
  function->isPrivate = node->isPrivate;
  function->isStatic = node->isStatic;

  return function;
}

k_value KInterpreter::visit(const FunctionCallNode* node) {
  auto callableType = getCallable(node->token, node->functionName);
  k_value result;

  if (callableType == KCallableType::Builtin) {
    return callBuiltinMethod(node);
  }

  auto functionFrame = createFrame();

  try {
    if (callableType == KCallableType::Method) {
      auto frame = callStack.top();
      if (!frame->inObjectContext()) {
        throw InvalidContextError(node->token);
      }

      auto& obj = frame->getObjectContext();
      auto& clazz = classes[obj->className];
      auto& clazzMethods = clazz->methods;

      const auto& func = clazzMethods[node->functionName];
      auto defaultParameters = func->defaultParameters;

      for (size_t i = 0; i < func->parameters.size(); ++i) {
        const auto& param = func->parameters[i];
        k_value argValue = {};
        if (i < node->arguments.size()) {
          const auto& arg = node->arguments[i];
          argValue = interpret(arg.get());
        } else if (defaultParameters.find(param.first) !=
                   defaultParameters.end()) {
          argValue = param.second;
        } else {
          throw ParameterCountMismatchError(node->token, node->functionName);
        }

        if (std::holds_alternative<k_lambda>(argValue)) {
          auto lambdaId = std::get<k_lambda>(argValue)->identifier;
          lambdaTable[param.first] = lambdaId;
        } else {
          functionFrame->variables[param.first] = argValue;
        }
      }

      callStack.push(functionFrame);

      const auto& decl = func->getBody();
      for (const auto& stmt : decl) {
        result = interpret(stmt.get());
        if (functionFrame->isFlagSet(FrameFlags::Return)) {
          result = functionFrame->returnValue;
          break;
        }
      }
    } else if (callableType == KCallableType::Function) {
      const auto& func = functions[node->functionName];
      auto defaultParameters = func->defaultParameters;

      for (size_t i = 0; i < func->parameters.size(); ++i) {
        const auto& param = func->parameters[i];
        k_value argValue = {};
        if (i < node->arguments.size()) {
          const auto& arg = node->arguments[i];
          argValue = interpret(arg.get());
        } else if (defaultParameters.find(param.first) !=
                   defaultParameters.end()) {
          argValue = param.second;
        } else {
          throw ParameterCountMismatchError(node->token, node->functionName);
        }

        if (std::holds_alternative<k_lambda>(argValue)) {
          auto lambdaId = std::get<k_lambda>(argValue)->identifier;
          lambdaTable[param.first] = lambdaId;
        } else {
          functionFrame->variables[param.first] = argValue;
        }
      }

      callStack.push(functionFrame);

      const auto& decl = func->getBody();
      for (const auto& stmt : decl) {
        result = interpret(stmt.get());
        if (functionFrame->isFlagSet(FrameFlags::Return)) {
          result = functionFrame->returnValue;
          break;
        }
      }
    } else if (callableType == KCallableType::Lambda) {
      result = callLambda(functionFrame, node->token, node->functionName,
                          node->arguments);
    }

    dropFrame();
  } catch (const KiwiError& e) {
    dropFrame();
    throw;
  }

  return result;
}

k_value KInterpreter::callLambda(
    std::shared_ptr<CallStackFrame>& lambdaFrame, const Token& token,
    const k_string& lambdaName,
    const std::vector<std::unique_ptr<ASTNode>>& arguments) {
  k_string targetLambda = lambdaName;
  k_value result;

  if (lambdas.find(targetLambda) == lambdas.end()) {
    if (lambdaTable.find(targetLambda) != lambdaTable.end()) {
      targetLambda = lambdaTable[targetLambda];
    }
  }

  const auto& func = lambdas[targetLambda];
  auto defaultParameters = func->defaultParameters;

  for (size_t i = 0; i < func->parameters.size(); ++i) {
    const auto& param = func->parameters[i];
    k_value argValue = {};
    if (i < arguments.size()) {
      const auto& arg = arguments[i];
      argValue = interpret(arg.get());
    } else if (defaultParameters.find(param.first) != defaultParameters.end()) {
      argValue = param.second;
    } else {
      throw ParameterCountMismatchError(token, targetLambda);
    }

    if (std::holds_alternative<k_lambda>(argValue)) {
      auto lambdaId = std::get<k_lambda>(argValue)->identifier;
      lambdaTable[param.first] = lambdaId;
    } else {
      lambdaFrame->variables[param.first] = argValue;
    }
  }

  lambdaFrame->setFlag(FrameFlags::InLambda);
  callStack.push(lambdaFrame);

  const auto& decl = func->getBody();
  for (const auto& stmt : decl) {
    result = interpret(stmt.get());
    if (lambdaFrame->isFlagSet(FrameFlags::Return)) {
      result = lambdaFrame->returnValue;
      break;
    }
  }

  return result;
}

KCallableType KInterpreter::getCallable(const Token& token,
                                        const std::string& name) const {
  if (functions.find(name) != functions.end()) {
    return KCallableType::Function;
  } else if (lambdas.find(name) != lambdas.end()) {
    return KCallableType::Lambda;
  } else if (KiwiBuiltins.is_builtin_method(name)) {
    return KCallableType::Builtin;
  }

  if (lambdaTable.find(name) != lambdaTable.end()) {
    return KCallableType::Lambda;
  }

  auto frame = callStack.top();

  if (frame->inObjectContext()) {
    auto& obj = frame->getObjectContext();
    auto& clazz = classes[obj->className];
    auto& clazzMethods = clazz->methods;
    if (clazzMethods.find(name) != clazzMethods.end()) {
      return KCallableType::Method;
    }
  }

  throw FunctionUndefinedError(token, name);
}

k_value KInterpreter::callFunction(
    const std::unique_ptr<KFunction>& function,
    const std::vector<std::unique_ptr<ASTNode>>& arguments, const Token& token,
    const std::string& functionName) {
  auto defaultParameters = function->defaultParameters;
  auto functionFrame = createFrame();

  for (size_t i = 0; i < function->parameters.size(); ++i) {
    const auto& param = function->parameters[i];
    k_value argValue = {};
    if (i < arguments.size()) {
      const auto& arg = arguments[i];
      argValue = interpret(arg.get());
    } else if (defaultParameters.find(param.first) != defaultParameters.end()) {
      argValue = param.second;
    } else {
      throw ParameterCountMismatchError(token, functionName);
    }

    if (std::holds_alternative<k_lambda>(argValue)) {
      auto lambdaId = std::get<k_lambda>(argValue)->identifier;
      lambdaTable[param.first] = lambdaId;
    } else {
      functionFrame->variables[param.first] = argValue;
    }
  }

  callStack.push(functionFrame);

  k_value result;

  try {
    result = executeFunctionBody(function);
    dropFrame();
  } catch (const KiwiError& e) {
    dropFrame();
    throw;
  }

  return result;
}

k_value KInterpreter::executeFunctionBody(
    const std::unique_ptr<KFunction>& function) {
  k_value result;
  const auto& decl = *function->decl;
  for (const auto& stmt : decl.body) {
    result = interpret(stmt.get());
    if (callStack.top()->isFlagSet(FrameFlags::Return)) {
      result = callStack.top()->returnValue;
      break;
    }
  }
  return result;
}

k_value KInterpreter::visit(const MethodCallNode* node) {
  auto object = interpret(node->object.get());

  if (std::holds_alternative<k_object>(object)) {
    return callObjectMethod(node, std::get<k_object>(object));
  } else if (std::holds_alternative<k_class>(object)) {
    return callClassMethod(node, std::get<k_class>(object));
  } else if (ListBuiltins.is_builtin(node->op)) {
    return interpretListBuiltin(node->token, object, node->op,
                                getMethodCallArguments(node->arguments));
  } else if (KiwiBuiltins.is_builtin(node->op)) {
    return BuiltinDispatch::execute(node->token, node->op, object,
                                    getMethodCallArguments(node->arguments));
  }

  throw UnknownBuiltinError(node->token, node->methodName);
}

std::vector<k_value> KInterpreter::getMethodCallArguments(
    const std::vector<std::unique_ptr<ASTNode>>& args) {
  std::vector<k_value> arguments;
  arguments.reserve(args.size());

  for (const auto& arg : args) {
    arguments.emplace_back(interpret(arg.get()));
  }

  return arguments;
}

k_value KInterpreter::callObjectBaseMethod(const MethodCallNode* node,
                                           const std::shared_ptr<Object>& obj,
                                           const k_string& baseClass,
                                           const k_string& methodName) {
  const auto& clazz = classes[baseClass];
  auto& function = clazz->methods[methodName];
  bool isCtor = methodName == Keywords.New;

  auto& frame = callStack.top();
  auto objContext = obj;
  bool contextSwitch = false;

  if (frame->inObjectContext()) {
    objContext = frame->getObjectContext();
    contextSwitch = true;
  }

  frame->setObjectContext(obj);

  if (!function) {
    throw UnimplementedMethodError(node->token, obj->className, methodName);
  }

  if (function->isPrivate) {
    throw InvalidContextError(node->token,
                              "Cannot invoke private method outside of class.");
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (contextSwitch) {
    frame->setObjectContext(objContext);
  }

  if (isCtor) {
    return obj;
  }

  return result;
}

k_value KInterpreter::callObjectMethod(const MethodCallNode* node,
                                       const std::shared_ptr<Object>& obj) {
  const auto& clazz = classes[obj->className];
  auto methodName = node->methodName;

  if (clazz->methods.find(methodName) == clazz->methods.end()) {
    auto baseClass = clazz->baseClass;

    if (baseClass.empty()) {
      throw UnimplementedMethodError(node->token, obj->className, methodName);
    }

    if (classes.find(baseClass) == classes.end()) {
      throw ClassUndefinedError(node->token, baseClass);
    }

    return callObjectBaseMethod(node, obj, baseClass, methodName);
  }

  auto& function = clazz->methods[methodName];
  bool isCtor = methodName == Keywords.New;

  auto& frame = callStack.top();
  auto objContext = obj;
  bool contextSwitch = false;

  if (frame->inObjectContext()) {
    objContext = frame->getObjectContext();
    contextSwitch = true;
  }

  frame->setObjectContext(obj);

  if (!function) {
    throw UnimplementedMethodError(node->token, obj->className, methodName);
  }

  if (function->isPrivate) {
    throw InvalidContextError(node->token,
                              "Cannot invoke private method outside of class.");
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (contextSwitch) {
    frame->setObjectContext(objContext);
  }

  if (isCtor) {
    return obj;
  }

  return result;
}

k_value KInterpreter::callClassMethod(const MethodCallNode* node,
                                      const k_class& clazz) {
  auto methodName = node->methodName;
  auto& frame = callStack.top();
  const auto& kclass = classes[clazz->identifier];
  auto& function = kclass->methods[methodName];
  k_object obj = std::make_shared<Object>();
  bool isCtor = methodName == Keywords.New;

  if (!function && isCtor) {
    return obj;  // default constructor
  }

  if (!function->isStatic && !isCtor) {
    throw InvalidContextError(node->token,
                              "Cannot invoke non-static method on class.");
  }

  if (isCtor) {
    obj->className = clazz->identifier;
    frame->setObjectContext(obj);
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (isCtor) {
    frame->clearFlag(FrameFlags::InObject);
    return obj;
  }

  return result;
}

k_value KInterpreter::callBuiltinMethod(const FunctionCallNode* node) {
  auto args = getMethodCallArguments(node->arguments);
  auto op = node->op;
  if (SerializerBuiltins.is_builtin(op)) {
    return interpretSerializerBuiltin(node->token, op, args);
  } else if (ReflectorBuiltins.is_builtin(op)) {
    return interpretReflectorBuiltin(node->token, op, args);
  } else if (WebServerBuiltins.is_builtin(op)) {
    return interpretWebServerBuiltin(node->token, op, args);
  }

  return BuiltinDispatch::execute(node->token, op, args, kiwiArgs);
}

k_value KInterpreter::interpretWebServerBuiltin(const Token& token,
                                                const KName& builtin,
                                                std::vector<k_value>& args) {
  switch (builtin) {
    case KName::Builtin_WebServer_Get:
      return interpretWebServerGet(token, args);

    case KName::Builtin_WebServer_Post:
      return interpretWebServerPost(token, args);

    case KName::Builtin_WebServer_Listen:
      return interpretWebServerListen(token, args);

    case KName::Builtin_WebServer_Public:
      return interpretWebServerPublic(token, args);

    default:
      break;
  }

  return {};
}

void KInterpreter::handleWebServerRequest(int webhookID, k_hash requestHash,
                                          k_string& redirect, k_string& content,
                                          k_string& contentType, int& status) {
  auto webhook = serverHooks[webhookID];
  auto webhookFrame = createFrame();

  auto& lambda = lambdas[webhook];

  for (const auto& param : lambda->parameters) {
    webhookFrame->variables[param.first] = requestHash;
    break;
  }

  k_value result;

  try {
    const auto& decl = lambda->getBody();
    for (const auto& stmt : decl) {
      result = interpret(stmt.get());
      if (webhookFrame->isFlagSet(FrameFlags::Return)) {
        result = webhookFrame->returnValue;
        break;
      }
    }

    if (std::holds_alternative<k_hash>(result)) {
      auto responseHash = std::get<k_hash>(result);
      if (responseHash->hasKey("content")) {
        auto responseHashContent = responseHash->get("content");
        content = Serializer::serialize(responseHashContent);
      }

      if (responseHash->hasKey("content-type")) {
        auto responseHashContent = responseHash->get("content-type");
        if (std::holds_alternative<k_string>(responseHashContent)) {
          contentType = std::get<k_string>(responseHashContent);
        }
      }

      if (responseHash->hasKey("status")) {
        auto responseHashContent = responseHash->get("status");
        if (std::holds_alternative<k_int>(responseHashContent)) {
          status = static_cast<int>(std::get<k_int>(responseHashContent));
        }
      }

      if (responseHash->hasKey("redirect")) {
        auto responseHashContent = responseHash->get("redirect");
        if (std::holds_alternative<k_string>(responseHashContent)) {
          redirect = std::get<k_string>(responseHashContent);
        }
      }
    }

    dropFrame();
  } catch (const KiwiError& e) {
    dropFrame();
    throw;
  }
}

int KInterpreter::getNextWebServerHook(const Token& token, k_value& arg) {
  if (!std::holds_alternative<k_lambda>(arg)) {
    throw InvalidOperationError(token,
                                "Expected lambda for second parameter of `" +
                                    WebServerBuiltins.Get + "`.");
  }

  auto lambdaName = std::get<k_lambda>(arg)->identifier;
  if (lambdas.find(lambdaName) == lambdas.end()) {
    if (lambdaTable.find(lambdaName) != lambdaTable.end()) {
      lambdaName = lambdaTable[lambdaName];
    }
  }
  int webhookID = 0;

  if (!serverHooks.empty()) {
    webhookID = static_cast<int>(serverHooks.size());
  }

  serverHooks[webhookID] = lambdaName;
  return webhookID;
}

k_value KInterpreter::interpretWebServerGet(const Token& token,
                                            std::vector<k_value>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Get);
  }

  auto endpointList = InterpHelper::getWebServerEndpointList(token, args.at(0));
  int webhookID = getNextWebServerHook(token, args.at(1));

  for (const auto& endpoint : endpointList) {
    server.Get(endpoint, [this, webhookID](const httplib::Request& req,
                                           httplib::Response& res) {
      auto requestHash = InterpHelper::getWebServerRequestHash(req);

      k_string content, redirect;
      k_string contentType = "text/plain";
      int status = 500;
      handleWebServerRequest(webhookID, requestHash, redirect, content,
                             contentType, status);

      res.status = status;
      res.set_content(content, contentType);
    });
  }

  return {};
}

k_value KInterpreter::interpretWebServerPost(const Token& token,
                                             std::vector<k_value>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Get);
  }

  auto endpointList = InterpHelper::getWebServerEndpointList(token, args.at(0));
  int webhookID = getNextWebServerHook(token, args.at(1));

  for (const auto& endpoint : endpointList) {
    server.Post(endpoint, [this, webhookID](const httplib::Request& req,
                                            httplib::Response& res) {
      auto requestHash = InterpHelper::getWebServerRequestHash(req);

      k_string content, redirect;
      k_string contentType = "text/plain";
      int status = 500;
      handleWebServerRequest(webhookID, requestHash, redirect, content,
                             contentType, status);

      if (!redirect.empty()) {
        res.set_redirect(redirect);
      } else {
        res.status = status;
        res.set_content(content, contentType);
      }
    });
  }

  return {};
}

k_value KInterpreter::interpretWebServerListen(const Token& token,
                                               std::vector<k_value>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Listen);
  }

  auto host = get_string(token, args.at(0));
  auto port = get_integer(token, args.at(1));

  server.listen(host, static_cast<int>(port));

  auto hash = std::make_shared<Hash>();
  hash->add("host", host);
  hash->add("port", port);

  return hash;
}

k_value KInterpreter::interpretWebServerPublic(const Token& token,
                                               std::vector<k_value>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Public);
  }

  auto endpoint = get_string(token, args.at(0));
  auto publicDir = get_string(token, args.at(1));

  if (!File::directoryExists(publicDir)) {
    return false;
  }

  server.set_mount_point(endpoint, publicDir);

  return true;
}

k_value KInterpreter::interpretReflectorBuiltin(const Token& token,
                                                const KName& builtin,
                                                std::vector<k_value>& args) {
  if (builtin != KName::Builtin_Reflector_RList) {
    throw InvalidOperationError(token, "Come back later.");
  }

  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RList);
  }

  auto rlist = std::make_shared<Hash>();
  auto rlistPackages = std::make_shared<List>();
  auto rlistClasses = std::make_shared<List>();
  auto rlistFunctions = std::make_shared<List>();
  auto rlistStack = std::make_shared<List>();

  rlistPackages->elements.reserve(packages.size());
  rlistClasses->elements.reserve(classes.size());
  rlistFunctions->elements.reserve(functions.size());
  rlistStack->elements.reserve(callStack.size());

  for (const auto& m : methods) {
    rlistFunctions->elements.emplace_back(m.first);
  }

  for (const auto& p : packages) {
    rlistPackages->elements.emplace_back(p.first);
  }

  for (const auto& c : classes) {
    rlistClasses->elements.emplace_back(c.first);
  }

  std::stack<std::shared_ptr<CallStackFrame>> tempStack(callStack);

  while (!tempStack.empty()) {
    const auto& outerFrame = tempStack.top();
    const auto& frameVariables = outerFrame->variables;

    auto rlistStackFrame = std::make_shared<Hash>();
    auto rlistStackFrameVariables = std::make_shared<List>();

    rlistStackFrameVariables->elements.reserve(frameVariables.size());

    for (const auto& v : frameVariables) {
      auto rlistStackFrameVariable = std::make_shared<Hash>();
      rlistStackFrameVariable->add(v.first, v.second);
      rlistStackFrameVariables->elements.emplace_back(rlistStackFrameVariable);
    }

    k_string tmp;
    sort_list(*rlistStackFrameVariables);

    rlistStackFrame->add("variables", rlistStackFrameVariables);
    rlistStack->elements.emplace_back(rlistStackFrame);

    tempStack.pop();
  }

  sort_list(*rlistPackages);
  sort_list(*rlistClasses);
  sort_list(*rlistFunctions);
  std::reverse(rlistStack->elements.begin(), rlistStack->elements.end());

  rlist->add("packages", rlistPackages);
  rlist->add("classes", rlistClasses);
  rlist->add("functions", rlistFunctions);
  rlist->add("stack", rlistStack);

  return rlist;
}

k_value KInterpreter::interpolateString(const Token& token,
                                        const k_string& input) {
  Parser parser;
  Lexer lexer(token.getFile(), input);
  auto tempStream = lexer.getTokenStream();
  auto ast = parser.parseTokenStream(tempStream, true);

  return interpret(ast.get());
}

k_value KInterpreter::interpretSerializerDeserialize(
    const Token& token, std::vector<k_value>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SerializerBuiltins.Deserialize);
  }

  return interpolateString(token, get_string(token, args.at(0)));
}

k_value KInterpreter::interpretSerializerSerialize(const Token& token,
                                                   std::vector<k_value>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SerializerBuiltins.Serialize);
  }

  return Serializer::serialize(args.at(0), true);
}

k_value KInterpreter::interpretSerializerBuiltin(const Token& token,
                                                 const KName& builtin,
                                                 std::vector<k_value>& args) {
  switch (builtin) {
    case KName::Builtin_Serializer_Deserialize:
      return interpretSerializerDeserialize(token, args);

    case KName::Builtin_Serializer_Serialize:
      return interpretSerializerSerialize(token, args);

    default:
      break;
  }

  return {};
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

    auto& lambda = lambdas[lambdaRef->identifier];
    const auto isReturnSet = callStack.top()->isFlagSet(FrameFlags::Return);
    k_value result;

    switch (op) {
      case KName::Builtin_List_Each:
        result = lambdaEach(lambda, list);
        break;

      case KName::Builtin_List_Map:
        result = lambdaMap(lambda, list);
        break;

      case KName::Builtin_List_None:
        result = lambdaNone(lambda, list);
        break;

      case KName::Builtin_List_Select:
        result = lambdaSelect(lambda, list);
        break;

      case KName::Builtin_List_All:
        result = lambdaAll(lambda, list);
        break;

      default:
        break;
    }

    auto frame = callStack.top();
    if (!isReturnSet && frame->isFlagSet(FrameFlags::Return)) {
      if (frame->returnValue != result) {
        frame->returnValue = result;
      }
    }

    return result;
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

    auto& lambda = lambdas[lambdaRef->identifier];

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

k_value KInterpreter::lambdaEach(std::unique_ptr<KLambda>& lambda,
                                 const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string valueVariable;
  k_string indexVariable;
  bool hasIndexVariable = false;

  if (lambda->parameters.empty()) {
    return {};
  }

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      valueVariable = param.first;
      frame->variables[valueVariable] = {};
    } else if (i == 1) {
      indexVariable = param.first;
      hasIndexVariable = true;
      frame->variables[indexVariable] = {};
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

k_value KInterpreter::lambdaNone(std::unique_ptr<KLambda>& lambda,
                                 const k_list& list) {
  auto selected = lambdaSelect(lambda, list);
  if (std::holds_alternative<k_list>(selected)) {
    return std::get<k_list>(selected)->elements.empty();
  }
  return false;
}

k_value KInterpreter::lambdaMap(std::unique_ptr<KLambda>& lambda,
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
      frame->variables[mapVariable] = {};
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

k_value KInterpreter::lambdaReduce(std::unique_ptr<KLambda>& lambda,
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
      frame->variables[valueVariable] = {};
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

k_value KInterpreter::lambdaAll(std::unique_ptr<KLambda>& lambda,
                                const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string valueVariable;
  k_string indexVariable;
  bool hasIndexVariable = false;

  const auto& listSize = list->elements.size();
  size_t newListSize = 0;

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      valueVariable = param.first;
      frame->variables[valueVariable] = {};
    } else if (i == 1) {
      indexVariable = param.first;
      hasIndexVariable = true;
      frame->variables[indexVariable] = {};
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

      if (MathImpl.is_truthy(result)) {
        ++newListSize;
      }
    }
  }

  frame->variables.erase(valueVariable);
  if (hasIndexVariable) {
    frame->variables.erase(indexVariable);
  }

  return newListSize == listSize;
}

k_value KInterpreter::lambdaSelect(std::unique_ptr<KLambda>& lambda,
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
      frame->variables[valueVariable] = {};
    } else if (i == 1) {
      indexVariable = param.first;
      hasIndexVariable = true;
      frame->variables[indexVariable] = {};
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
#ifndef KIWI_INTERPRETER_H
#define KIWI_INTERPRETER_H

#include <algorithm>
#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <signal.h>
#include <thread>
#include <unordered_map>
#include <unistd.h>

#include "globals.h"
#include "builtin.h"
#include "math/functions.h"
#include "parsing/ast.h"
#include "parsing/builtins.h"
#include "stackframe.h"
#include "tracing/error.h"
#include "typing/value.h"
#include "util/file.h"
#include "concurrency/task.h"
#include "ffi/ffimanager.h"
#include "net/socketmanager.h"
#include "callable.h"
#include "context.h"

std::atomic<bool> signal_pending(false);
std::atomic<int> last_received_signal;
std::mutex signal_mutex;
std::unordered_map<int, k_string> k_signal_handlers;

class KInterpreter {
 public:
  KInterpreter() : ctx(nullptr), taskmgr(), ffimgr(), sockmgr() {}

  KValue interpret(const ASTNode* node);

  void setContext(std::unique_ptr<KContext> context) {
    ctx = std::move(context);
  }

  void setProgramArgs(const std::unordered_map<k_string, k_string> args) {
    cliArgs = args;
  }

  bool hasActiveTasks() { return taskmgr.hasActiveTasks(); }

 private:
  std::unique_ptr<KContext> ctx;
  TaskManager taskmgr;
  FFIManager ffimgr;
  SocketManager sockmgr;
  std::stack<std::shared_ptr<CallStackFrame>> callStack;
  std::stack<k_string> packageStack;
  std::stack<k_string> structStack;
  std::stack<k_string> funcStack;
  std::unordered_map<k_string, k_string> cliArgs;

  const int SAFEMODE_MAX_ITERATIONS = 1000000;

  static void k_signal_handler(int signum) {
    std::lock_guard<std::mutex> lock(signal_mutex);
    last_received_signal.store(signum, std::memory_order_relaxed);
    signal_pending.store(true, std::memory_order_relaxed);
  }

  KValue visit(const ProgramNode* node);
  KValue visit(const SelfNode* node);
  KValue visit(const PackAssignmentNode* node);
  KValue visit(const AssignmentNode* node);
  KValue visit(const ConstAssignmentNode* node);
  KValue visit(const IndexAssignmentNode* node);
  KValue visit(const MemberAssignmentNode* node);
  KValue visit(const MemberAccessNode* node);
  KValue visit(const IdentifierNode* node);
  KValue visit(const LiteralNode* node);
  KValue visit(const ListLiteralNode* node);
  KValue visit(const RangeLiteralNode* node);
  KValue visit(const HashLiteralNode* node);
  KValue visit(const PrintNode* node);
  KValue visit(const PrintXyNode* node);
  KValue visit(const UnaryOperationNode* node);
  KValue visit(const BinaryOperationNode* node);
  KValue visit(const TernaryOperationNode* node);
  KValue visit(const IfNode* node);
  KValue visit(const CaseNode* node);
  KValue visit(const ForLoopNode* node);
  KValue visit(const WhileLoopNode* node);
  KValue visit(const RepeatLoopNode* node);
  KValue visit(const BreakNode* node);
  KValue visit(const NextNode* node);
  KValue visit(const TryNode* node);
  KValue visit(const LambdaNode* node);
  KValue visit(const LambdaCallNode* node);
  KValue visit(const StructNode* node);
  KValue visit(const FunctionDeclarationNode* node);
  KValue visit(const VariableDeclarationNode* node);
  KValue visit(const FunctionCallNode* node);
  KValue visit(const MethodCallNode* node);
  KValue visit(const PackageNode* node);
  KValue visit(const ImportNode* node);
  KValue visit(const ExportNode* node);
  KValue visit(const ParseNode* node);
  KValue visit(const ExitNode* node);
  KValue visit(const ThrowNode* node);
  KValue visit(const ReturnNode* node);
  KValue visit(const IndexingNode* node);
  KValue visit(const SliceNode* node);
  KValue visit(const SpawnNode* node);

  // Stack frame management
  std::shared_ptr<CallStackFrame> createFrame(const k_string& name,
                                              bool isMethodInvocation);
  bool pushFrame(std::shared_ptr<CallStackFrame> frame) {
    callStack.push(frame);
    funcStack.push(frame->name);
    return true;
  }
  void dropFrame();

  k_string getTemporaryId();
  k_string id(const ASTNode* node);

  void importPackage(const KValue& packageName, const Token& token);
  void importExternal(const k_string& packageName, const Token& token);

  KCallableType getCallable(const Token& token, const k_string& name) const;
  std::vector<KValue> getMethodCallArguments(
      const std::vector<std::unique_ptr<ASTNode>>& args);
  KValue callBuiltinMethod(const FunctionCallNode* node);
  KValue callStructMethod(const MethodCallNode* node, const k_struct& struc);
  KValue callObjectBaseMethod(const MethodCallNode* node,
                              const std::shared_ptr<Object>& obj,
                              const k_string& baseStruct,
                              const k_string& methodName);
  KValue callObjectMethod(const MethodCallNode* node,
                          const std::shared_ptr<Object>& obj);
  KValue callLambda(const Token& token, const k_string& lambdaName,
                    const std::vector<std::unique_ptr<ASTNode>>& arguments,
                    bool& requireDrop);
  KValue callLambda(const Token& token, const k_string& lambdaName,
                    const std::vector<KValue>& arguments);
  void prepareLambdaCall(const std::unique_ptr<KLambda>& func,
                         const std::vector<std::unique_ptr<ASTNode>>& arguments,
                         std::unordered_set<k_string>& defaultParameters,
                         const Token& token, k_string& targetLambda,
                         const std::unordered_map<k_string, KName>& typeHints,
                         const k_string& lambdaName,
                         std::shared_ptr<CallStackFrame>& lambdaFrame);
  void prepareLambdaVariables(
      const std::unordered_map<k_string, KName>& typeHints,
      const std::pair<k_string, KValue>& param, KValue& argValue,
      const Token& token, size_t i, const k_string& lambdaName,
      std::shared_ptr<CallStackFrame>& lambdaFrame);
  void prepareLambdaCall(const std::unique_ptr<KLambda>& func,
                         const std::vector<KValue>& arguments,
                         std::unordered_set<k_string>& defaultParameters,
                         const Token& token, k_string& targetLambda,
                         const std::unordered_map<k_string, KName>& typeHints,
                         const k_string& lambdaName,
                         std::shared_ptr<CallStackFrame>& lambdaFrame);
  KValue callFunction(const std::unique_ptr<KFunction>& function,
                      const std::vector<std::unique_ptr<ASTNode>>& arguments,
                      const Token& token, const k_string& functionName);
  KValue callFunction(const FunctionCallNode* node, bool& requireDrop);

  void prepareFunctionVariables(
      const std::unordered_map<k_string, KName>& typeHints,
      const std::pair<k_string, KValue>& param, KValue& argValue,
      const Token& token, size_t i, const k_string& functionName,
      std::shared_ptr<CallStackFrame>& functionFrame);

  std::unique_ptr<KFunction> createFunction(const FunctionDeclarationNode* node,
                                            const k_string& name);

  KValue executeStructMethod(
      std::unordered_map<k_string, std::unique_ptr<KFunction>>& methods,
      k_string& methodName, std::shared_ptr<CallStackFrame>& frame,
      const MethodCallNode* node, const k_struct& struc);
  KValue executeInstanceMethod(const FunctionCallNode* node, bool& requireDrop);
  KValue executeInstanceMethodFunction(
      std::unordered_map<k_string, std::unique_ptr<KFunction>>& strucMethods,
      const FunctionCallNode* node, bool& requireDrop);
  void prepareFunctionCall(const std::unique_ptr<KFunction>& func,
                           const FunctionCallNode* node,
                           std::unordered_set<k_string>& defaultParameters,
                           const std::unordered_map<k_string, KName>& typeHints,
                           std::shared_ptr<CallStackFrame>& functionFrame);
  KValue executeFunctionBody(const std::unique_ptr<KFunction>& function);

  KValue handleNestedIndexing(const IndexingNode* indexExpr, KValue baseObj,
                              const KName& op, const KValue& newValue);

  // Scope value propagation
  bool shouldUpdateFrameVariables(
      const k_string& varName, const std::shared_ptr<CallStackFrame> nextFrame);
  void updateVariablesInCallerFrame(
      const std::unordered_map<k_string, KValue>& variables,
      std::shared_ptr<CallStackFrame> callerFrame);

  // Slices
  SliceIndex getSlice(const SliceNode* node, KValue object);
  KValue doSliceAssignment(const Token& token, KValue& slicedObj,
                           const SliceIndex& slice, KValue& newValue);
  void updateListSlice(const Token& token, bool insertOp, k_list& targetList,
                       const SliceIndex& slice, const k_list& rhsValues);
  KValue stringSlice(const Token& token, SliceIndex& slice,
                     const k_string& value);
  KValue listSlice(const Token& token, const SliceIndex& slice,
                   const k_list& value);

  // Loops.
  KValue listLoop(const ForLoopNode* node, const k_list& list);
  KValue hashLoop(const ForLoopNode* node, const k_hashmap& hash);

  // Signals
  KValue interpretSignalBuiltin(const Token& token, const KName& op,
                                std::vector<KValue> args);
  KValue interpretSignalTrap(const Token& token, std::vector<KValue> args);
  KValue interpretSignalRaise(const Token& token,
                              const std::vector<KValue>& args);
  KValue interpretSignalSend(const Token& token,
                             const std::vector<KValue>& args);
  void handlePendingSignals(const Token& token);

  // List builtins
  KValue interpretListBuiltin(const Token& token, KValue& object,
                              const KName& op, std::vector<KValue> args);
  KValue listSum(const k_list& list);
  KValue listMin(const Token& token, const k_list& list);
  KValue listMax(const Token& token, const k_list& list);
  KValue listSort(const k_list& list);
  KValue lambdaEach(std::unique_ptr<KLambda>& lambda, const k_list& list);
  KValue lambdaNone(std::unique_ptr<KLambda>& lambda, const k_list& list);
  KValue lambdaMap(std::unique_ptr<KLambda>& lambda, const k_list& list);
  KValue lambdaReduce(std::unique_ptr<KLambda>& lambda, KValue accumulator,
                      const k_list& list);
  KValue lambdaSelect(std::unique_ptr<KLambda>& lambda, const k_list& list);
  KValue lambdaAll(std::unique_ptr<KLambda>& lambda, const k_list& list);

  // Serialization
  KValue interpolateString(const Token& token, const k_string& input);

  KValue interpretSerializerDeserialize(const Token& token,
                                        std::vector<KValue>& args);
  KValue interpretSerializerSerialize(const Token& token,
                                      std::vector<KValue>& args);
  KValue interpretSerializerBuiltin(const Token& token, const KName& builtin,
                                    std::vector<KValue>& args);

  // Reflection
  KValue interpretReflectorBuiltin(const Token& token, const KName& builtin,
                                   std::vector<KValue>& args);
  KValue interpretReflectorRList(const Token& token, std::vector<KValue>& args);
  KValue interpretReflectorRObject(const Token& token,
                                   std::vector<KValue>& args);
  KValue interpretReflectorRStack(const Token& token,
                                  std::vector<KValue>& args);
  KValue interpretReflectorRFFlags(const Token& token,
                                   std::vector<KValue>& args);
  KValue interpretReflectorRRetVal(const Token& token,
                                   std::vector<KValue>& args);

  // Web server
  KValue interpretWebServerBuiltin(const Token& token, const KName& builtin,
                                   std::vector<KValue>& args);
  KValue interpretWebServerGet(const Token& token, std::vector<KValue>& args);
  KValue interpretWebServerPost(const Token& token, std::vector<KValue>& args);
  KValue interpretWebServerListen(const Token& token,
                                  std::vector<KValue>& args);
  KValue interpretWebServerPublic(const Token& token,
                                  std::vector<KValue>& args);
  int getNextWebServerHook(const Token& token, KValue& arg);
  k_hashmap getWebServerRequestHash(const httplib::Request& req);
  void handleWebServerRequest(int webhookID, k_hashmap requestHash,
                              k_string& redirect, k_string& content,
                              k_string& contentType, int& status);
  std::vector<k_string> getWebServerEndpointList(const Token& token,
                                                 KValue& arg);
};

KValue KInterpreter::interpret(const ASTNode* node) {
  if (!node) {
    return {};
  }

  KValue result = {};
  bool sigCheck = false;

  switch (node->type) {
    case ASTNodeType::PROGRAM:
      result = visit(static_cast<const ProgramNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::SELF:
      result = visit(static_cast<const SelfNode*>(node));
      break;

    case ASTNodeType::PACKAGE:
      result = visit(static_cast<const PackageNode*>(node));
      break;

    case ASTNodeType::STRUCT:
      result = visit(static_cast<const StructNode*>(node));
      break;

    case ASTNodeType::IMPORT:
      result = visit(static_cast<const ImportNode*>(node));
      break;

    case ASTNodeType::EXPORT:
      result = visit(static_cast<const ExportNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::EXIT:
      result = visit(static_cast<const ExitNode*>(node));
      break;

    case ASTNodeType::THROW:
      result = visit(static_cast<const ThrowNode*>(node));
      break;

    case ASTNodeType::ASSIGNMENT:
      result = visit(static_cast<const AssignmentNode*>(node));
      break;

    case ASTNodeType::CONST_ASSIGNMENT:
      result = visit(static_cast<const ConstAssignmentNode*>(node));
      break;

    case ASTNodeType::INDEX_ASSIGNMENT:
      result = visit(static_cast<const IndexAssignmentNode*>(node));
      break;

    case ASTNodeType::MEMBER_ASSIGNMENT:
      result = visit(static_cast<const MemberAssignmentNode*>(node));
      break;

    case ASTNodeType::PACK_ASSIGNMENT:
      result = visit(static_cast<const PackAssignmentNode*>(node));
      break;

    case ASTNodeType::MEMBER_ACCESS:
      result = visit(static_cast<const MemberAccessNode*>(node));
      break;

    case ASTNodeType::LITERAL:
      result = visit(static_cast<const LiteralNode*>(node));
      break;

    case ASTNodeType::LIST_LITERAL:
      result = visit(static_cast<const ListLiteralNode*>(node));
      break;

    case ASTNodeType::RANGE_LITERAL:
      result = visit(static_cast<const RangeLiteralNode*>(node));
      break;

    case ASTNodeType::HASH_LITERAL:
      result = visit(static_cast<const HashLiteralNode*>(node));
      break;

    case ASTNodeType::IDENTIFIER:
      result = visit(static_cast<const IdentifierNode*>(node));
      break;

    case ASTNodeType::PRINT:
      result = visit(static_cast<const PrintNode*>(node));
      break;

    case ASTNodeType::PRINTXY:
      result = visit(static_cast<const PrintXyNode*>(node));
      break;

    case ASTNodeType::TERNARY_OPERATION:
      result = visit(static_cast<const TernaryOperationNode*>(node));
      break;

    case ASTNodeType::BINARY_OPERATION:
      result = visit(static_cast<const BinaryOperationNode*>(node));
      break;

    case ASTNodeType::UNARY_OPERATION:
      result = visit(static_cast<const UnaryOperationNode*>(node));
      break;

    case ASTNodeType::IF:
      result = visit(static_cast<const IfNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::CASE:
      result = visit(static_cast<const CaseNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::FOR_LOOP:
      result = visit(static_cast<const ForLoopNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::WHILE_LOOP:
      result = visit(static_cast<const WhileLoopNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::REPEAT_LOOP:
      result = visit(static_cast<const RepeatLoopNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::BREAK:
      result = visit(static_cast<const BreakNode*>(node));
      break;

    case ASTNodeType::NEXT:
      result = visit(static_cast<const NextNode*>(node));
      break;

    case ASTNodeType::TRY:
      result = visit(static_cast<const TryNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::LAMBDA:
      result = visit(static_cast<const LambdaNode*>(node));
      break;

    case ASTNodeType::LAMBDA_CALL:
      result = visit(static_cast<const LambdaCallNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::FUNCTION:
      result = visit(static_cast<const FunctionDeclarationNode*>(node));
      break;

    case ASTNodeType::VARIABLE:
      result = visit(static_cast<const VariableDeclarationNode*>(node));
      break;

    case ASTNodeType::FUNCTION_CALL:
      result = visit(static_cast<const FunctionCallNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::METHOD_CALL:
      result = visit(static_cast<const MethodCallNode*>(node));
      sigCheck = true;
      break;

    case ASTNodeType::RETURN:
      result = visit(static_cast<const ReturnNode*>(node));
      break;

    case ASTNodeType::INDEX:
      result = visit(static_cast<const IndexingNode*>(node));
      break;

    case ASTNodeType::SLICE:
      result = visit(static_cast<const SliceNode*>(node));
      break;

    case ASTNodeType::PARSE:
      result = visit(static_cast<const ParseNode*>(node));
      break;

    case ASTNodeType::NO_OP:
      break;

    case ASTNodeType::SPAWN:
      result = visit(static_cast<const SpawnNode*>(node));
      break;

    default:
      node->print();
      break;
  }

  if (sigCheck) {
    handlePendingSignals(node->token);
  }

  return result;
}

std::shared_ptr<CallStackFrame> KInterpreter::createFrame(
    const k_string& name, bool isMethodInvocation = false) {
  std::shared_ptr<CallStackFrame> frame = callStack.top();
  auto subFrame = std::make_shared<CallStackFrame>();
  auto& subFrameVariables = subFrame->variables;

  subFrame->name = name;

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

void KInterpreter::dropFrame() {
  if (callStack.empty()) {
    return;
  }

  if (!funcStack.empty()) {
    funcStack.pop();
  }

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

    updateVariablesInCallerFrame(topVariables, callerFrame);
  }
}

k_string KInterpreter::id(const ASTNode* node) {
  return static_cast<const IdentifierNode*>(node)->name;
}

KValue KInterpreter::visit(const SpawnNode* node) {
  auto frame = std::make_shared<CallStackFrame>();
  auto top = callStack.top();

  frame->name = Keywords.Spawn;

  for (const auto& var : top->variables) {
    frame->variables[var.first] = clone_value(var.second);
  }

  auto taskExpr = node->expression->clone();

  auto interp = std::make_shared<KInterpreter>();
  interp->setContext(ctx->clone());
  interp->pushFrame(frame);

  TaskManager::TaskFunction task(
      [interp, frame, taskExpr = std::move(taskExpr)]() {
        auto result = interp->interpret(taskExpr.get());

        if (frame->isFlagSet(FrameFlags::Return)) {
          result = frame->returnValue;
        }

        return result;
      });

  auto taskId = taskmgr.addTask(std::move(task));

  return KValue::createInteger(taskId);
}

KValue KInterpreter::visit(const ProgramNode* node) {
  // This is the program root
  if (!node->isScript) {
    auto programFrame = std::make_shared<CallStackFrame>();
    programFrame->name = kiwi_name;
    programFrame->variables[Keywords.Global] =
        KValue::createHashmap(std::make_shared<Hashmap>());
    pushFrame(programFrame);
  }

  KValue result;

  for (const auto& stmt : node->statements) {
    result = interpret(stmt.get());
  }

  return result;
}

KValue KInterpreter::visit(const ExitNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto exitValue = interpret(node->exitValue.get());
    auto exitCode = static_cast<k_int>(0);

    if (exitValue.isInteger()) {
      exitCode = exitValue.getInteger();
    } else {
      exitCode = static_cast<k_int>(1);
    }

    exit(static_cast<int>(exitCode));
  }

  return {};
}

KValue KInterpreter::visit(const ReturnNode* node) {
  KValue returnValue = {};

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

KValue KInterpreter::visit(const ThrowNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    k_string errorType = "KiwiError";
    k_string errorMessage;
    if (node->errorValue) {
      auto errorValue = interpret(node->errorValue.get());

      if (errorValue.isHashmap()) {
        auto errorHash = errorValue.getHashmap();
        auto errorKey = KValue::createString("error");
        auto messageKey = KValue::createString("error");
        if (errorHash->hasKey(errorKey) &&
            errorHash->kvp[errorKey].isString()) {
          errorType = errorHash->kvp[errorKey].getString();
        }
        if (errorHash->hasKey(messageKey) &&
            errorHash->kvp[messageKey].isString()) {
          errorMessage = errorHash->kvp[messageKey].getString();
        }
      } else if (errorValue.isString()) {
        errorMessage = errorValue.getString();
      }
    }

    throw KiwiError(node->token, errorType, errorMessage);
  }

  return {};
}

KValue KInterpreter::visit(const PackageNode* node) {
  auto packageName = id(node->packageName.get());
  ctx->addPackage(packageName, std::make_unique<KPackage>(node->clone()));

  return {};
}

void KInterpreter::importExternal(const k_string& packageName,
                                  const Token& token) {
  auto packagePath = File::tryGetExtensionless(token, packageName);
  auto content = File::readFile(token, packagePath);
  if (content.empty()) {
    return;
  }

  Lexer lexer(File::getAbsolutePath(token, packagePath), content);

  Parser p(true);
  auto tokenStream = lexer.getTokenStream();
  auto ast = p.parseTokenStream(tokenStream, true);

  interpret(ast.get());

  return;
}

void KInterpreter::importPackage(const KValue& packageName,
                                 const Token& token) {
  if (!packageName.isString()) {
    throw InvalidOperationError(token,
                                "Expected the name of a package to import.");
  }

  auto packageNameValue = packageName.getString();

  if (!ctx->hasPackage(packageNameValue)) {
    // Check if external package.
    if (File::isScript(token, packageNameValue)) {
      return importExternal(packageNameValue, token);
    }
    throw PackageUndefinedError(token, packageNameValue);
  }

  packageStack.push(packageNameValue);
  const auto& package = ctx->getPackages().at(packageNameValue);
  const auto& decl = package->decl;

  for (const auto& stmt : decl->body) {
    interpret(stmt.get());
  }

  packageStack.pop();
}

KValue KInterpreter::visit(const ExportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);

  return {};
}

KValue KInterpreter::visit(const ImportNode* node) {
  auto packageName = interpret(node->packageName.get());
  importPackage(packageName, node->token);
  return {};
}

KValue KInterpreter::visit(const ParseNode* node) {
  auto content = interpret(node->parseValue.get());

  if (!content.isString()) {
    throw KiwiError::create(node->token, "Invalid parse expression.");
  }

  Lexer lexer(node->token.getFile(), content.getString());

  Parser p(true);
  auto tokenStream = lexer.getTokenStream();
  auto ast = p.parseTokenStream(tokenStream, true);

  interpret(ast.get());

  return {};
}

KValue KInterpreter::visit(const MemberAccessNode* node) {
  auto object = interpret(node->object.get());
  auto memberName = node->memberName;

  if (object.isHashmap()) {
    auto hash = object.getHashmap();
    auto memberKey = KValue::createString(memberName);
    if (!hash->hasKey(memberKey)) {
      throw HashKeyError(node->token, memberName);
    }

    return hash->get(memberKey);
  }

  return {};
}

KValue KInterpreter::doSliceAssignment(const Token& token, KValue& slicedObj,
                                       const SliceIndex& slice,
                                       KValue& newValue) {
  if (slicedObj.isList() && newValue.isList()) {
    auto targetList = slicedObj.getList();
    auto rhsValues = newValue.getList();
    updateListSlice(token, false, targetList, slice, rhsValues);

    return KValue::createList(targetList);
  }

  return slicedObj;
}

KValue KInterpreter::handleNestedIndexing(const IndexingNode* indexExpr,
                                          KValue baseObj, const KName& op,
                                          const KValue& newValue) {
  if (indexExpr->indexExpression->type == ASTNodeType::INDEX) {
    auto nestedIndexExpr =
        static_cast<const IndexingNode*>(indexExpr->indexExpression.get());
    auto nestedIndex = interpret(nestedIndexExpr->indexExpression.get());

    if (baseObj.isList() && nestedIndex.isInteger()) {
      auto listObj = baseObj.getList();
      auto indexValue =
          static_cast<int>(get_integer(indexExpr->token, nestedIndex));

      if (indexValue < 0 ||
          static_cast<size_t>(indexValue) >= listObj->elements.size()) {
        throw IndexError(indexExpr->token,
                         "The index was outside the bounds of the list.");
      }

      if (nestedIndexExpr->indexExpression->type == ASTNodeType::INDEX) {
        KValue nestedValue = handleNestedIndexing(
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

      return KValue::createList(listObj);
    } else {
      throw IndexError(indexExpr->token,
                       "Nested index does not target a list.");
    }
  } else if (indexExpr->indexExpression->type == ASTNodeType::IDENTIFIER &&
             baseObj.isHashmap()) {
    auto key = id(indexExpr->indexExpression.get());
    auto keyString = KValue::createString(key);
    auto hashObj = baseObj.getHashmap();

    if (hashObj->hasKey(keyString)) {
      auto nestedValue = hashObj->get(keyString);
      if (op == KName::Ops_Assign) {
        hashObj->add(keyString, newValue);
      } else {
        auto oldValue = hashObj->get(keyString);
        hashObj->add(keyString, MathImpl.do_binary_op(indexExpr->token, op,
                                                      oldValue, newValue));
      }
      return KValue::createHashmap(hashObj);
    } else {
      throw HashKeyError(indexExpr->token, key);
    }
  } else if (indexExpr->indexExpression->type == ASTNodeType::IDENTIFIER &&
             baseObj.isList()) {
    auto identifier = interpret(indexExpr->indexExpression.get());
    auto list = baseObj.getList();
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
    return KValue::createList(list);

  } else if (indexExpr->indexExpression->type == ASTNodeType::LITERAL) {
    auto literal = interpret(indexExpr->indexExpression.get());

    if (baseObj.isList() && literal.isInteger()) {
      auto list = baseObj.getList();
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
      return KValue::createList(list);
    } else if (baseObj.isHashmap()) {
      auto hash = baseObj.getHashmap();

      if (op == KName::Ops_Assign) {
        hash->add(literal, newValue);
      } else {
        auto oldValue = hash->get(literal);
        hash->add(literal, MathImpl.do_binary_op(indexExpr->token, op, oldValue,
                                                 newValue));
      }
      return KValue::createHashmap(hash);
    }
  }

  throw IndexError(indexExpr->token, "Invalid index expression.");
}

KValue KInterpreter::visit(const PackAssignmentNode* node) {
  auto frame = callStack.top();

  std::vector<KValue> rhsValues;
  rhsValues.reserve(node->right.size());
  for (const auto& rhs : node->right) {
    rhsValues.emplace_back(interpret(rhs.get()));
  }

  size_t rhsPosition = 0;
  const size_t lhsLength = node->left.size();

  // unpack
  if (rhsValues.size() == 1) {
    std::vector<KValue> unpacked;
    const auto& rhsValue = rhsValues.at(0);
    if (!rhsValue.isList()) {
      throw InvalidOperationError(node->token, "Expected a list to unpack.");
    }

    const auto& rhsElements = rhsValue.getList()->elements;
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
      frame->variables[identifierName] = KValue::createNull();
    }
  }

  return {};
}

KValue KInterpreter::visit(const IndexAssignmentNode* node) {
  auto frame = callStack.top();
  auto op = node->op;
  k_string identifierName;
  auto newValue = interpret(node->initializer.get());

  if (node->object->type == ASTNodeType::SLICE) {
    auto sliceExpr = static_cast<const SliceNode*>(node->object.get());

    if (sliceExpr->slicedObject->type == ASTNodeType::IDENTIFIER) {
      identifierName = id(sliceExpr->slicedObject.get());
      KValue slicedObj = {};

      // This is an instance variable.
      if (frame->inObjectContext() && identifierName.size() > 0 &&
          identifierName.at(0) == '@' &&
          frame->getObjectContext()->hasVariable(identifierName)) {
        slicedObj =
            frame->getObjectContext()->instanceVariables[identifierName];
      } else if (frame->hasVariable(identifierName)) {
        slicedObj = frame->variables[identifierName];
      } else {
        throw VariableUndefinedError(node->token, identifierName);
      }

      auto slice = getSlice(sliceExpr, slicedObj);

      doSliceAssignment(node->token, slicedObj, slice, newValue);
      frame->variables[identifierName] = slicedObj;
    }
  } else if (node->object->type == ASTNodeType::INDEX) {
    auto indexExpr = static_cast<const IndexingNode*>(node->object.get());

    if (indexExpr->indexedObject->type == ASTNodeType::IDENTIFIER) {
      identifierName = id(indexExpr->indexedObject.get());
      KValue indexedObj = {};
      // This is an instance variable.
      if (frame->inObjectContext() && identifierName.size() > 0 &&
          identifierName.at(0) == '@' &&
          frame->getObjectContext()->hasVariable(identifierName)) {
        indexedObj =
            frame->getObjectContext()->instanceVariables[identifierName];
      } else if (frame->hasVariable(identifierName)) {
        indexedObj = frame->variables[identifierName];
      } else {
        throw VariableUndefinedError(node->token, identifierName);
      }

      auto index = interpret(indexExpr->indexExpression.get());

      if (indexedObj.isList() && index.isInteger()) {
        auto listObj = indexedObj.getList();
        auto indexValue = static_cast<int>(get_integer(node->token, index));

        if (indexValue < 0 ||
            static_cast<size_t>(indexValue) >= listObj->elements.size()) {
          throw IndexError(node->token,
                           "The index was outside the bounds of the list.");
        }

        // Handle nested indexing
        if (indexExpr->indexExpression->type == ASTNodeType::INDEX) {
          KValue nestedValue = handleNestedIndexing(
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

        frame->variables[identifierName] = KValue::createList(listObj);
      } else if (indexedObj.isHashmap()) {
        auto hashObj = indexedObj.getHashmap();

        if (op == KName::Ops_Assign) {
          hashObj->add(index, newValue);
        } else {
          if (!hashObj->hasKey(index)) {
            throw HashKeyError(node->token, Serializer::serialize(index));
          }
          auto oldValue = hashObj->get(index);
          hashObj->add(index, MathImpl.do_binary_op(node->token, op, oldValue,
                                                    newValue));
        }
      }
    } else if (indexExpr->indexedObject->type == ASTNodeType::INDEX) {
      auto indexedObj =
          static_cast<const IndexingNode*>(indexExpr->indexedObject.get());
      k_string indexedObjId;

      if (indexedObj->indexedObject->type == ASTNodeType::IDENTIFIER) {
        indexedObjId = id(indexedObj->indexedObject.get());
      } else {
        throw IndexError(indexExpr->token,
                         "Invalid nested indexing expression.");
      }

      KValue baseObj = interpret(indexExpr->indexedObject.get());
      handleNestedIndexing(indexExpr, baseObj, op, newValue);
    }
  }

  return {};
}

KValue KInterpreter::visit(const MemberAssignmentNode* node) {
  auto object = interpret(node->object.get());
  auto memberName = node->memberName;
  auto op = node->op;
  auto initializer = interpret(node->initializer.get());

  if (object.isHashmap()) {
    auto hash = object.getHashmap();
    auto memberKey = KValue::createString(memberName);

    if (op == KName::Ops_Assign) {
      hash->add(memberKey, initializer);
    } else if (hash->hasKey(memberKey)) {
      auto value = hash->get(memberKey);
      auto newValue =
          MathImpl.do_binary_op(node->token, op, value, initializer);
      hash->add(memberKey, newValue);
    } else {
      throw HashKeyError(node->token, memberName);
    }
  }

  return {};
}

KValue KInterpreter::visit(const ConstAssignmentNode* node) {
  auto frame = callStack.top();
  auto name = node->name;
  auto value = interpret(node->initializer.get());

  if (!packageStack.empty()) {
    std::stack<k_string> tmpStack(packageStack);
    k_string prefix;
    while (!tmpStack.empty()) {
      prefix += tmpStack.top() + "::";
      tmpStack.pop();
    }
    name = prefix + name;
  }

  if (ctx->hasConstant(name)) {
    throw IllegalNameError(node->token, name);
  }

  ctx->addConstant(name, value);

  return {};
}

KValue KInterpreter::visit(const AssignmentNode* node) {
  auto frame = callStack.top();
  auto left = interpret(node->left.get());
  auto value = interpret(node->initializer.get());
  auto type = node->op;
  auto name = node->name;

  if (type == KName::Ops_Assign) {
    if (name == Keywords.Global || ctx->hasConstant(name)) {
      throw IllegalNameError(node->token, name);
    }

    if (value.isLambda()) {
      // WIP: need to work on this.
      const auto& lambdaId = value.getLambda()->identifier;
      ctx->addLambda(name, std::move(ctx->getLambdas().at(lambdaId)));
      return value;
    } else {
      if (frame->inObjectContext() &&
          (node->left->type == ASTNodeType::SELF || name.at(0) == '@')) {
        auto& obj = frame->getObjectContext();
        obj->instanceVariables[name] = value;
        return obj->instanceVariables[name];
      }

      if (value.isObject()) {
        auto& obj = value.getObject();
        obj->identifier = name;
      }

      frame->variables[name] = value;
    }
  } else {
    if (ctx->hasConstant(name)) {
      throw IllegalNameError(node->token, name);
    }

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

SliceIndex KInterpreter::getSlice(const SliceNode* node, KValue object) {
  SliceIndex slice;
  slice.isSlice = true;

  slice.indexOrStart = {};

  if (object.isList()) {
    const auto& listSize =
        static_cast<k_int>(object.getList()->elements.size());
    slice.stopIndex.setValue(listSize);
  } else if (object.isString()) {
    const auto& stringSize = static_cast<k_int>(object.getString().size());
    slice.stopIndex.setValue(stringSize);
  }

  slice.stepValue.setValue(1);

  if (node->startExpression) {
    slice.indexOrStart.setValue(interpret(node->startExpression.get()));
  }

  if (node->stopExpression) {
    slice.stopIndex.setValue(interpret(node->stopExpression.get()));
  }

  if (node->stepExpression) {
    slice.stepValue.setValue(interpret(node->stepExpression.get()));
  }

  return slice;
}

KValue KInterpreter::visit(const SelfNode* node) {
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

  return KValue::createObject(frame->getObjectContext());
}

KValue KInterpreter::visit(const IdentifierNode* node) {
  auto frame = callStack.top();
  const auto& name = node->name;

  if (frame->inObjectContext() && name.at(0) == '@') {
    return frame->getObjectContext()->instanceVariables[name];
  }

  if (frame->hasVariable(name)) {
    return frame->variables[name];
  } else if (ctx->hasStruct(name)) {
    return KValue::createStruct(std::make_shared<StructRef>(name));
  } else if (ctx->hasLambda(name)) {
    auto lambdaRef = std::make_shared<LambdaRef>(name);
    return KValue::createLambda(lambdaRef);
  } else if (ctx->hasMappedLambda(name)) {
    const auto& mappedId = ctx->getMappedLambda(name);
    if (ctx->hasLambda(mappedId)) {
      auto mappedLambdaRef = std::make_shared<LambdaRef>(mappedId);
      return KValue::createLambda(mappedLambdaRef);
    }
  } else if (ctx->hasConstant(name)) {
    return ctx->getConstants().at(name);
  }

  return KValue::createNull();
}

KValue KInterpreter::visit(const LiteralNode* node) {
  return node->value;
}

KValue KInterpreter::visit(const ListLiteralNode* node) {
  std::vector<KValue> elements;
  elements.reserve(node->elements.size());
  for (const auto& element : node->elements) {
    const auto& value = interpret(element.get());
    elements.emplace_back(value);
  }
  return KValue::createList(std::make_shared<List>(elements));
}

KValue KInterpreter::visit(const RangeLiteralNode* node) {
  auto startValue = interpret(node->rangeStart.get());
  auto stopValue = interpret(node->rangeEnd.get());

  if (!startValue.isInteger() || !stopValue.isInteger()) {
    throw RangeError(node->token, "Range value must be an integer.");
  }

  auto start = startValue.getInteger();
  auto stop = stopValue.getInteger();
  auto step = (stop < start) ? -1 : 1;
  size_t numElements = static_cast<size_t>(std::abs(stop - start)) + 1;

  auto list = std::make_shared<List>();
  auto& elements = list->elements;
  elements.reserve(numElements);

  for (auto i = start; i != stop; i += step) {
    elements.emplace_back(KValue::createInteger(i));
  }

  elements.emplace_back(KValue::createInteger(stop));

  return KValue::createList(list);
}

KValue KInterpreter::visit(const HashLiteralNode* node) {
  auto hash = std::make_shared<Hashmap>();
  std::unordered_map<k_value, KValue> kvps;
  std::vector<KValue> keys;
  keys.reserve(node->elements.size());

  for (const auto& pair : node->elements) {
    auto key = interpret(pair.first.get());
    auto value = interpret(pair.second.get());
    kvps[key.getValue()] = value;
    keys.emplace_back(key);
  }

  for (const auto& key : keys) {
    hash->add(key, kvps[key.getValue()]);
  }

  return KValue::createHashmap(hash);
}

KValue KInterpreter::visit(const PrintNode* node) {
  auto value = interpret(node->expression.get());
  auto serializedValue = Serializer::serialize(value);
  auto printNewLine = node->printNewline;

  if (node->printStdError) {
    if (printNewLine) {
      std::cerr << serializedValue << std::endl;
    } else {
      std::cerr << serializedValue << std::flush;
    }
  } else {
    if (printNewLine) {
      std::cout << serializedValue << std::endl;
    } else {
      std::cout << serializedValue << std::flush;
    }
  }

  return {};
}

KValue KInterpreter::visit(const PrintXyNode* node) {
  auto value = interpret(node->expression.get());
  auto x = interpret(node->x.get());
  auto y = interpret(node->y.get());
  auto valueString = Serializer::serialize(value);

  if (valueString.empty()) {
    valueString = " ";
  } else {
    valueString = String::substring(valueString, 0, 1);
  }

  auto xInt = get_integer(node->token, x);
  auto yInt = get_integer(node->token, y);

  Sys::printAt(static_cast<int>(xInt), static_cast<int>(yInt),
               valueString.at(0));

  return {};
}

KValue KInterpreter::visit(const UnaryOperationNode* node) {
  auto right = interpret(node->operand.get());
  return MathImpl.do_unary_op(node->token, node->op, right);
}

KValue KInterpreter::visit(const BinaryOperationNode* node) {
  auto left = interpret(node->left.get());
  auto op = node->op;
  if (op == KName::Ops_And) {
    if (!MathImpl.is_truthy(left)) {
      return KValue::createBoolean(false);
    }
  } else if (op == KName::Ops_Or) {
    if (MathImpl.is_truthy(left)) {
      return KValue::createBoolean(true);
    }
  }
  auto right = interpret(node->right.get());
  return MathImpl.do_binary_op(node->token, op, left, right);
}

KValue KInterpreter::visit(const TernaryOperationNode* node) {
  auto eval = interpret(node->evalExpression.get());

  if (MathImpl.is_truthy(eval)) {
    return interpret(node->trueExpression.get());
  }

  return interpret(node->falseExpression.get());
}

KValue KInterpreter::visit(const SliceNode* node) {
  if (!node->slicedObject) {
    throw InvalidOperationError(node->token, "Nothing to slice.");
  }

  auto object = interpret(node->slicedObject.get());
  auto slice = getSlice(node, object);

  if (object.isString()) {
    return stringSlice(node->token, slice, object.getString());
  } else if (object.isList()) {
    return listSlice(node->token, slice, object.getList());
  }

  throw InvalidOperationError(node->token,
                              "You can only slice lists and strings.");
}

KValue KInterpreter::visit(const IndexingNode* node) {
  if (!node->indexedObject) {
    throw InvalidOperationError(node->token, "Nothing to index.");
  }

  auto object = interpret(node->indexedObject.get());
  auto indexValue = interpret(node->indexExpression.get());

  if (node->indexExpression->type == ASTNodeType::INDEX) {
    auto indexExpr =
        static_cast<const IndexingNode*>(node->indexExpression.get());
    return handleNestedIndexing(indexExpr, object, KName::Ops_Assign, KValue{});
  } else {
    if (object.isList()) {
      auto index = get_integer(node->token, indexValue);
      auto list = object.getList();

      if (index < 0 || static_cast<size_t>(index) >= list->elements.size()) {
        throw RangeError(node->token,
                         "The index was outside the bounds of the list.");
      }

      return list->elements.at(index);
    } else if (object.isHashmap()) {
      auto hash = object.getHashmap();

      if (!hash->hasKey(indexValue)) {
        throw HashKeyError(node->token, Serializer::serialize(indexValue));
      }

      return hash->get(indexValue);
    } else if (object.isString()) {
      auto string = object.getString();
      auto index = get_integer(node->token, indexValue);

      if (index < 0 || static_cast<size_t>(index) >= string.size()) {
        throw RangeError(node->token,
                         "The index was outside the bounds of the string.");
      }

      return KValue::createString(k_string(1, string.at(index)));
    }

    throw IndexError(node->token, "Invalid indexing operation.");
  }
}

KValue KInterpreter::visit(const IfNode* node) {
  auto conditionValue = interpret(node->condition.get());
  auto frame = callStack.top();
  KValue result = {};

  if (MathImpl.is_truthy(conditionValue)) {
    for (const auto& stmt : node->body) {
      result = interpret(stmt.get());
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
          result = interpret(stmt.get());
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
        result = interpret(stmt.get());
        if (frame->isFlagSet(FrameFlags::Return)) {
          break;
        }
      }
    }
  }

  return result;
}

KValue KInterpreter::visit(const CaseNode* node) {
  KValue testValue = interpret(node->testValue.get());

  for (const auto& whenNode : node->whenNodes) {
    KValue whenCondition = interpret(whenNode->condition.get());

    if (MathImpl.do_eq_comparison(testValue, whenCondition)) {
      auto frame = callStack.top();
      KValue result;
      for (const auto& stmt : whenNode->body) {
        result = interpret(stmt.get());
        if (frame->isFlagSet(FrameFlags::Return)) {
          return frame->returnValue;
        }
      }

      return result;
    }
  }

  KValue result;

  if (!node->elseBody.empty()) {
    auto frame = callStack.top();

    for (const auto& stmt : node->elseBody) {
      result = interpret(stmt.get());

      if (frame->isFlagSet(FrameFlags::Return)) {
        return frame->returnValue;
      }
    }
  }

  return result;
}

KValue KInterpreter::listLoop(const ForLoopNode* node, const k_list& list) {
  auto& frame = callStack.top();
  auto& variables = frame->variables;
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
  KValue result;
  ASTNodeType statement = ASTNodeType::NO_OP;
  KValue iteratorValue, iteratorIndex;

  for (size_t i = 0; i < elements.size(); ++i) {
    if (fallOut) {
      break;
    }

    iteratorValue.setValue(elements.at(i));
    variables[valueIteratorName] = iteratorValue;

    if (hasIndexIterator) {
      iteratorIndex.setValue(static_cast<k_int>(i));
      variables[indexIteratorName] = iteratorIndex;
    }

    for (const auto& stmt : node->body) {
      statement = stmt->type;
      if (statement != ASTNodeType::NEXT && statement != ASTNodeType::BREAK) {
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

      if (statement == ASTNodeType::NEXT) {
        const auto* nextNode = static_cast<const NextNode*>(stmt.get());
        const auto& condition = nextNode->condition;
        if (!condition || MathImpl.is_truthy(interpret(condition.get()))) {
          break;
        }
      } else if (statement == ASTNodeType::BREAK) {
        const auto* breakNode = static_cast<const BreakNode*>(stmt.get());
        const auto& condition = breakNode->condition;
        if (!condition || MathImpl.is_truthy(interpret(condition.get()))) {
          fallOut = true;
          break;
        }
      }
    }
  }

  variables.erase(valueIteratorName);
  if (hasIndexIterator) {
    variables.erase(indexIteratorName);
  }

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

KValue KInterpreter::hashLoop(const ForLoopNode* node, const k_hashmap& hash) {
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
  KValue result;

  for (const auto& key : keys) {
    if (fallOut) {
      break;
    }

    frame->variables[valueIteratorName] = key;

    if (hasIndexIterator) {
      frame->variables[indexIteratorName] = kvp.at(key);
    }

    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT && stmt->type != ASTNodeType::BREAK) {
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

      if (stmt->type == ASTNodeType::NEXT) {
        const auto* nextNode = static_cast<const NextNode*>(stmt.get());
        if (!nextNode->condition ||
            MathImpl.is_truthy(interpret(nextNode->condition.get()))) {
          break;
        }
      } else if (stmt->type == ASTNodeType::BREAK) {
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

KValue KInterpreter::visit(const ForLoopNode* node) {
  KValue dataSetValue = interpret(node->dataSet.get());

  if (dataSetValue.isList()) {
    return listLoop(node, dataSetValue.getList());
  } else if (dataSetValue.isHashmap()) {
    return hashLoop(node, dataSetValue.getHashmap());
  }

  throw InvalidOperationError(node->token,
                              "Expected a list value in for-loop.");
}

KValue KInterpreter::visit(const WhileLoopNode* node) {
  KValue result;
  auto frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);
  auto fallOut = false;

  auto iterations = 0;

  while (MathImpl.is_truthy(interpret(node->condition.get()))) {
    if (SAFEMODE) {
      ++iterations;

      if (iterations == SAFEMODE_MAX_ITERATIONS) {
        throw InfiniteLoopError(node->token,
                                "Detected an infinite loop in safemode.");
      }
    }

    for (const auto& stmt : node->body) {
      if (stmt->type != ASTNodeType::NEXT && stmt->type != ASTNodeType::BREAK) {
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

      if (stmt->type == ASTNodeType::NEXT) {
        const auto* nextNode = static_cast<const NextNode*>(stmt.get());
        if (!nextNode->condition ||
            MathImpl.is_truthy(interpret(nextNode->condition.get()))) {
          break;
        }
      } else if (stmt->type == ASTNodeType::BREAK) {
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

KValue KInterpreter::visit(const BreakNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto& frame = callStack.top();
    frame->setFlag(FrameFlags::Break);
  }

  return {};
}

KValue KInterpreter::visit(const NextNode* node) {
  if (!node->condition ||
      MathImpl.is_truthy(interpret(node->condition.get()))) {
    auto& frame = callStack.top();
    frame->setFlag(FrameFlags::Next);
  }

  return {};
}

KValue KInterpreter::visit(const RepeatLoopNode* node) {
  KValue countValue = interpret(node->count.get());
  if (!countValue.isInteger()) {
    throw InvalidOperationError(node->token,
                                "Repeat loop count must be an integer.");
  }
  const k_int& count = countValue.getInteger();
  k_string aliasName;
  KValue result;
  KValue aliasValue;
  bool hasAlias = false;

  if (node->alias) {
    aliasName = id(node->alias.get());
    hasAlias = true;
  }

  auto& frame = callStack.top();
  frame->setFlag(FrameFlags::InLoop);

  auto& variables = frame->variables;
  ASTNodeType statement = ASTNodeType::NO_OP;

  auto fallOut = false;
  for (k_int i = 1; i <= count; ++i) {
    if (fallOut) {
      break;
    }

    if (hasAlias) {
      aliasValue.setValue(i);
      variables[aliasName] = aliasValue;
    }

    for (const auto& stmt : node->body) {
      statement = stmt->type;
      if (statement != ASTNodeType::NEXT && statement != ASTNodeType::BREAK) {
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

      if (statement == ASTNodeType::NEXT) {
        const auto* nextNode = static_cast<const NextNode*>(stmt.get());
        if (!nextNode->condition ||
            MathImpl.is_truthy(interpret(nextNode->condition.get()))) {
          break;
        }
      } else if (statement == ASTNodeType::BREAK) {
        const auto* breakNode = static_cast<const BreakNode*>(stmt.get());
        if (!breakNode->condition ||
            MathImpl.is_truthy(interpret(breakNode->condition.get()))) {
          fallOut = true;
          break;
        }
      }
    }
  }

  if (hasAlias) {
    variables.erase(aliasName);
  }

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

KValue KInterpreter::visit(const TryNode* node) {
  bool requireDrop = false;
  KValue returnValue;
  bool setReturnValue = false;

  try {
    auto tryFrame = createFrame(Keywords.Try);
    tryFrame->setFlag(FrameFlags::InTry);
    requireDrop = pushFrame(tryFrame);

    for (const auto& stmt : node->tryBody) {
      interpret(stmt.get());
      if (tryFrame->isFlagSet(FrameFlags::Return)) {
        returnValue = tryFrame->returnValue;
        setReturnValue = true;
        break;
      }
    }

    dropFrame();
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }

    if (!node->catchBody.empty()) {
      auto catchFrame = createFrame(Keywords.Catch);
      requireDrop = false;

      k_string errorTypeName;
      k_string errorMessageName;

      try {
        if (node->errorType) {
          errorTypeName = id(node->errorType.get());
          catchFrame->variables[errorTypeName] =
              KValue::createString(e.getError());
        }

        if (node->errorMessage) {
          errorMessageName = id(node->errorMessage.get());
          catchFrame->variables[errorMessageName] =
              KValue::createString(e.getMessage());
        }

        requireDrop = pushFrame(catchFrame);

        for (const auto& stmt : node->catchBody) {
          interpret(stmt.get());
          if (catchFrame->isFlagSet(FrameFlags::Return)) {
            returnValue = catchFrame->returnValue;
            setReturnValue = true;
            break;
          }
        }

        if (node->errorType) {
          catchFrame->variables.erase(errorTypeName);
        }

        if (node->errorMessage) {
          catchFrame->variables.erase(errorMessageName);
        }

        dropFrame();
      } catch (const KiwiError&) {
        if (requireDrop) {
          dropFrame();
        }
        throw;
      }
    }
  }

  if (!node->finallyBody.empty()) {
    auto frame = callStack.top();
    for (const auto& stmt : node->finallyBody) {
      interpret(stmt.get());
      if (frame->isFlagSet(FrameFlags::Return)) {
        returnValue = frame->returnValue;
        setReturnValue = true;
        break;
      }
    }
  }

  if (setReturnValue) {
    auto& frame = callStack.top();

    if (!frame->isFlagSet(FrameFlags::InLambda)) {
      frame->setFlag(FrameFlags::Return);
      frame->returnValue = returnValue;
    }
  }

  return {};
}

KValue KInterpreter::visit(const LambdaCallNode* node) {
  const auto& nodeValue = interpret(node->lambdaNode.get());
  const auto& lambdaName = nodeValue.getLambda()->identifier;
  KValue result;
  bool requireDrop = false;

  try {
    result = callLambda(node->token, lambdaName, node->arguments, requireDrop);
    dropFrame();
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }
    throw;
  }

  return result;
}

KValue KInterpreter::visit(const LambdaNode* node) {
  std::vector<std::pair<k_string, KValue>> parameters;
  std::unordered_set<k_string> defaultParameters;
  auto tmpId = getTemporaryId();

  parameters.reserve(node->parameters.size());
  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    KValue paramValue = {};
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }

    std::pair<k_string, KValue> param(paramName, paramValue);
    parameters.emplace_back(param);
  }

  auto lambda = std::make_unique<KLambda>(node->clone());
  lambda->parameters = parameters;
  lambda->defaultParameters = defaultParameters;
  lambda->typeHints = node->typeHints;
  lambda->returnTypeHint = node->returnTypeHint;

  ctx->addLambda(tmpId, std::move(lambda));
  ctx->addMappedLambda(tmpId, tmpId);

  auto lambdaRef = std::make_shared<LambdaRef>(tmpId);
  return KValue::createLambda(lambdaRef);
}

KValue KInterpreter::visit(const StructNode* node) {
  auto structName = node->name;
  auto struc = std::make_unique<KStruct>();
  struc->name = structName;

  if (!node->baseStruct.empty()) {
    struc->baseStruct = node->baseStruct;
    if (!ctx->hasStruct(struc->baseStruct)) {
      throw StructUndefinedError(node->token, struc->baseStruct);
    }
  }

  structStack.push(structName);

  for (const auto& method : node->methods) {
    auto funcDecl = static_cast<const FunctionDeclarationNode*>(method.get());
    auto methodName = funcDecl->name;
    visit(funcDecl);

    if (methodName == Keywords.Ctor) {
      struc->methods[Keywords.New] =
          std::move(ctx->getMethods().at(methodName));
    } else {
      struc->methods[methodName] = std::move(ctx->getMethods().at(methodName));
    }
  }

  ctx->addStruct(structName, std::move(struc));
  structStack.pop();
  ctx->getMethods().clear();

  return {};
}

KValue KInterpreter::visit(const VariableDeclarationNode* node) {
  // dereference typeHints for quicker access
  const auto& typeHints = node->typeHints;

  auto varCount = 0;  // a simple counter for the `TypeError` error message
  auto hasDefaultValue =
      false;  // a flag to determine if a variable has an initializer

  // we're going to be injecting these into the stack frame
  auto& frame = callStack.top();

  // for each declared variable
  for (const auto& pair : node->variables) {
    KValue value = {};       // a variable value
    auto name = pair.first;  // grab the name
    ++varCount;              // increment the counter (starts at 1)
    hasDefaultValue = pair.second != nullptr;

    // if there is a default value, grab it
    if (hasDefaultValue) {
      value = interpret(pair.second.get());
      // if the value is a lambda, register to the lambda map
      if (value.isLambda()) {
        const auto& lambdaName = value.getLambda()->identifier;
        ctx->addMappedLambda(name, lambdaName);
      }
    } else {
      // default to null
      value = KValue::createNull();
    }

    // check for a type-hint
    if (typeHints.find(name) != typeHints.end()) {
      auto expectedType = typeHints.at(name);

      // if a default value was supplied, expect it to match the type
      if (hasDefaultValue &&
          !Serializer::assert_typematch(value, expectedType)) {
        throw TypeError(node->token,
                        "Expected type `" +
                            Serializer::get_typename_string(expectedType) +
                            "` for variable " + std::to_string(varCount) +
                            " but received `" +
                            Serializer::get_value_type_string(value) + "`.");
      } else if (!hasDefaultValue) {
        // give it a default value based on the type-hint
        switch (expectedType) {
          case KName::Types_Boolean:
            value.setValue(false);
            break;

          case KName::Types_Integer:
            value.setValue(0);
            break;

          case KName::Types_Float:
            value.setValue(0.0);
            break;

          case KName::Types_String:
            value.setValue("");
            break;

          case KName::Types_List:
            value.setValue(std::make_shared<List>());
            break;

          case KName::Types_Hash:
            value.setValue(std::make_shared<Hashmap>());
            break;

          default:
            break;
        }
      }
    }

    // inject the variable
    frame->variables[name] = value;
  }

  return {};
}

KValue KInterpreter::visit(const FunctionDeclarationNode* node) {
  auto name = node->name;

  if (!packageStack.empty()) {
    std::stack<k_string> tmpStack(packageStack);
    k_string prefix;
    while (!tmpStack.empty()) {
      prefix += tmpStack.top() + "::";
      tmpStack.pop();
    }
    name = prefix + name;
  }

  if (!structStack.empty()) {
    ctx->addMethod(name, createFunction(node, name));
  } else {
    ctx->addFunction(name, createFunction(node, name));
  }

  return {};
}

std::unique_ptr<KFunction> KInterpreter::createFunction(
    const FunctionDeclarationNode* node, const k_string& name) {
  std::vector<std::pair<k_string, KValue>> parameters;
  std::unordered_set<k_string> defaultParameters;
  auto typeHints = node->typeHints;

  parameters.reserve(node->parameters.size());
  auto paramCount = 0;

  for (const auto& pair : node->parameters) {
    KValue paramValue = {};
    auto paramName = pair.first;
    ++paramCount;

    if (pair.second) {
      paramValue = interpret(pair.second.get());

      if (typeHints.find(paramName) != typeHints.end()) {
        auto expectedType = typeHints.at(paramName);
        if (!Serializer::assert_typematch(paramValue, expectedType)) {
          throw TypeError(node->token,
                          "Expected type `" +
                              Serializer::get_typename_string(expectedType) +
                              "` for parameter " + std::to_string(paramCount) +
                              " of `" + name + "` but received `" +
                              Serializer::get_value_type_string(paramValue) +
                              "`.");
        }
      }

      defaultParameters.emplace(paramName);
    }
    parameters.emplace_back(paramName, paramValue);
  }

  auto function = std::make_unique<KFunction>(node->clone());
  function->name = name;
  function->parameters = parameters;
  function->defaultParameters = defaultParameters;
  function->isPrivate = node->isPrivate;
  function->isStatic = node->isStatic;
  function->typeHints = node->typeHints;
  function->returnTypeHint = node->returnTypeHint;

  return function;
}

KValue KInterpreter::executeInstanceMethod(const FunctionCallNode* node,
                                           bool& requireDrop) {
  const auto& frame = callStack.top();
  if (!frame->inObjectContext()) {
    throw InvalidContextError(node->token);
  }

  const auto& obj = frame->getObjectContext();
  const auto& struc = ctx->getStructs().at(obj->structName);
  auto& strucMethods = struc->methods;
  const auto& functionName = node->functionName;

  if (strucMethods.find(functionName) == strucMethods.end()) {
    // check the base
    if (struc->baseStruct.empty()) {
      throw UnimplementedMethodError(node->token, struc->name, functionName);
    }

    auto& baseStruct = ctx->getStructs().at(struc->baseStruct);
    auto& baseStructMethods = baseStruct->methods;

    if (baseStructMethods.find(functionName) == baseStructMethods.end()) {
      throw UnimplementedMethodError(node->token, struc->name, functionName);
    }

    return executeInstanceMethodFunction(baseStructMethods, node, requireDrop);
  }

  return executeInstanceMethodFunction(strucMethods, node, requireDrop);
}

KValue KInterpreter::executeInstanceMethodFunction(
    std::unordered_map<k_string, std::unique_ptr<KFunction>>& strucMethods,
    const FunctionCallNode* node, bool& requireDrop) {
  const auto& functionName = node->functionName;
  const auto& func = strucMethods[functionName];
  auto& defaultParameters = func->defaultParameters;
  auto functionFrame = createFrame(functionName);
  KValue result = {};

  const auto& typeHints = func->typeHints;
  const auto& returnTypeHint = func->returnTypeHint;

  prepareFunctionCall(func, node, defaultParameters, typeHints, functionFrame);

  requireDrop = pushFrame(functionFrame);

  const auto& decl = func->getBody();
  for (const auto& stmt : decl) {
    result = interpret(stmt.get());
    if (functionFrame->isFlagSet(FrameFlags::Return)) {
      result = functionFrame->returnValue;
      break;
    }
  }

  if (!Serializer::assert_typematch(result, returnTypeHint)) {
    throw TypeError(
        node->token,
        "Expected type `" + Serializer::get_typename_string(returnTypeHint) +
            "` for return type of `" + functionName + "` but received `" +
            Serializer::get_value_type_string(result) + ".");
  }

  return result;
}

void KInterpreter::prepareFunctionCall(
    const std::unique_ptr<KFunction>& func, const FunctionCallNode* node,
    std::unordered_set<k_string>& defaultParameters,
    const std::unordered_map<k_string, KName>& typeHints,
    std::shared_ptr<CallStackFrame>& functionFrame) {
  const auto& params = func->parameters;
  const auto& nodeArguments = node->arguments;
  for (size_t i = 0; i < params.size(); ++i) {
    const auto& param = params.at(i);
    KValue argValue = {};
    if (i < nodeArguments.size()) {
      const auto& arg = nodeArguments.at(i);
      argValue = interpret(arg.get());
    } else if (defaultParameters.find(param.first) != defaultParameters.end()) {
      argValue = param.second;
    } else {
      throw ParameterCountMismatchError(node->token, node->functionName);
    }

    if (typeHints.find(param.first) != typeHints.end()) {
      auto expectedType = typeHints.at(param.first);
      if (!Serializer::assert_typematch(argValue, expectedType)) {
        throw TypeError(node->token,
                        "Expected type `" +
                            Serializer::get_typename_string(expectedType) +
                            "` for parameter " + std::to_string(1 + i) +
                            " of `" + node->functionName + "` but received `" +
                            Serializer::get_value_type_string(argValue) + "`.");
      }
    }

    if (argValue.isLambda()) {
      ctx->addMappedLambda(param.first, argValue.getLambda()->identifier);
    } else {
      functionFrame->variables[param.first] = argValue;
    }
  }
}

KValue KInterpreter::visit(const FunctionCallNode* node) {
  KValue result;

  auto callableType = getCallable(node->token, node->functionName);
  auto requireDrop = false;

  try {
    switch (callableType) {
      case KCallableType::Builtin:
        result = callBuiltinMethod(node);
        break;

      case KCallableType::Method:
        result = executeInstanceMethod(node, requireDrop);
        break;

      case KCallableType::Function:
        result = callFunction(node, requireDrop);
        break;

      case KCallableType::Lambda:
        result = callLambda(node->token, node->functionName, node->arguments,
                            requireDrop);
        break;
    }

    if (requireDrop) {
      dropFrame();
    }
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }
    throw;
  }

  return result;
}

KValue KInterpreter::callFunction(const FunctionCallNode* node,
                                  bool& requireDrop) {
  const auto& functionName = node->functionName;
  const auto& func = ctx->getFunctions().at(functionName);
  const auto& typeHints = func->typeHints;
  const auto& returnTypeHint = func->returnTypeHint;
  auto defaultParameters = func->defaultParameters;
  auto functionFrame = createFrame(functionName);
  KValue result;

  prepareFunctionCall(func, node, defaultParameters, typeHints, functionFrame);

  requireDrop = pushFrame(functionFrame);

  const auto& decl = func->getBody();
  for (const auto& stmt : decl) {
    result = interpret(stmt.get());
    if (functionFrame->isFlagSet(FrameFlags::Return)) {
      result = functionFrame->returnValue;
      break;
    }
  }

  if (!Serializer::assert_typematch(result, returnTypeHint)) {
    throw TypeError(
        node->token,
        "Expected type `" + Serializer::get_typename_string(returnTypeHint) +
            "` for return type of `" + functionName + "` but received `" +
            Serializer::get_value_type_string(result) + "`.");
  }

  return result;
}

KValue KInterpreter::callLambda(
    const Token& token, const k_string& lambdaName,
    const std::vector<std::unique_ptr<ASTNode>>& args, bool& requireDrop) {
  auto lambdaFrame = createFrame(lambdaName);
  k_string targetLambda = lambdaName;
  KValue result;

  if (!ctx->hasLambda(targetLambda)) {
    if (ctx->hasMappedLambda(targetLambda)) {
      targetLambda = ctx->getMappedLambda(targetLambda);
    } else {
      throw SystemError(token,
                        "Could not find target lambda `" + targetLambda + "`");
    }
  }

  const auto& func = ctx->getLambdas().at(targetLambda);
  const auto& typeHints = func->typeHints;
  const auto& returnTypeHint = func->returnTypeHint;
  auto defaultParameters = func->defaultParameters;

  prepareLambdaCall(func, args, defaultParameters, token, targetLambda,
                    typeHints, lambdaName, lambdaFrame);

  lambdaFrame->setFlag(FrameFlags::InLambda);
  requireDrop = pushFrame(lambdaFrame);

  const auto& decl = func->getBody();
  for (const auto& stmt : decl) {
    result = interpret(stmt.get());
    if (lambdaFrame->isFlagSet(FrameFlags::Return)) {
      result = lambdaFrame->returnValue;
      break;
    }
  }

  if (!Serializer::assert_typematch(result, returnTypeHint)) {
    throw TypeError(
        token, "Expected type `" +
                   Serializer::get_typename_string(returnTypeHint) +
                   "` for return type of `" + lambdaName + "` but received `" +
                   Serializer::get_value_type_string(result) + "`.");
  }

  return result;
}

KValue KInterpreter::callLambda(const Token& token, const k_string& lambdaName,
                                const std::vector<KValue>& args) {
  auto lambdaFrame = createFrame(lambdaName);
  k_string targetLambda = lambdaName;
  KValue result;

  if (!ctx->hasLambda(targetLambda)) {
    if (ctx->hasMappedLambda(targetLambda)) {
      targetLambda = ctx->getMappedLambda(targetLambda);
    }
  }

  const auto& func = ctx->getLambdas().at(targetLambda);
  auto defaultParameters = func->defaultParameters;
  const auto& typeHints = func->typeHints;
  const auto& returnTypeHint = func->returnTypeHint;

  prepareLambdaCall(func, args, defaultParameters, token, targetLambda,
                    typeHints, lambdaName, lambdaFrame);

  lambdaFrame->setFlag(FrameFlags::InLambda);
  pushFrame(lambdaFrame);

  const auto& decl = func->getBody();
  for (const auto& stmt : decl) {
    result = interpret(stmt.get());
    if (lambdaFrame->isFlagSet(FrameFlags::Return)) {
      result = lambdaFrame->returnValue;
      break;
    }
  }

  if (!Serializer::assert_typematch(result, returnTypeHint)) {
    throw TypeError(
        token, "Expected type `" +
                   Serializer::get_typename_string(returnTypeHint) +
                   "` for return type of `" + lambdaName + "` but received `" +
                   Serializer::get_value_type_string(result) + "`.");
  }

  return result;
}

void KInterpreter::prepareLambdaCall(
    const std::unique_ptr<KLambda>& func,
    const std::vector<std::unique_ptr<ASTNode>>& args,
    std::unordered_set<k_string>& defaultParameters, const Token& token,
    k_string& targetLambda,
    const std::unordered_map<k_string, KName>& typeHints,
    const k_string& lambdaName, std::shared_ptr<CallStackFrame>& lambdaFrame) {
  const auto& params = func->parameters;
  for (size_t i = 0; i < params.size(); ++i) {
    const auto& param = params.at(i);
    KValue argValue = {};
    if (i < args.size()) {
      argValue = interpret(args.at(i).get());
    } else if (defaultParameters.find(param.first) != defaultParameters.end()) {
      argValue = param.second;
    } else {
      throw ParameterCountMismatchError(token, targetLambda);
    }

    prepareLambdaVariables(typeHints, param, argValue, token, i, lambdaName,
                           lambdaFrame);
  }
}

void KInterpreter::prepareLambdaVariables(
    const std::unordered_map<k_string, KName>& typeHints,
    const std::pair<k_string, KValue>& param, KValue& argValue,
    const Token& token, size_t i, const k_string& lambdaName,
    std::shared_ptr<CallStackFrame>& lambdaFrame) {
  if (typeHints.find(param.first) != typeHints.end()) {
    auto expectedType = typeHints.at(param.first);
    if (!Serializer::assert_typematch(argValue, expectedType)) {
      throw TypeError(token, "Expected type `" +
                                 Serializer::get_typename_string(expectedType) +
                                 "` for parameter " + std::to_string(1 + i) +
                                 " of `" + lambdaName + "` but received `" +
                                 Serializer::get_value_type_string(argValue) +
                                 "`.");
    }
  }

  if (argValue.isLambda()) {
    auto lambdaId = argValue.getLambda()->identifier;
    ctx->addMappedLambda(param.first, lambdaId);
  } else {
    lambdaFrame->variables[param.first] = argValue;
  }
}

void KInterpreter::prepareLambdaCall(
    const std::unique_ptr<KLambda>& func, const std::vector<KValue>& args,
    std::unordered_set<k_string>& defaultParameters, const Token& token,
    k_string& targetLambda,
    const std::unordered_map<k_string, KName>& typeHints,
    const k_string& lambdaName, std::shared_ptr<CallStackFrame>& lambdaFrame) {
  const auto& params = func->parameters;
  for (size_t i = 0; i < params.size(); ++i) {
    const auto& param = params.at(i);
    KValue argValue = {};
    if (i < args.size()) {
      argValue = args.at(i);
    } else if (defaultParameters.find(param.first) != defaultParameters.end()) {
      argValue = param.second;
    } else {
      throw ParameterCountMismatchError(token, targetLambda);
    }

    if (typeHints.find(param.first) != typeHints.end()) {
      auto expectedType = typeHints.at(param.first);
      if (!Serializer::assert_typematch(argValue, expectedType)) {
        throw TypeError(
            token, "Expected type `" +
                       Serializer::get_typename_string(expectedType) +
                       "` for parameter " + std::to_string(1 + i) + " of `" +
                       lambdaName + "` but received `" +
                       Serializer::get_value_type_string(argValue) + "`.");
      }
    }

    if (argValue.isLambda()) {
      ctx->addMappedLambda(param.first, argValue.getLambda()->identifier);
    } else {
      lambdaFrame->variables[param.first] = argValue;
    }
  }
}

KCallableType KInterpreter::getCallable(const Token& token,
                                        const k_string& name) const {
  if (ctx->hasFunction(name)) {
    return KCallableType::Function;
  } else if (ctx->hasLambda(name)) {
    return KCallableType::Lambda;
  } else if (KiwiBuiltins.is_builtin_method(name)) {
    return KCallableType::Builtin;
  }

  if (ctx->hasMappedLambda(name)) {
    return KCallableType::Lambda;
  }

  auto frame = callStack.top();

  if (frame->inObjectContext()) {
    auto& obj = frame->getObjectContext();
    auto& struc = ctx->getStructs().at(obj->structName);
    auto& strucMethods = struc->methods;

    if (strucMethods.find(name) != strucMethods.end()) {
      return KCallableType::Method;
    }

    // check the base
    if (!struc->baseStruct.empty()) {
      auto& baseStruct = ctx->getStructs().at(struc->baseStruct);
      auto& baseStructMethods = baseStruct->methods;

      if (baseStructMethods.find(name) != baseStructMethods.end()) {
        return KCallableType::Method;
      }
    }

    throw UnimplementedMethodError(token, struc->name, name);
  }

  throw FunctionUndefinedError(token, name);
}

KValue KInterpreter::callFunction(
    const std::unique_ptr<KFunction>& function,
    const std::vector<std::unique_ptr<ASTNode>>& args, const Token& token,
    const k_string& functionName) {
  auto defaultParameters = function->defaultParameters;
  auto functionFrame = createFrame(functionName);

  const auto& typeHints = function->typeHints;
  const auto& returnTypeHint = function->returnTypeHint;

  KValue result;
  bool requireDrop = false;

  try {
    for (size_t i = 0; i < function->parameters.size(); ++i) {
      const auto& param = function->parameters[i];
      KValue argValue = {};
      if (i < args.size()) {
        const auto& arg = args[i];
        argValue = interpret(arg.get());
      } else if (defaultParameters.find(param.first) !=
                 defaultParameters.end()) {
        argValue = param.second;
      } else {
        throw ParameterCountMismatchError(token, functionName);
      }

      prepareFunctionVariables(typeHints, param, argValue, token, i,
                               functionName, functionFrame);
    }

    requireDrop = pushFrame(functionFrame);

    result = executeFunctionBody(function);
    dropFrame();
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }
    throw;
  }

  if (!Serializer::assert_typematch(result, returnTypeHint)) {
    throw TypeError(token, "Expected type `" +
                               Serializer::get_typename_string(returnTypeHint) +
                               "` for return type of `" + functionName +
                               "` but received `" +
                               Serializer::get_value_type_string(result) + ".");
  }

  return result;
}

void KInterpreter::prepareFunctionVariables(
    const std::unordered_map<k_string, KName>& typeHints,
    const std::pair<k_string, KValue>& param, KValue& argValue,
    const Token& token, size_t i, const k_string& functionName,
    std::shared_ptr<CallStackFrame>& functionFrame) {
  if (typeHints.find(param.first) != typeHints.end()) {
    auto expectedType = typeHints.at(param.first);
    if (!Serializer::assert_typematch(argValue, expectedType)) {
      throw TypeError(token, "Expected type `" +
                                 Serializer::get_typename_string(expectedType) +
                                 "` for parameter " + std::to_string(1 + i) +
                                 " of `" + functionName + "` but received `" +
                                 Serializer::get_value_type_string(argValue) +
                                 "`.");
    }
  }

  if (argValue.isLambda()) {
    auto lambdaId = argValue.getLambda()->identifier;
    ctx->addMappedLambda(param.first, lambdaId);
  } else {
    functionFrame->variables[param.first] = argValue;
  }
}

KValue KInterpreter::executeFunctionBody(
    const std::unique_ptr<KFunction>& function) {
  KValue result;
  const auto& decl = function->decl;
  for (const auto& stmt : decl->body) {
    result = interpret(stmt.get());
    if (callStack.top()->isFlagSet(FrameFlags::Return)) {
      result = callStack.top()->returnValue;
      break;
    }
  }
  return result;
}

KValue KInterpreter::visit(const MethodCallNode* node) {
  auto object = interpret(node->object.get());

  if (object.isObject()) {
    return callObjectMethod(node, object.getObject());
  } else if (object.isStruct()) {
    return callStructMethod(node, object.getStruct());
  } else if (ListBuiltins.is_builtin(node->op)) {
    return interpretListBuiltin(node->token, object, node->op,
                                getMethodCallArguments(node->arguments));
  } else if (KiwiBuiltins.is_builtin(node->op)) {
    return BuiltinDispatch::execute(node->token, node->op, object,
                                    getMethodCallArguments(node->arguments));
  }

  throw UnknownBuiltinError(node->token, node->methodName);
}

std::vector<KValue> KInterpreter::getMethodCallArguments(
    const std::vector<std::unique_ptr<ASTNode>>& args) {
  std::vector<KValue> arguments;
  arguments.reserve(args.size());

  for (const auto& arg : args) {
    arguments.emplace_back(interpret(arg.get()));
  }

  return arguments;
}

KValue KInterpreter::callObjectBaseMethod(const MethodCallNode* node,
                                          const std::shared_ptr<Object>& obj,
                                          const k_string& baseStruct,
                                          const k_string& methodName) {
  const auto& struc = ctx->getStructs().at(baseStruct);
  auto& function = struc->methods[methodName];
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
    throw UnimplementedMethodError(node->token, obj->structName, methodName);
  }

  if (function->isPrivate) {
    throw InvalidContextError(
        node->token, "Cannot invoke private method outside of struct.");
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (contextSwitch) {
    frame->setObjectContext(objContext);
  } else {
    frame->clearFlag(FrameFlags::InObject);
  }

  if (isCtor) {
    return KValue::createObject(obj);
  }

  return result;
}

KValue KInterpreter::callObjectMethod(const MethodCallNode* node,
                                      const std::shared_ptr<Object>& obj) {
  const auto& struc = ctx->getStructs().at(obj->structName);
  auto methodName = node->methodName;

  if (struc->methods.find(methodName) == struc->methods.end()) {
    auto baseStruct = struc->baseStruct;

    if (baseStruct.empty()) {
      throw UnimplementedMethodError(node->token, obj->structName, methodName);
    }

    if (!ctx->hasStruct(baseStruct)) {
      throw StructUndefinedError(node->token, baseStruct);
    }

    return callObjectBaseMethod(node, obj, baseStruct, methodName);
  }

  auto& function = struc->methods[methodName];
  bool isCtor = methodName == Keywords.New;

  auto& frame = callStack.top();
  auto oldObjContext = frame->getObjectContext();
  auto objContext = obj;
  bool contextSwitch = false;

  if (frame->inObjectContext()) {
    contextSwitch = true;
  }

  frame->setObjectContext(obj);

  if (!function) {
    throw UnimplementedMethodError(node->token, obj->structName, methodName);
  }

  if (function->isPrivate) {
    throw InvalidContextError(
        node->token, "Cannot invoke private method outside of struct.");
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (contextSwitch) {
    frame->setObjectContext(oldObjContext);
  } else {
    frame->clearFlag(FrameFlags::InObject);
  }

  if (isCtor) {
    return KValue::createObject(obj);
  }

  return result;
}

KValue KInterpreter::callStructMethod(const MethodCallNode* node,
                                      const k_struct& struc) {
  auto methodName = node->methodName;
  auto& frame = callStack.top();
  const auto& kstruct = ctx->getStructs().at(struc->identifier);
  auto& methods = kstruct->methods;

  // check base
  if (methods.find(methodName) == methods.end()) {
    if (kstruct->baseStruct.empty()) {
      throw UnimplementedMethodError(node->token, struc->identifier,
                                     methodName);
    }

    const auto& baseStruct = ctx->getStructs().at(kstruct->baseStruct);
    auto& baseStructMethods = baseStruct->methods;

    if (baseStructMethods.find(methodName) == baseStructMethods.end()) {
      throw UnimplementedMethodError(node->token, struc->identifier,
                                     methodName);
    }

    return executeStructMethod(baseStructMethods, methodName, frame, node,
                               struc);
  }

  return executeStructMethod(methods, methodName, frame, node, struc);
}

KValue KInterpreter::executeStructMethod(
    std::unordered_map<k_string, std::unique_ptr<KFunction>>& methods,
    k_string& methodName, std::shared_ptr<CallStackFrame>& frame,
    const MethodCallNode* node, const k_struct& struc) {
  auto& function = methods[methodName];
  k_object obj = std::make_shared<Object>();
  bool isCtor = methodName == Keywords.New;

  auto oldObjectContext = frame->getObjectContext();
  bool contextSwitch = false;

  if (!function && isCtor) {
    return KValue::createObject(obj);  // default constructor
  }

  if (!function->isStatic && !isCtor) {
    throw InvalidContextError(node->token,
                              "Cannot invoke non-static method on struct.");
  }

  if (isCtor) {
    if (frame->inObjectContext()) {
      contextSwitch = true;
    }

    obj->structName = struc->identifier;
    frame->setObjectContext(obj);
  }

  auto result =
      callFunction(function, node->arguments, node->token, methodName);

  if (isCtor) {
    if (contextSwitch) {
      frame->setObjectContext(oldObjectContext);
    } else {
      frame->clearFlag(FrameFlags::InObject);
    }
    return KValue::createObject(obj);
  }

  return result;
}

KValue KInterpreter::callBuiltinMethod(const FunctionCallNode* node) {
  auto args = getMethodCallArguments(node->arguments);
  auto op = node->op;

  if (SerializerBuiltins.is_builtin(op)) {
    return interpretSerializerBuiltin(node->token, op, args);
  } else if (ReflectorBuiltins.is_builtin(op)) {
    return interpretReflectorBuiltin(node->token, op, args);
  } else if (WebServerBuiltins.is_builtin(op)) {
    return interpretWebServerBuiltin(node->token, op, args);
  } else if (SignalBuiltins.is_builtin(op)) {
    return interpretSignalBuiltin(node->token, op, args);
  } else if (FFIBuiltins.is_builtin(op)) {
    return BuiltinDispatch::execute(ffimgr, node->token, op, args);
  } else if (SocketBuiltins.is_builtin(op)) {
    return BuiltinDispatch::execute(sockmgr, node->token, op, args);
  } else if (TaskBuiltins.is_builtin(op)) {
    return BuiltinDispatch::execute(taskmgr, node->token, op, args);
  }

  return BuiltinDispatch::execute(node->token, op, args, cliArgs);
}

KValue KInterpreter::interpretWebServerBuiltin(const Token& token,
                                               const KName& builtin,
                                               std::vector<KValue>& args) {
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

void KInterpreter::handleWebServerRequest(int webhookID, k_hashmap requestHash,
                                          k_string& redirect, k_string& content,
                                          k_string& contentType, int& status) {
  KValue result;
  bool requireDrop = false;

  try {
    auto webhook = ctx->getWebHook(webhookID);
    auto webhookFrame = createFrame(Keywords.Spawn);

    auto& lambda = ctx->getLambdas().at(webhook);

    for (const auto& param : lambda->parameters) {
      webhookFrame->variables[param.first] = KValue::createHashmap(requestHash);
      break;
    }

    requireDrop = pushFrame(webhookFrame);

    const auto& decl = lambda->getBody();
    for (const auto& stmt : decl) {
      result = interpret(stmt.get());
      if (webhookFrame->isFlagSet(FrameFlags::Return)) {
        result = webhookFrame->returnValue;
        break;
      }
    }

    if (result.isHashmap()) {
      const auto& contentKey = KValue::createString("content");
      const auto& contentTypeKey = KValue::createString("content-type");
      const auto& statusKey = KValue::createString("status");
      const auto& redirectKey = KValue::createString("redirect");

      auto responseHash = result.getHashmap();
      if (responseHash->hasKey(contentKey)) {
        auto responseHashContent = responseHash->get(contentKey);
        content = Serializer::serialize(responseHashContent);
      }

      if (responseHash->hasKey(contentTypeKey)) {
        auto responseHashContent = responseHash->get(contentTypeKey);
        if (responseHashContent.isString()) {
          contentType = responseHashContent.getString();
        }
      }

      if (responseHash->hasKey(statusKey)) {
        auto responseHashContent = responseHash->get(statusKey);
        if (responseHashContent.isInteger()) {
          status = static_cast<int>(responseHashContent.getInteger());
        }
      }

      if (responseHash->hasKey(redirectKey)) {
        auto responseHashContent = responseHash->get(redirectKey);
        if (responseHashContent.isString()) {
          redirect = responseHashContent.getString();
        }
      }
    }

    dropFrame();
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }
    throw;
  }
}

int KInterpreter::getNextWebServerHook(const Token& token, KValue& arg) {
  if (!arg.isLambda()) {
    throw InvalidOperationError(token,
                                "Expected lambda for second parameter of `" +
                                    WebServerBuiltins.Get + "`.");
  }

  auto lambdaName = arg.getLambda()->identifier;
  if (!ctx->hasLambda(lambdaName)) {
    if (ctx->hasMappedLambda(lambdaName)) {
      lambdaName = ctx->getMappedLambda(lambdaName);
    }
  }
  int webhookID = 0;

  if (!ctx->getWebHooks().empty()) {
    webhookID = static_cast<int>(ctx->getWebHooks().size());
  }

  ctx->addWebHook(webhookID, lambdaName);
  return webhookID;
}

KValue KInterpreter::interpretWebServerGet(const Token& token,
                                           std::vector<KValue>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Get);
  }

  auto endpointList = getWebServerEndpointList(token, args.at(0));
  int webhookID = getNextWebServerHook(token, args.at(1));

  for (const auto& endpoint : endpointList) {
    ctx->getServer().Get(
        endpoint,
        [this, webhookID](const httplib::Request& req, httplib::Response& res) {
          auto requestHash = getWebServerRequestHash(req);

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

KValue KInterpreter::interpretWebServerPost(const Token& token,
                                            std::vector<KValue>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Post);
  }

  auto endpointList = getWebServerEndpointList(token, args.at(0));
  int webhookID = getNextWebServerHook(token, args.at(1));

  for (const auto& endpoint : endpointList) {
    ctx->getServer().Post(
        endpoint,
        [this, webhookID](const httplib::Request& req, httplib::Response& res) {
          auto requestHash = getWebServerRequestHash(req);

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

KValue KInterpreter::interpretWebServerListen(const Token& token,
                                              std::vector<KValue>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Listen);
  }

  auto host = get_string(token, args.at(0));
  auto port = get_integer(token, args.at(1));

  ctx->getServer().listen(host, static_cast<int>(port));

  auto hash = std::make_shared<Hashmap>();
  hash->add(KValue::createString("host"), KValue::createString(host));
  hash->add(KValue::createString("port"), KValue::createInteger(port));

  return KValue::createHashmap(hash);
}

KValue KInterpreter::interpretWebServerPublic(const Token& token,
                                              std::vector<KValue>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Public);
  }

  auto endpoint = get_string(token, args.at(0));
  auto publicDir = get_string(token, args.at(1));

  if (!File::directoryExists(token, publicDir)) {
    return KValue::createBoolean(false);
  }

  ctx->getServer().set_mount_point(endpoint, publicDir);

  return KValue::createBoolean(true);
}

std::vector<k_string> KInterpreter::getWebServerEndpointList(const Token& token,
                                                             KValue& arg) {
  std::vector<k_string> endpointList;

  if (arg.isString()) {
    endpointList.emplace_back(get_string(token, arg));
  } else if (arg.isList()) {
    const auto& elements = arg.getList()->elements;
    k_string endpoint;

    for (const auto& el : elements) {
      if (!el.isString()) {
        continue;
      }

      endpoint = get_string(token, el);
      if (std::find(endpointList.begin(), endpointList.end(), endpoint) ==
          endpointList.end()) {
        endpointList.emplace_back(endpoint);
      }
    }
  }

  return endpointList;
}

k_hashmap KInterpreter::getWebServerRequestHash(const httplib::Request& req) {
  auto requestHash = std::make_shared<Hashmap>();
  auto headers = req.headers;
  auto params = req.params;

  for (auto it = headers.begin(); it != headers.end(); ++it) {
    const auto& x = *it;
    requestHash->add(KValue::createString(x.first),
                     KValue::createString(x.second));
  }

  auto pathParamsHash = std::make_shared<Hashmap>();
  for (const auto& pair : req.path_params) {
    pathParamsHash->add(KValue::createString(pair.first),
                        KValue::createString(pair.second));
  }

  auto paramsHash = std::make_shared<Hashmap>();
  for (auto it = params.begin(); it != params.end(); ++it) {
    const auto& x = *it;
    paramsHash->add(KValue::createString(x.first),
                    KValue::createString(x.second));
  }

  auto filesHash = std::make_shared<Hashmap>();

  const auto& contentKey = KValue::createString("content");
  const auto& contentTypeKey = KValue::createString("content_type");
  const auto& fileNameKey = KValue::createString("filename");
  const auto& nameKey = KValue::createString("name");

  for (const auto& file : req.files) {
    auto fileHash = std::make_shared<Hashmap>();
    fileHash->add(contentKey, KValue::createString(file.second.content));
    fileHash->add(contentTypeKey,
                  KValue::createString(file.second.content_type));
    fileHash->add(fileNameKey, KValue::createString(file.second.filename));
    fileHash->add(nameKey, KValue::createString(file.second.name));
    filesHash->add(KValue::createString(file.first),
                   KValue::createHashmap(fileHash));
  }

  requestHash->add(KValue::createString("body"),
                   KValue::createString(req.body));
  requestHash->add(KValue::createString("files"),
                   KValue::createHashmap(filesHash));
  requestHash->add(KValue::createString("path"),
                   KValue::createString(req.path));
  requestHash->add(KValue::createString("path_params"),
                   KValue::createHashmap(pathParamsHash));
  requestHash->add(KValue::createString("params"),
                   KValue::createHashmap(paramsHash));

  return requestHash;
}

KValue KInterpreter::interpolateString(const Token& token,
                                       const k_string& input) {
  Parser parser(true);
  Lexer lexer(token.getFile(), input);
  auto tempStream = lexer.getTokenStream();
  auto ast = parser.parseTokenStream(tempStream, true);

  return interpret(ast.get());
}

KValue KInterpreter::interpretReflectorBuiltin(const Token& token,
                                               const KName& builtin,
                                               std::vector<KValue>& args) {
  if (builtin == KName::Builtin_Reflector_RList) {
    return interpretReflectorRList(token, args);
  } else if (builtin == KName::Builtin_Reflector_RObject) {
    return interpretReflectorRObject(token, args);
  } else if (builtin == KName::Builtin_Reflector_RStack) {
    return interpretReflectorRStack(token, args);
  } else if (builtin == KName::Builtin_Reflector_RFFlags) {
    return interpretReflectorRFFlags(token, args);
  } else if (builtin == KName::Builtin_Reflector_RRetVal) {
    return interpretReflectorRRetVal(token, args);
  }

  throw InvalidOperationError(token, "Come back later.");
}

KValue KInterpreter::interpretReflectorRFFlags(const Token& token,
                                               std::vector<KValue>& args) {
  if (args.size() > 1) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RFFlags);
  }

  std::vector<KValue> list;
  int fromTop = 0;

  if (args.size() == 1) {
    fromTop = get_integer(token, args.at(0));
    if (fromTop < 0) {
      throw InvalidOperationError(
          token, "Expected a positive integer for first parameter of `" +
                     ReflectorBuiltins.RFFlags + "`.");
    }
  }

  std::stack<std::shared_ptr<CallStackFrame>> tempStack(callStack);

  if (static_cast<size_t>(fromTop) > tempStack.size()) {
    throw InvalidOperationError(
        token, "Too many frames. Expected < " +
                   std::to_string(tempStack.size()) +
                   " but instead received: " + std::to_string(fromTop));
  }

  int frameCounter = 0;

  while (!tempStack.empty()) {
    auto frame = tempStack.top();

    if (frameCounter - fromTop == 0) {
      if (frame->isFlagSet(FrameFlags::Break)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::Break)));
      }

      if (frame->isFlagSet(FrameFlags::InLambda)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::InLambda)));
      }

      if (frame->isFlagSet(FrameFlags::InLoop)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::InLoop)));
      }

      if (frame->isFlagSet(FrameFlags::InObject)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::InObject)));
      }

      if (frame->isFlagSet(FrameFlags::InTry)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::InTry)));
      }

      if (frame->isFlagSet(FrameFlags::Next)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::Next)));
      }

      if (frame->isFlagSet(FrameFlags::None)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::None)));
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::Return)));
      }

      if (frame->isFlagSet(FrameFlags::SubFrame)) {
        list.push_back(
            KValue::createInteger(static_cast<k_int>(FrameFlags::SubFrame)));
      }

      break;
    }

    frameCounter++;
    tempStack.pop();
  }

  return KValue::createList(std::make_shared<List>(list));
}

KValue KInterpreter::interpretReflectorRRetVal(const Token& token,
                                               std::vector<KValue>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RRetVal);
  }

  return callStack.top()->returnValue;
}

KValue KInterpreter::interpretReflectorRList(const Token& token,
                                             std::vector<KValue>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RList);
  }

  auto rlist = std::make_shared<Hashmap>();
  auto rlistPackages = std::make_shared<List>();
  auto rlistStructs = std::make_shared<List>();
  auto rlistFunctions = std::make_shared<List>();
  auto rlistStack = std::make_shared<List>();

  const auto& variablesKey = KValue::createString("variables");
  const auto& packagesKey = KValue::createString("packages");
  const auto& structsKey = KValue::createString("structs");
  const auto& functionsKey = KValue::createString("functions");
  const auto& stackKey = KValue::createString("stack");

  rlistPackages->elements.reserve(ctx->getPackages().size());
  rlistStructs->elements.reserve(ctx->getStructs().size());
  rlistFunctions->elements.reserve(ctx->getFunctions().size());
  rlistStack->elements.reserve(callStack.size());

  for (const auto& m : ctx->getMethods()) {
    rlistFunctions->elements.emplace_back(KValue::createString(m.first));
  }

  for (const auto& p : ctx->getPackages()) {
    rlistPackages->elements.emplace_back(KValue::createString(p.first));
  }

  for (const auto& c : ctx->getStructs()) {
    rlistStructs->elements.emplace_back(KValue::createString(c.first));
  }

  std::stack<std::shared_ptr<CallStackFrame>> tempStack(callStack);

  while (!tempStack.empty()) {
    const auto& outerFrame = tempStack.top();
    const auto& frameVariables = outerFrame->variables;

    auto rlistStackFrame = std::make_shared<Hashmap>();
    auto rlistStackFrameVariables = std::make_shared<List>();

    rlistStackFrameVariables->elements.reserve(frameVariables.size());

    for (const auto& v : frameVariables) {
      auto rlistStackFrameVariable = std::make_shared<Hashmap>();
      rlistStackFrameVariable->add(KValue::createString(v.first), v.second);
      rlistStackFrameVariables->elements.emplace_back(
          KValue::createHashmap(rlistStackFrameVariable));
    }

    sort_list(*rlistStackFrameVariables);

    rlistStackFrame->add(variablesKey,
                         KValue::createList(rlistStackFrameVariables));
    rlistStack->elements.emplace_back(KValue::createHashmap(rlistStackFrame));

    tempStack.pop();
  }

  sort_list(*rlistPackages);
  sort_list(*rlistStructs);
  sort_list(*rlistFunctions);
  std::reverse(rlistStack->elements.begin(), rlistStack->elements.end());

  rlist->add(packagesKey, KValue::createList(rlistPackages));
  rlist->add(structsKey, KValue::createList(rlistStructs));
  rlist->add(functionsKey, KValue::createList(rlistFunctions));
  rlist->add(stackKey, KValue::createList(rlistStack));

  return KValue::createHashmap(rlist);
}

KValue KInterpreter::interpretReflectorRObject(const Token& token,
                                               std::vector<KValue>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RObject);
  }

  auto frame = callStack.top();
  if (frame->inObjectContext()) {
    const auto& serializedObject =
        Serializer::basic_serialize_object(frame->getObjectContext());
    return KValue::createString(serializedObject);
  }

  return {};
}

KValue KInterpreter::interpretReflectorRStack(const Token& token,
                                              std::vector<KValue>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RStack);
  }

  std::stack<k_string> tempStack(funcStack);
  std::vector<KValue> stackNames;
  stackNames.reserve(tempStack.size());

  while (!tempStack.empty()) {
    stackNames.emplace_back(KValue::createString(tempStack.top()));
    tempStack.pop();
  }

  return KValue::createList(std::make_shared<List>(stackNames));
}

KValue KInterpreter::interpretSerializerDeserialize(const Token& token,
                                                    std::vector<KValue>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SerializerBuiltins.Deserialize);
  }

  return interpolateString(token, get_string(token, args.at(0)));
}

KValue KInterpreter::interpretSerializerSerialize(const Token& token,
                                                  std::vector<KValue>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SerializerBuiltins.Serialize);
  }

  return KValue::createString(Serializer::serialize(args.at(0), true));
}

KValue KInterpreter::interpretSerializerBuiltin(const Token& token,
                                                const KName& builtin,
                                                std::vector<KValue>& args) {
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

KValue KInterpreter::interpretSignalBuiltin(const Token& token, const KName& op,
                                            std::vector<KValue> args) {
  if (SAFEMODE) {
    return {};
  }

  switch (op) {
    case KName::Builtin_Signal_Send:
      return interpretSignalSend(token, args);

    case KName::Builtin_Signal_Raise:
      return interpretSignalRaise(token, args);

    case KName::Builtin_Signal_Trap:
      return interpretSignalTrap(token, args);

    default:
      break;
  }

  return {};
}

void KInterpreter::handlePendingSignals(const Token& token) {
  if (!signal_pending.load(std::memory_order_relaxed)) {
    return;
  }

  int signum = last_received_signal.load(std::memory_order_relaxed);
  k_string lambda;

  {
    std::lock_guard<std::mutex> lock(signal_mutex);
    auto it = k_signal_handlers.find(signum);
    if (it != k_signal_handlers.end()) {
      lambda = it->second;
    } else {
      // No handler registered for this signal
      return;
    }
  }

  signal_pending.store(false, std::memory_order_relaxed);

  if (!lambda.empty()) {
    // Call the lambda
    std::vector<KValue> args;
    this->callLambda(token, lambda, args);
  }
}

KValue KInterpreter::interpretSignalTrap(const Token& token,
                                         std::vector<KValue> args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, SignalBuiltins.Trap);
  }

  int signum = static_cast<int>(get_integer(token, args.at(0)));

  if (!args.at(1).isLambda()) {
    throw InvalidOperationError(
        token,
        "Expected a lambda for parameter 2 of `" + SignalBuiltins.Trap + "`.");
  }

  auto lambdaRef = args.at(1).getLambda();

  if (!ctx->hasLambda(lambdaRef->identifier)) {
    throw InvalidOperationError(
        token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
  }

  {
    std::lock_guard<std::mutex> lock(signal_mutex);
    k_signal_handlers[signum] = lambdaRef->identifier;
  }

  struct sigaction sa;
  sa.sa_handler = k_signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;  // Restart interrupted system calls

  int res = sigaction(signum, &sa, nullptr);

  if (res == -1) {
    throw SignalError(token, "Failed to register signal handler: " +
                                 k_string(std::strerror(errno)));
  }

  return KValue::createInteger(res);
}

KValue KInterpreter::interpretSignalRaise(const Token& token,
                                          const std::vector<KValue>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SignalBuiltins.Raise);
  }

  int sig = static_cast<int>(get_integer(token, args[0]));
  int res = raise(sig);

  if (res != 0) {
    throw SignalError(token, "Failed to raise signal");
  }

  return KValue::createInteger(res);
}

KValue KInterpreter::interpretSignalSend(const Token& token,
                                         const std::vector<KValue>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, SignalBuiltins.Send);
  }

  pid_t pid = static_cast<pid_t>(get_integer(token, args.at(0)));
  int sig = static_cast<int>(get_integer(token, args.at(1)));
  int res = kill(pid, sig);

  if (res == -1) {
    throw SignalError(
        token, "Failed to send signal: " + k_string(std::strerror(errno)));
  }

  return KValue::createInteger(res);
}

KValue KInterpreter::interpretListBuiltin(const Token& token, KValue& object,
                                          const KName& op,
                                          std::vector<KValue> args) {
  if (!object.isList()) {
    throw InvalidOperationError(
        token, "Expected a list for specialized list builtin.");
  }

  auto list = object.getList();

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

  if (args.size() == 1) {
    auto arg = args.at(0);
    if (!arg.isLambda()) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }

    const auto& lambdaRef = arg.getLambda();

    if (!ctx->hasLambda(lambdaRef->identifier)) {
      throw InvalidOperationError(
          token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
    }

    auto& lambda = ctx->getLambdas().at(lambdaRef->identifier);
    const auto isReturnSet = callStack.top()->isFlagSet(FrameFlags::Return);
    KValue result;

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
      if (!same_value(frame->returnValue.getValue(), result.getValue())) {
        frame->returnValue = result;
      }
    }

    return result;
  } else if (args.size() == 2 && op == KName::Builtin_List_Reduce) {
    auto arg = args.at(1);

    if (!arg.isLambda()) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }
    auto lambdaRef = arg.getLambda();

    if (!ctx->hasLambda(lambdaRef->identifier)) {
      throw InvalidOperationError(
          token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
    }

    auto& lambda = ctx->getLambdas().at(lambdaRef->identifier);

    return lambdaReduce(lambda, args.at(0), list);
  }

  throw InvalidOperationError(token,
                              "Invalid specialized list builtin invocation.");
}

KValue KInterpreter::listSum(const k_list& list) {
  return sum_listvalue(list);
}

KValue KInterpreter::listMin(const Token& token, const k_list& list) {
  if (list->elements.empty()) {
    throw EmptyListError(token);
  }

  return min_listvalue(list);
}

KValue KInterpreter::listMax(const Token& token, const k_list& list) {
  if (list->elements.empty()) {
    throw EmptyListError(token);
  }

  return max_listvalue(list);
}

KValue KInterpreter::listSort(const k_list& list) {
  sort_list(*list);
  return KValue::createList(list);
}

KValue KInterpreter::lambdaEach(std::unique_ptr<KLambda>& lambda,
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

  KValue result, indexValue;
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      indexValue.setValue(static_cast<k_int>(i));
      frame->variables[indexVariable] = indexValue;
    }

    for (const auto& stmt : decl->body) {
      result = interpret(stmt.get());
    }
  }

  frame->variables.erase(valueVariable);
  if (hasIndexVariable) {
    frame->variables.erase(indexVariable);
  }

  return result;
}

KValue KInterpreter::lambdaNone(std::unique_ptr<KLambda>& lambda,
                                const k_list& list) {
  auto selected = lambdaSelect(lambda, list);
  auto noneFound = KValue::createBoolean(false);

  if (selected.isList()) {
    const auto& isEmpty = selected.getList()->elements.empty();
    noneFound.setValue(isEmpty);
  }

  return noneFound;
}

KValue KInterpreter::lambdaMap(std::unique_ptr<KLambda>& lambda,
                               const k_list& list) {
  auto defaultParameters = lambda->defaultParameters;
  auto frame = callStack.top();

  k_string mapVariable;

  if (lambda->parameters.empty()) {
    return KValue::createList(list);
  }

  for (size_t i = 0; i < lambda->parameters.size(); ++i) {
    const auto& param = lambda->parameters[i];
    if (i == 0) {
      mapVariable = param.first;
      frame->variables[mapVariable] = {};
    }
  }

  const auto& decl = lambda->decl;
  const auto& elements = list->elements;
  std::vector<KValue> resultList;
  KValue result = {};

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[mapVariable] = elements.at(i);

    for (const auto& stmt : decl->body) {
      result = interpret(stmt.get());
      if (frame->isFlagSet(FrameFlags::Return)) {
        frame->clearFlag(FrameFlags::Return);
      }
      resultList.emplace_back(result);
    }
  }

  frame->variables.erase(mapVariable);

  return KValue::createList(std::make_shared<List>(resultList));
}

KValue KInterpreter::lambdaReduce(std::unique_ptr<KLambda>& lambda,
                                  KValue accumulator, const k_list& list) {
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
  const auto& decl = lambda->decl;
  KValue result;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    for (const auto& stmt : decl->body) {
      result = interpret(stmt.get());
    }
  }

  result = frame->variables[accumVariable];

  frame->variables.erase(accumVariable);
  frame->variables.erase(valueVariable);

  return result;
}

KValue KInterpreter::lambdaAll(std::unique_ptr<KLambda>& lambda,
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

  KValue result, indexValue;
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      indexValue.setValue(static_cast<k_int>(i));
      frame->variables[indexVariable] = indexValue;
    }

    for (const auto& stmt : decl->body) {
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

  return KValue::createBoolean(newListSize == listSize);
}

KValue KInterpreter::lambdaSelect(std::unique_ptr<KLambda>& lambda,
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

  KValue result, indexValue;
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;
  std::vector<KValue> resultList;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      indexValue.setValue(static_cast<k_int>(i));
      frame->variables[indexVariable] = indexValue;
    }

    for (const auto& stmt : decl->body) {
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

  return KValue::createList(std::make_shared<List>(resultList));
}

bool KInterpreter::shouldUpdateFrameVariables(
    const k_string& varName, const std::shared_ptr<CallStackFrame> nextFrame) {
  return nextFrame->hasVariable(varName);
}

void KInterpreter::updateVariablesInCallerFrame(
    const std::unordered_map<k_string, KValue>& variables,
    std::shared_ptr<CallStackFrame> callerFrame) {
  auto& frameVariables = callerFrame->variables;
  for (const auto& var : variables) {
    if (shouldUpdateFrameVariables(var.first, callerFrame)) {
      frameVariables[var.first] = var.second;
    }
  }
}

k_string KInterpreter::getTemporaryId() {
  return "temporary_" + RNG::getInstance().random16();
}

void KInterpreter::updateListSlice(const Token& token, bool insertOp,
                                   k_list& targetList, const SliceIndex& slice,
                                   const k_list& rhsValues) {
  if (!slice.indexOrStart.isInteger()) {
    throw IndexError(token, "Start index must be an integer.");
  } else if (!slice.stopIndex.isInteger()) {
    throw IndexError(token, "Stop index must be an integer.");
  } else if (!slice.stepValue.isInteger()) {
    throw IndexError(token, "Step value must be an integer.");
  }

  int start = slice.indexOrStart.getInteger();
  int stop = slice.stopIndex.getInteger();
  int step = slice.stepValue.getInteger();

  // This is a single element assignment.
  if (!slice.isSlice && insertOp) {
    stop = start;
  }

  auto& elements = targetList->elements;
  auto& rhsElements = rhsValues->elements;

  // Convert negative indices and adjust ranges
  int listSize = static_cast<int>(elements.size());
  int rhsSize = static_cast<int>(rhsElements.size());

  start += start < 0 ? listSize : 0;
  stop += stop < 0 ? listSize : 0;
  start = start < 0 ? 0 : start;
  stop = stop > listSize ? listSize : stop;
  // Special case for reverse slicing
  stop = step < 0 && stop == listSize ? -1 : stop;

  if (step == 1) {
    // Simple case: step is 1
    if (start >= stop) {
      elements.erase(elements.begin() + start, elements.begin() + stop);
      elements.insert(elements.begin() + start, rhsElements.begin(),
                      rhsElements.end());
    } else {
      std::copy(rhsElements.begin(), rhsElements.end(),
                elements.begin() + start);
    }
  } else {
    // Complex case: step != 1
    int rhsIndex = 0;
    for (int i = start; i != stop && rhsIndex < rhsSize; i += step) {
      if ((step > 0 && i < listSize) || (step < 0 && i >= 0)) {
        elements[i] = rhsElements.at(rhsIndex++);
      } else {
        break;  // Avoid going out of bounds
      }
    }
  }
}

KValue KInterpreter::stringSlice(const Token& token, SliceIndex& slice,
                                 const k_string& string) {
  auto list = std::make_shared<List>();

  auto& elements = list->elements;
  elements.reserve(string.size());
  k_string temp(1, '\0');
  for (const char& c : string) {
    temp[0] = c;
    elements.emplace_back(KValue::createString(temp));
  }

  auto sliced = listSlice(token, slice, list);

  if (sliced.isList()) {
    auto slicedlist = sliced.getList()->elements;
    std::ostringstream sv;

    for (auto it = slicedlist.begin(); it != slicedlist.end(); ++it) {
      sv << Serializer::serialize(*it);
    }

    return KValue::createString(sv.str());
  }

  return KValue::createString(Serializer::serialize(sliced));
}

KValue KInterpreter::listSlice(const Token& token, const SliceIndex& slice,
                               const k_list& list) {
  auto& elements = list->elements;

  if (!slice.indexOrStart.isInteger()) {
    throw IndexError(token, "Start index must be an integer.");
  } else if (!slice.stopIndex.isInteger()) {
    throw IndexError(token, "Stop index must be an integer.");
  } else if (!slice.stepValue.isInteger()) {
    throw IndexError(token, "Step value must be an integer.");
  }

  int start = slice.indexOrStart.getInteger(),
      stop = slice.stopIndex.getInteger(), step = slice.stepValue.getInteger();

  // Adjust negative indices
  int listSize = static_cast<int>(elements.size());
  if (start < 0) {
    start = start + listSize > 0 ? start + listSize : 0;
  }

  if (stop < 0) {
    stop += listSize;
  } else {
    stop = stop < listSize ? stop : listSize;
  }

  // Adjust stop for reverse slicing
  if (step < 0 && stop == listSize) {
    stop = -1;
  }

  auto slicedList = std::make_shared<List>();
  auto& slicedElements = slicedList->elements;

  if (step < 0) {
    for (int i = (start == 0 ? listSize - 1 : start); i >= stop; i += step) {
      // Prevent out-of-bounds access
      if (i < 0 || i >= listSize) {
        break;
      }

      slicedElements.emplace_back(elements.at(i));
    }
  } else {
    for (int i = start; i < stop; i += step) {
      // Prevent out-of-bounds access
      if (i >= listSize) {
        break;
      }

      slicedElements.emplace_back(elements.at(i));
    }
  }
  return KValue::createList(slicedList);
}

#endif
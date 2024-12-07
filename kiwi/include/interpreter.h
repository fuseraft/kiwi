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

  k_value interpret(const ASTNode* node);

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

  k_value visit(const ProgramNode* node);
  k_value visit(const SelfNode* node);
  k_value visit(const PackAssignmentNode* node);
  k_value visit(const AssignmentNode* node);
  k_value visit(const ConstAssignmentNode* node);
  k_value visit(const IndexAssignmentNode* node);
  k_value visit(const MemberAssignmentNode* node);
  k_value visit(const MemberAccessNode* node);
  k_value visit(const IdentifierNode* node);
  k_value visit(const LiteralNode* node);
  k_value visit(const ListLiteralNode* node);
  k_value visit(const RangeLiteralNode* node);
  k_value visit(const HashLiteralNode* node);
  k_value visit(const PrintNode* node);
  k_value visit(const PrintXyNode* node);
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
  k_value visit(const StructNode* node);
  k_value visit(const FunctionDeclarationNode* node);
  k_value visit(const VariableDeclarationNode* node);
  k_value visit(const FunctionCallNode* node);
  k_value visit(const MethodCallNode* node);
  k_value visit(const PackageNode* node);
  k_value visit(const ImportNode* node);
  k_value visit(const ExportNode* node);
  k_value visit(const ParseNode* node);
  k_value visit(const ExitNode* node);
  k_value visit(const ThrowNode* node);
  k_value visit(const ReturnNode* node);
  k_value visit(const IndexingNode* node);
  k_value visit(const SliceNode* node);
  k_value visit(const SpawnNode* node);

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

  void importPackage(const k_value& packageName, const Token& token);
  void importExternal(const k_string& packageName, const Token& token);

  KCallableType getCallable(const Token& token, const k_string& name) const;
  std::vector<k_value> getMethodCallArguments(
      const std::vector<std::unique_ptr<ASTNode>>& args);
  k_value callBuiltinMethod(const FunctionCallNode* node);
  k_value callStructMethod(const MethodCallNode* node, const k_struct& struc);
  k_value callObjectBaseMethod(const MethodCallNode* node,
                               const std::shared_ptr<Object>& obj,
                               const k_string& baseStruct,
                               const k_string& methodName);
  k_value callObjectMethod(const MethodCallNode* node,
                           const std::shared_ptr<Object>& obj);
  k_value callLambda(const Token& token, const k_string& lambdaName,
                     const std::vector<std::unique_ptr<ASTNode>>& arguments,
                     bool& requireDrop);
  k_value callLambda(const Token& token, const k_string& lambdaName,
                     const std::vector<k_value>& arguments);
  void prepareLambdaCall(const std::unique_ptr<KLambda>& func,
                         const std::vector<std::unique_ptr<ASTNode>>& arguments,
                         std::unordered_set<k_string>& defaultParameters,
                         const Token& token, k_string& targetLambda,
                         const std::unordered_map<k_string, KName>& typeHints,
                         const k_string& lambdaName,
                         std::shared_ptr<CallStackFrame>& lambdaFrame);
  void prepareLambdaVariables(
      const std::unordered_map<k_string, KName>& typeHints,
      const std::pair<k_string, k_value>& param, k_value& argValue,
      const Token& token, size_t i, const k_string& lambdaName,
      std::shared_ptr<CallStackFrame>& lambdaFrame);
  void prepareLambdaCall(const std::unique_ptr<KLambda>& func,
                         const std::vector<k_value>& arguments,
                         std::unordered_set<k_string>& defaultParameters,
                         const Token& token, k_string& targetLambda,
                         const std::unordered_map<k_string, KName>& typeHints,
                         const k_string& lambdaName,
                         std::shared_ptr<CallStackFrame>& lambdaFrame);
  k_value callFunction(const std::unique_ptr<KFunction>& function,
                       const std::vector<std::unique_ptr<ASTNode>>& arguments,
                       const Token& token, const k_string& functionName);

  void prepareFunctionVariables(
      const std::unordered_map<k_string, KName>& typeHints,
      const std::pair<k_string, k_value>& param, k_value& argValue,
      const Token& token, size_t i, const k_string& functionName,
      std::shared_ptr<CallStackFrame>& functionFrame);

  std::unique_ptr<KFunction> createFunction(const FunctionDeclarationNode* node,
                                            const k_string& name);

  k_value executeStructMethod(
      std::unordered_map<k_string, std::unique_ptr<KFunction>>& methods,
      k_string& methodName, std::shared_ptr<CallStackFrame>& frame,
      const MethodCallNode* node, const k_struct& struc);
  k_value executeInstanceMethod(const FunctionCallNode* node,
                                bool& requireDrop);
  k_value executeInstanceMethodFunction(
      std::unordered_map<k_string, std::unique_ptr<KFunction>>& strucMethods,
      const FunctionCallNode* node, bool& requireDrop);
  void prepareFunctionCall(const std::unique_ptr<KFunction>& func,
                           const FunctionCallNode* node,
                           std::unordered_set<k_string>& defaultParameters,
                           const std::unordered_map<k_string, KName>& typeHints,
                           std::shared_ptr<CallStackFrame>& functionFrame);
  k_value executeFunctionBody(const std::unique_ptr<KFunction>& function);

  k_value handleNestedIndexing(const IndexingNode* indexExpr, k_value baseObj,
                               const KName& op, const k_value& newValue);

  // Scope value propagation
  bool shouldUpdateFrameVariables(
      const k_string& varName, const std::shared_ptr<CallStackFrame> nextFrame);
  void updateVariablesInCallerFrame(
      const std::unordered_map<k_string, k_value>& variables,
      std::shared_ptr<CallStackFrame> callerFrame);

  // Slices
  SliceIndex getSlice(const SliceNode* node, k_value object);
  k_value doSliceAssignment(const Token& token, k_value& slicedObj,
                            const SliceIndex& slice, k_value& newValue);
  void updateListSlice(const Token& token, bool insertOp, k_list& targetList,
                       const SliceIndex& slice, const k_list& rhsValues);
  k_value stringSlice(const Token& token, SliceIndex& slice,
                      const k_value& value);
  k_value listSlice(const Token& token, const SliceIndex& slice,
                    const k_value& value);

  // Loops.
  k_value listLoop(const ForLoopNode* node, const k_list& list);
  k_value hashLoop(const ForLoopNode* node, const k_hashmap& hash);

  // Signals
  k_value interpretSignalBuiltin(const Token& token, const KName& op,
                                 std::vector<k_value> args);
  k_value interpretSignalTrap(const Token& token, std::vector<k_value> args);
  k_value interpretSignalRaise(const Token& token,
                               const std::vector<k_value>& args);
  k_value interpretSignalSend(const Token& token,
                              const std::vector<k_value>& args);
  void handlePendingSignals(const Token& token);

  // List builtins
  k_value interpretListBuiltin(const Token& token, k_value& object,
                               const KName& op, std::vector<k_value> args);
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

  // Serialization
  k_value interpolateString(const Token& token, const k_string& input);

  k_value interpretSerializerDeserialize(const Token& token,
                                         std::vector<k_value>& args);
  k_value interpretSerializerSerialize(const Token& token,
                                       std::vector<k_value>& args);
  k_value interpretSerializerBuiltin(const Token& token, const KName& builtin,
                                     std::vector<k_value>& args);

  // Reflection
  k_value interpretReflectorBuiltin(const Token& token, const KName& builtin,
                                    std::vector<k_value>& args);
  k_value interpretReflectorRList(const Token& token,
                                  std::vector<k_value>& args);
  k_value interpretReflectorRObject(const Token& token,
                                    std::vector<k_value>& args);
  k_value interpretReflectorRStack(const Token& token,
                                   std::vector<k_value>& args);
  k_value interpretReflectorRFFlags(const Token& token,
                                    std::vector<k_value>& args);
  k_value interpretReflectorRRetVal(const Token& token,
                                    std::vector<k_value>& args);

  // Web server
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
  k_hashmap getWebServerRequestHash(const httplib::Request& req);
  void handleWebServerRequest(int webhookID, k_hashmap requestHash,
                              k_string& redirect, k_string& content,
                              k_string& contentType, int& status);
  std::vector<k_string> getWebServerEndpointList(const Token& token,
                                                 k_value& arg);
};

k_value KInterpreter::interpret(const ASTNode* node) {
  if (!node) {
    return {};
  }

  k_value result = {};
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

k_value KInterpreter::visit(const SpawnNode* node) {
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

  return k_value(taskId);
}

k_value KInterpreter::visit(const ProgramNode* node) {
  // This is the program root
  if (!node->isScript) {
    auto programFrame = std::make_shared<CallStackFrame>();
    programFrame->name = kiwi_name;
    programFrame->variables[Keywords.Global] = std::make_shared<Hashmap>();
    pushFrame(programFrame);
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

      if (std::holds_alternative<k_hashmap>(errorValue)) {
        auto errorHash = std::get<k_hashmap>(errorValue);
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

void KInterpreter::importPackage(const k_value& packageName,
                                 const Token& token) {
  if (!std::holds_alternative<k_string>(packageName)) {
    throw InvalidOperationError(token,
                                "Expected the name of a package to import.");
  }

  auto packageNameValue = std::get<k_string>(packageName);

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

k_value KInterpreter::visit(const ParseNode* node) {
  auto content = interpret(node->parseValue.get());

  if (!std::holds_alternative<k_string>(content)) {
    throw KiwiError::create(node->token, "Invalid parse expression.");
  }

  Lexer lexer(node->token.getFile(), std::get<k_string>(content));

  Parser p(true);
  auto tokenStream = lexer.getTokenStream();
  auto ast = p.parseTokenStream(tokenStream, true);

  interpret(ast.get());

  return {};
}

k_value KInterpreter::visit(const MemberAccessNode* node) {
  auto object = interpret(node->object.get());
  auto memberName = node->memberName;

  if (std::holds_alternative<k_hashmap>(object)) {
    auto hash = std::get<k_hashmap>(object);
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
    updateListSlice(token, false, targetList, slice, rhsValues);

    return targetList;
  }

  return slicedObj;
}

k_value KInterpreter::handleNestedIndexing(const IndexingNode* indexExpr,
                                           k_value baseObj, const KName& op,
                                           const k_value& newValue) {
  if (indexExpr->indexExpression->type == ASTNodeType::INDEX) {
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

      if (nestedIndexExpr->indexExpression->type == ASTNodeType::INDEX) {
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
             std::holds_alternative<k_hashmap>(baseObj)) {
    auto key = id(indexExpr->indexExpression.get());
    auto hashObj = std::get<k_hashmap>(baseObj);

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
    } else if (std::holds_alternative<k_hashmap>(baseObj)) {
      auto hash = std::get<k_hashmap>(baseObj);

      if (op == KName::Ops_Assign) {
        hash->add(literal, newValue);
      } else {
        auto oldValue = hash->get(literal);
        hash->add(literal, MathImpl.do_binary_op(indexExpr->token, op, oldValue,
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

  if (node->object->type == ASTNodeType::SLICE) {
    auto sliceExpr = static_cast<const SliceNode*>(node->object.get());

    if (sliceExpr->slicedObject->type == ASTNodeType::IDENTIFIER) {
      identifierName = id(sliceExpr->slicedObject.get());
      k_value slicedObj = {};

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
      k_value indexedObj = {};
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
        if (indexExpr->indexExpression->type == ASTNodeType::INDEX) {
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
      } else if (std::holds_alternative<k_hashmap>(indexedObj)) {
        auto hashObj = std::get<k_hashmap>(indexedObj);

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

  if (std::holds_alternative<k_hashmap>(object)) {
    auto hash = std::get<k_hashmap>(object);

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

k_value KInterpreter::visit(const ConstAssignmentNode* node) {
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

k_value KInterpreter::visit(const AssignmentNode* node) {
  auto frame = callStack.top();
  auto left = interpret(node->left.get());
  auto value = interpret(node->initializer.get());
  auto type = node->op;
  auto name = node->name;

  if (type == KName::Ops_Assign) {
    if (name == Keywords.Global || ctx->hasConstant(name)) {
      throw IllegalNameError(node->token, name);
    }

    if (std::holds_alternative<k_lambda>(value)) {
      // WIP: need to work on this.
      const auto& lambdaId = std::get<k_lambda>(value)->identifier;
      ctx->addLambda(name, std::move(ctx->getLambdas().at(lambdaId)));
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
  const auto& name = node->name;

  if (frame->inObjectContext() && name.at(0) == '@') {
    return frame->getObjectContext()->instanceVariables[name];
  }

  if (frame->hasVariable(name)) {
    return frame->variables[name];
  } else if (ctx->hasStruct(name)) {
    return std::make_shared<StructRef>(name);
  } else if (ctx->hasLambda(name)) {
    return std::make_shared<LambdaRef>(name);
  } else if (ctx->hasMappedLambda(name)) {
    const auto& mappedId = ctx->getMappedLambda(name);
    if (ctx->hasLambda(mappedId)) {
      return std::make_shared<LambdaRef>(mappedId);
    }
  } else if (ctx->hasConstant(name)) {
    return ctx->getConstants().at(name);
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
  auto hash = std::make_shared<Hashmap>();
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

k_value KInterpreter::visit(const PrintXyNode* node) {
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
    return stringSlice(node->token, slice, object);
  } else if (std::holds_alternative<k_list>(object)) {
    return listSlice(node->token, slice, object);
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

  if (node->indexExpression->type == ASTNodeType::INDEX) {
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
    } else if (std::holds_alternative<k_hashmap>(object)) {
      auto hash = std::get<k_hashmap>(object);

      if (!hash->hasKey(indexValue)) {
        throw HashKeyError(node->token, Serializer::serialize(indexValue));
      }

      return hash->get(indexValue);
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
  k_value result = {};

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

k_value KInterpreter::visit(const CaseNode* node) {
  k_value testValue = interpret(node->testValue.get());

  for (const auto& whenNode : node->whenNodes) {
    k_value whenCondition = interpret(whenNode->condition.get());

    if (std::get<bool>(MathImpl.do_eq_comparison(testValue, whenCondition))) {
      auto frame = callStack.top();
      k_value result;
      for (const auto& stmt : whenNode->body) {
        result = interpret(stmt.get());
        if (frame->isFlagSet(FrameFlags::Return)) {
          return frame->returnValue;
        }
      }

      return result;
    }
  }

  k_value result;

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

k_value KInterpreter::hashLoop(const ForLoopNode* node, const k_hashmap& hash) {
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

k_value KInterpreter::visit(const ForLoopNode* node) {
  k_value dataSetValue = interpret(node->dataSet.get());

  if (std::holds_alternative<k_list>(dataSetValue)) {
    return listLoop(node, std::get<k_list>(dataSetValue));
  }

  if (std::holds_alternative<k_hashmap>(dataSetValue)) {
    return hashLoop(node, std::get<k_hashmap>(dataSetValue));
  }

  throw InvalidOperationError(node->token,
                              "Expected a list value in for-loop.");
}

k_value KInterpreter::visit(const WhileLoopNode* node) {
  k_value result;
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

  if (hasAlias && frame->hasVariable(aliasName)) {
    frame->variables.erase(aliasName);
  }

  frame->clearFlag(FrameFlags::InLoop);

  return result;
}

k_value KInterpreter::visit(const TryNode* node) {
  bool requireDrop = false;
  k_value returnValue;
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
          catchFrame->variables[errorTypeName] = e.getError();
        }

        if (node->errorMessage) {
          errorMessageName = id(node->errorMessage.get());
          catchFrame->variables[errorMessageName] = e.getMessage();
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

k_value KInterpreter::visit(const LambdaCallNode* node) {
  auto lambdaName =
      std::get<k_lambda>(interpret(node->lambdaNode.get()))->identifier;
  k_value result;
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

k_value KInterpreter::visit(const LambdaNode* node) {
  std::vector<std::pair<k_string, k_value>> parameters;
  std::unordered_set<k_string> defaultParameters;
  auto tmpId = getTemporaryId();

  parameters.reserve(node->parameters.size());
  for (const auto& pair : node->parameters) {
    auto paramName = pair.first;
    k_value paramValue = {};
    if (pair.second) {
      paramValue = interpret(pair.second.get());
      defaultParameters.emplace(paramName);
    }

    std::pair<k_string, k_value> param(paramName, paramValue);
    parameters.emplace_back(param);
  }

  auto lambda = std::make_unique<KLambda>(node->clone());
  lambda->parameters = parameters;
  lambda->defaultParameters = defaultParameters;
  lambda->typeHints = node->typeHints;
  lambda->returnTypeHint = node->returnTypeHint;

  ctx->addLambda(tmpId, std::move(lambda));
  ctx->addMappedLambda(tmpId, tmpId);

  return std::make_shared<LambdaRef>(tmpId);
}

k_value KInterpreter::visit(const StructNode* node) {
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

k_value KInterpreter::visit(const VariableDeclarationNode* node) {
  // dereference typeHints for quicker access
  const auto& typeHints = node->typeHints;

  auto varCount = 0;  // a simple counter for the `TypeError` error message
  auto hasDefaultValue =
      false;  // a flag to determine if a variable has an initializer

  // we're going to be injecting these into the stack frame
  auto& frame = callStack.top();

  // for each declared variable
  for (const auto& pair : node->variables) {
    k_value value = {};      // a variable value
    auto name = pair.first;  // grab the name
    ++varCount;              // increment the counter (starts at 1)
    hasDefaultValue = pair.second != nullptr;

    // if there is a default value, grab it
    if (hasDefaultValue) {
      value = interpret(pair.second.get());
      // if the value is a lambda, register to the lambda map
      if (std::holds_alternative<k_lambda>(value)) {
        const auto& lambdaName = std::get<k_lambda>(value)->identifier;
        ctx->addMappedLambda(name, lambdaName);
      }
    } else {
      // default to null
      value = std::make_shared<Null>();
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
            value = false;
            break;

          case KName::Types_Integer:
            value = static_cast<k_int>(0);
            break;

          case KName::Types_Float:
            value = static_cast<double>(0);
            break;

          case KName::Types_String:
            value = k_string("");
            break;

          case KName::Types_List:
            value = std::make_shared<List>();
            break;

          case KName::Types_Hash:
            value = std::make_shared<Hashmap>();
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

k_value KInterpreter::visit(const FunctionDeclarationNode* node) {
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
  std::vector<std::pair<k_string, k_value>> parameters;
  std::unordered_set<k_string> defaultParameters;
  auto typeHints = node->typeHints;

  parameters.reserve(node->parameters.size());
  auto paramCount = 0;

  for (const auto& pair : node->parameters) {
    k_value paramValue = {};
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

k_value KInterpreter::executeInstanceMethod(const FunctionCallNode* node,
                                            bool& requireDrop) {
  auto frame = callStack.top();
  if (!frame->inObjectContext()) {
    throw InvalidContextError(node->token);
  }

  auto& obj = frame->getObjectContext();
  auto& struc = ctx->getStructs().at(obj->structName);
  auto& strucMethods = struc->methods;
  const auto functionName = node->functionName;

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

k_value KInterpreter::executeInstanceMethodFunction(
    std::unordered_map<k_string, std::unique_ptr<KFunction>>& strucMethods,
    const FunctionCallNode* node, bool& requireDrop) {
  const auto& functionName = node->functionName;
  const auto& func = strucMethods[functionName];
  auto defaultParameters = func->defaultParameters;
  auto functionFrame = createFrame(functionName);
  k_value result = {};

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
  for (size_t i = 0; i < func->parameters.size(); ++i) {
    const auto& param = func->parameters[i];
    k_value argValue = {};
    if (i < node->arguments.size()) {
      const auto& arg = node->arguments[i];
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

    if (std::holds_alternative<k_lambda>(argValue)) {
      auto lambdaId = std::get<k_lambda>(argValue)->identifier;
      ctx->addMappedLambda(param.first, lambdaId);
    } else {
      functionFrame->variables[param.first] = argValue;
    }
  }
}

k_value KInterpreter::visit(const FunctionCallNode* node) {
  k_value result;

  if (node->functionName == "foo::bar") {
    std::cout << "";
  }

  auto callableType = getCallable(node->token, node->functionName);

  if (callableType == KCallableType::Builtin) {
    return callBuiltinMethod(node);
  }

  auto requireDrop = false;

  try {
    if (callableType == KCallableType::Method) {
      result = executeInstanceMethod(node, requireDrop);
    } else if (callableType == KCallableType::Function) {
      const auto& functionName = node->functionName;
      const auto& func = ctx->getFunctions().at(functionName);
      auto defaultParameters = func->defaultParameters;
      auto functionFrame = createFrame(functionName);
      const auto& typeHints = func->typeHints;
      const auto& returnTypeHint = func->returnTypeHint;

      prepareFunctionCall(func, node, defaultParameters, typeHints,
                          functionFrame);

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
        throw TypeError(node->token,
                        "Expected type `" +
                            Serializer::get_typename_string(returnTypeHint) +
                            "` for return type of `" + functionName +
                            "` but received `" +
                            Serializer::get_value_type_string(result) + "`.");
      }
    } else if (callableType == KCallableType::Lambda) {
      result = callLambda(node->token, node->functionName, node->arguments,
                          requireDrop);
    }

    dropFrame();
  } catch (const KiwiError& e) {
    if (requireDrop) {
      dropFrame();
    }
    throw;
  }

  return result;
}

k_value KInterpreter::callLambda(
    const Token& token, const k_string& lambdaName,
    const std::vector<std::unique_ptr<ASTNode>>& args, bool& requireDrop) {
  auto lambdaFrame = createFrame(lambdaName);
  k_string targetLambda = lambdaName;
  k_value result;

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

k_value KInterpreter::callLambda(const Token& token, const k_string& lambdaName,
                                 const std::vector<k_value>& args) {
  auto lambdaFrame = createFrame(lambdaName);
  k_string targetLambda = lambdaName;
  k_value result;

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
  for (size_t i = 0; i < func->parameters.size(); ++i) {
    const auto& param = func->parameters[i];
    k_value argValue = {};
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
    const std::pair<k_string, k_value>& param, k_value& argValue,
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

  if (std::holds_alternative<k_lambda>(argValue)) {
    auto lambdaId = std::get<k_lambda>(argValue)->identifier;
    ctx->addMappedLambda(param.first, lambdaId);
  } else {
    lambdaFrame->variables[param.first] = argValue;
  }
}

void KInterpreter::prepareLambdaCall(
    const std::unique_ptr<KLambda>& func, const std::vector<k_value>& args,
    std::unordered_set<k_string>& defaultParameters, const Token& token,
    k_string& targetLambda,
    const std::unordered_map<k_string, KName>& typeHints,
    const k_string& lambdaName, std::shared_ptr<CallStackFrame>& lambdaFrame) {
  for (size_t i = 0; i < func->parameters.size(); ++i) {
    const auto& param = func->parameters[i];
    k_value argValue = {};
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

    if (std::holds_alternative<k_lambda>(argValue)) {
      auto lambdaId = std::get<k_lambda>(argValue)->identifier;
      ctx->addMappedLambda(param.first, lambdaId);
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

k_value KInterpreter::callFunction(
    const std::unique_ptr<KFunction>& function,
    const std::vector<std::unique_ptr<ASTNode>>& args, const Token& token,
    const k_string& functionName) {
  auto defaultParameters = function->defaultParameters;
  auto functionFrame = createFrame(functionName);

  const auto& typeHints = function->typeHints;
  const auto& returnTypeHint = function->returnTypeHint;

  k_value result;
  bool requireDrop = false;

  try {
    for (size_t i = 0; i < function->parameters.size(); ++i) {
      const auto& param = function->parameters[i];
      k_value argValue = {};
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
    const std::pair<k_string, k_value>& param, k_value& argValue,
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

  if (std::holds_alternative<k_lambda>(argValue)) {
    auto lambdaId = std::get<k_lambda>(argValue)->identifier;
    ctx->addMappedLambda(param.first, lambdaId);
  } else {
    functionFrame->variables[param.first] = argValue;
  }
}

k_value KInterpreter::executeFunctionBody(
    const std::unique_ptr<KFunction>& function) {
  k_value result;
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

k_value KInterpreter::visit(const MethodCallNode* node) {
  auto object = interpret(node->object.get());

  if (std::holds_alternative<k_object>(object)) {
    return callObjectMethod(node, std::get<k_object>(object));
  } else if (std::holds_alternative<k_struct>(object)) {
    return callStructMethod(node, std::get<k_struct>(object));
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
    return obj;
  }

  return result;
}

k_value KInterpreter::callObjectMethod(const MethodCallNode* node,
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
    return obj;
  }

  return result;
}

k_value KInterpreter::callStructMethod(const MethodCallNode* node,
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

k_value KInterpreter::executeStructMethod(
    std::unordered_map<k_string, std::unique_ptr<KFunction>>& methods,
    k_string& methodName, std::shared_ptr<CallStackFrame>& frame,
    const MethodCallNode* node, const k_struct& struc) {
  auto& function = methods[methodName];
  k_object obj = std::make_shared<Object>();
  bool isCtor = methodName == Keywords.New;

  auto oldObjectContext = frame->getObjectContext();
  bool contextSwitch = false;

  if (!function && isCtor) {
    return obj;  // default constructor
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

void KInterpreter::handleWebServerRequest(int webhookID, k_hashmap requestHash,
                                          k_string& redirect, k_string& content,
                                          k_string& contentType, int& status) {
  k_value result;
  bool requireDrop = false;

  try {
    auto webhook = ctx->getWebHook(webhookID);
    auto webhookFrame = createFrame(Keywords.Spawn);

    auto& lambda = ctx->getLambdas().at(webhook);

    for (const auto& param : lambda->parameters) {
      webhookFrame->variables[param.first] = requestHash;
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

    if (std::holds_alternative<k_hashmap>(result)) {
      auto responseHash = std::get<k_hashmap>(result);
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
    if (requireDrop) {
      dropFrame();
    }
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

k_value KInterpreter::interpretWebServerGet(const Token& token,
                                            std::vector<k_value>& args) {
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

k_value KInterpreter::interpretWebServerPost(const Token& token,
                                             std::vector<k_value>& args) {
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

k_value KInterpreter::interpretWebServerListen(const Token& token,
                                               std::vector<k_value>& args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, WebServerBuiltins.Listen);
  }

  auto host = get_string(token, args.at(0));
  auto port = get_integer(token, args.at(1));

  ctx->getServer().listen(host, static_cast<int>(port));

  auto hash = std::make_shared<Hashmap>();
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

  if (!File::directoryExists(token, publicDir)) {
    return false;
  }

  ctx->getServer().set_mount_point(endpoint, publicDir);

  return true;
}

std::vector<k_string> KInterpreter::getWebServerEndpointList(const Token& token,
                                                             k_value& arg) {
  std::vector<k_string> endpointList;

  if (std::holds_alternative<k_string>(arg)) {
    endpointList.emplace_back(get_string(token, arg));
  } else if (std::holds_alternative<k_list>(arg)) {
    for (const auto& el : std::get<k_list>(arg)->elements) {
      if (std::holds_alternative<k_string>(el)) {
        auto endpoint = get_string(token, el);
        if (std::find(endpointList.begin(), endpointList.end(), endpoint) ==
            endpointList.end()) {
          endpointList.emplace_back(endpoint);
        }
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
    requestHash->add(x.first, x.second);
  }

  auto pathParamsHash = std::make_shared<Hashmap>();
  for (const auto& pair : req.path_params) {
    pathParamsHash->add(pair.first, pair.second);
  }

  auto paramsHash = std::make_shared<Hashmap>();
  for (auto it = params.begin(); it != params.end(); ++it) {
    const auto& x = *it;
    paramsHash->add(x.first, x.second);
  }

  auto filesHash = std::make_shared<Hashmap>();

  for (const auto& file : req.files) {
    auto fileHash = std::make_shared<Hashmap>();
    fileHash->add("content", file.second.content);
    fileHash->add("content_type", file.second.content_type);
    fileHash->add("filename", file.second.filename);
    fileHash->add("name", file.second.name);
    filesHash->add(file.first, fileHash);
  }

  requestHash->add("body", req.body);
  requestHash->add("files", filesHash);
  requestHash->add("path", req.path);
  requestHash->add("path_params", pathParamsHash);
  requestHash->add("params", paramsHash);

  return requestHash;
}

k_value KInterpreter::interpolateString(const Token& token,
                                        const k_string& input) {
  Parser parser(true);
  Lexer lexer(token.getFile(), input);
  auto tempStream = lexer.getTokenStream();
  auto ast = parser.parseTokenStream(tempStream, true);

  return interpret(ast.get());
}

k_value KInterpreter::interpretReflectorBuiltin(const Token& token,
                                                const KName& builtin,
                                                std::vector<k_value>& args) {
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

k_value KInterpreter::interpretReflectorRFFlags(const Token& token,
                                                std::vector<k_value>& args) {
  if (args.size() > 1) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RFFlags);
  }

  std::vector<k_value> list;
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
        list.push_back(static_cast<k_int>(FrameFlags::Break));
      }

      if (frame->isFlagSet(FrameFlags::InLambda)) {
        list.push_back(static_cast<k_int>(FrameFlags::InLambda));
      }

      if (frame->isFlagSet(FrameFlags::InLoop)) {
        list.push_back(static_cast<k_int>(FrameFlags::InLoop));
      }

      if (frame->isFlagSet(FrameFlags::InObject)) {
        list.push_back(static_cast<k_int>(FrameFlags::InObject));
      }

      if (frame->isFlagSet(FrameFlags::InTry)) {
        list.push_back(static_cast<k_int>(FrameFlags::InTry));
      }

      if (frame->isFlagSet(FrameFlags::Next)) {
        list.push_back(static_cast<k_int>(FrameFlags::Next));
      }

      if (frame->isFlagSet(FrameFlags::None)) {
        list.push_back(static_cast<k_int>(FrameFlags::None));
      }

      if (frame->isFlagSet(FrameFlags::Return)) {
        list.push_back(static_cast<k_int>(FrameFlags::Return));
      }

      if (frame->isFlagSet(FrameFlags::SubFrame)) {
        list.push_back(static_cast<k_int>(FrameFlags::SubFrame));
      }

      break;
    }

    frameCounter++;
    tempStack.pop();
  }

  return std::make_shared<List>(list);
}

k_value KInterpreter::interpretReflectorRRetVal(const Token& token,
                                                std::vector<k_value>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RRetVal);
  }

  return callStack.top()->returnValue;
}

k_value KInterpreter::interpretReflectorRList(const Token& token,
                                              std::vector<k_value>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RList);
  }

  auto rlist = std::make_shared<Hashmap>();
  auto rlistPackages = std::make_shared<List>();
  auto rlistStructs = std::make_shared<List>();
  auto rlistFunctions = std::make_shared<List>();
  auto rlistStack = std::make_shared<List>();

  rlistPackages->elements.reserve(ctx->getPackages().size());
  rlistStructs->elements.reserve(ctx->getStructs().size());
  rlistFunctions->elements.reserve(ctx->getFunctions().size());
  rlistStack->elements.reserve(callStack.size());

  for (const auto& m : ctx->getMethods()) {
    rlistFunctions->elements.emplace_back(m.first);
  }

  for (const auto& p : ctx->getPackages()) {
    rlistPackages->elements.emplace_back(p.first);
  }

  for (const auto& c : ctx->getStructs()) {
    rlistStructs->elements.emplace_back(c.first);
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
  sort_list(*rlistStructs);
  sort_list(*rlistFunctions);
  std::reverse(rlistStack->elements.begin(), rlistStack->elements.end());

  rlist->add("packages", rlistPackages);
  rlist->add("structs", rlistStructs);
  rlist->add("functions", rlistFunctions);
  rlist->add("stack", rlistStack);

  return rlist;
}

k_value KInterpreter::interpretReflectorRObject(const Token& token,
                                                std::vector<k_value>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RObject);
  }

  auto frame = callStack.top();
  if (frame->inObjectContext()) {
    return Serializer::serialize(frame->getObjectContext());
  }

  return {};
}

k_value KInterpreter::interpretReflectorRStack(const Token& token,
                                               std::vector<k_value>& args) {
  if (args.size() != 0) {
    throw BuiltinUnexpectedArgumentError(token, ReflectorBuiltins.RStack);
  }

  std::stack<k_string> tempStack(funcStack);
  std::vector<k_value> stackNames;
  stackNames.reserve(tempStack.size());

  while (!tempStack.empty()) {
    stackNames.emplace_back(tempStack.top());
    tempStack.pop();
  }

  return std::make_shared<List>(stackNames);
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

k_value KInterpreter::interpretSignalBuiltin(const Token& token,
                                             const KName& op,
                                             std::vector<k_value> args) {
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
    std::vector<k_value> args;
    this->callLambda(token, lambda, args);
  }
}

k_value KInterpreter::interpretSignalTrap(const Token& token,
                                          std::vector<k_value> args) {
  if (args.size() != 2) {
    throw BuiltinUnexpectedArgumentError(token, SignalBuiltins.Trap);
  }

  int signum = static_cast<int>(get_integer(token, args.at(0)));

  if (!std::holds_alternative<k_lambda>(args.at(1))) {
    throw InvalidOperationError(
        token,
        "Expected a lambda for parameter 2 of `" + SignalBuiltins.Trap + "`.");
  }

  auto lambdaRef = std::get<k_lambda>(args.at(1));

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

  return static_cast<k_int>(res);
}

k_value KInterpreter::interpretSignalRaise(const Token& token,
                                           const std::vector<k_value>& args) {
  if (args.size() != 1) {
    throw BuiltinUnexpectedArgumentError(token, SignalBuiltins.Raise);
  }

  int sig = static_cast<int>(std::get<k_int>(args[0]));
  int res = raise(sig);

  if (res != 0) {
    throw SignalError(token, "Failed to raise signal");
  }

  return static_cast<k_int>(res);
}

k_value KInterpreter::interpretSignalSend(const Token& token,
                                          const std::vector<k_value>& args) {
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

  return static_cast<k_int>(res);
}

k_value KInterpreter::interpretListBuiltin(const Token& token, k_value& object,
                                           const KName& op,
                                           std::vector<k_value> args) {
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

  if (args.size() == 1) {
    auto arg = args.at(0);
    if (!std::holds_alternative<k_lambda>(arg)) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }
    auto lambdaRef = std::get<k_lambda>(arg);

    if (!ctx->hasLambda(lambdaRef->identifier)) {
      throw InvalidOperationError(
          token, "Unrecognized lambda '" + lambdaRef->identifier + "'.");
    }

    auto& lambda = ctx->getLambdas().at(lambdaRef->identifier);
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
  } else if (args.size() == 2 && op == KName::Builtin_List_Reduce) {
    auto arg = args.at(1);

    if (!std::holds_alternative<k_lambda>(arg)) {
      throw InvalidOperationError(
          token, "Expected a lambda in specialized list builtin.");
    }
    auto lambdaRef = std::get<k_lambda>(arg);

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
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      frame->variables[indexVariable] = static_cast<k_int>(i);
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

  const auto& decl = lambda->decl;
  const auto& elements = list->elements;
  std::vector<k_value> resultList;
  k_value result = {};

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
  const auto& decl = lambda->decl;
  k_value result;

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
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      frame->variables[indexVariable] = static_cast<k_int>(i);
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
  const auto& decl = lambda->decl;
  const auto& elements = list->elements;
  std::vector<k_value> resultList;

  for (size_t i = 0; i < elements.size(); ++i) {
    frame->variables[valueVariable] = elements.at(i);

    if (hasIndexVariable) {
      frame->variables[indexVariable] = static_cast<k_int>(i);
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

  return std::make_shared<List>(resultList);
}

bool KInterpreter::shouldUpdateFrameVariables(
    const k_string& varName, const std::shared_ptr<CallStackFrame> nextFrame) {
  return nextFrame->hasVariable(varName);
}

void KInterpreter::updateVariablesInCallerFrame(
    const std::unordered_map<k_string, k_value>& variables,
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
  if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
    throw IndexError(token, "Start index must be an integer.");
  } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
    throw IndexError(token, "Stop index must be an integer.");
  } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
    throw IndexError(token, "Step value must be an integer.");
  }

  int start = std::get<k_int>(slice.indexOrStart);
  int stop = std::get<k_int>(slice.stopIndex);
  int step = std::get<k_int>(slice.stepValue);

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

k_value KInterpreter::stringSlice(const Token& token, SliceIndex& slice,
                                  const k_value& value) {
  auto string = std::get<k_string>(value);
  auto list = std::make_shared<List>();

  auto& elements = list->elements;
  elements.reserve(string.size());
  k_string temp(1, '\0');
  for (const char& c : string) {
    temp[0] = c;
    elements.emplace_back(temp);
  }

  auto sliced = listSlice(token, slice, list);

  if (std::holds_alternative<k_list>(sliced)) {
    auto slicedlist = std::get<k_list>(sliced)->elements;
    std::ostringstream sv;

    for (auto it = slicedlist.begin(); it != slicedlist.end(); ++it) {
      sv << Serializer::serialize(*it);
    }

    return sv.str();
  }

  return Serializer::serialize(sliced);
}

k_value KInterpreter::listSlice(const Token& token, const SliceIndex& slice,
                                const k_value& value) {
  auto list = std::get<k_list>(value);
  auto& elements = list->elements;

  if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
    throw IndexError(token, "Start index must be an integer.");
  } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
    throw IndexError(token, "Stop index must be an integer.");
  } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
    throw IndexError(token, "Step value must be an integer.");
  }

  int start = static_cast<int>(std::get<k_int>(slice.indexOrStart)),
      stop = static_cast<int>(std::get<k_int>(slice.stopIndex)),
      step = static_cast<int>(std::get<k_int>(slice.stepValue));

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
  return slicedList;
}

#endif
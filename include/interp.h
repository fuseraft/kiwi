#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <unordered_map>
#include <stack>
#include "concurrency/task.h"
#include "errors/error.h"
#include "errors/handler.h"
#include "errors/state.h"
#include "logging/logger.h"
#include "objects/class.h"
#include "objects/conditional.h"
#include "objects/method.h"
#include "objects/module.h"
#include "objects/sliceindex.h"
#include "parsing/builtins.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "parsing/keywords.h"
#include "util/file.h"
#include "util/string.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"
#include "globals.h"
#include "interp_builtin.h"
#include "interp_helper.h"
#include "stackframe.h"

class Interpreter {
 public:
  Interpreter() {}
  ~Interpreter() {}

  void setKiwiArgs(const std::unordered_map<std::string, std::string>& args) {
    kiwiArgs = args;
  }

  int interpretKiwi(const std::string& kiwiCode) {
    Lexer lexer("", kiwiCode);
    return interpret(lexer);
  }

  int interpretScript(const std::string& path) {
    auto content = File::readFile(path);
    if (content.empty()) {
      return -1;
    }

    auto cwd = File::getCurrentDirectory();
    auto scriptDir = File::getParentPath(path);
    File::setCurrentDirectory(scriptDir);

    Lexer lexer(path, content);
    int result = interpret(lexer);

    File::setCurrentDirectory(cwd);
    return result;
  }

  void preserveMainStackFrame() { preservingMainStackFrame = true; }

 private:
  bool preservingMainStackFrame = false;

  k_int interpretAsyncMethodInvocation(
      std::shared_ptr<CallStackFrame> codeFrame, Method& method) {
    auto taskFunc = [this, codeFrame, method]() -> Value {
      callStack.push(codeFrame);
      streamStack.push(std::make_shared<TokenStream>(method.getCode()));

      interpretStackFrame();

      if (!callStack.empty()) {
        return callStack.top()->returnValue;
      }

      return {};
    };

    return task.addTask(taskFunc);
  }

  Value interpretAwait(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip "await"

    auto term = stream->current();
    if (term.getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(term, "Expected identifier near `await`.");
    }

    auto identifier = term.getText();
    auto taskId = interpretMethodInvocation(stream, frame, identifier, true);
    if (!std::holds_alternative<k_int>(taskId)) {
      throw ConversionError(term, "Expected a task.");
    }

    return task.getTaskResult(std::get<k_int>(taskId));
  }

  int interpret(Lexer& lexer) {
    auto stream = std::make_shared<TokenStream>(lexer.getAllTokens());
    return interpret(stream);
  }

  int interpret(std::shared_ptr<TokenStream> stream) {
    if (stream->empty()) {
      return 0;
    }

    callStack.push(std::make_shared<CallStackFrame>());
    streamStack.push(stream);

    interpretStackFrame();

    if (!preservingMainStackFrame && callStack.size() == 1) {
      callStack.pop();
    }

    return 0;
  }

  /// @brief Pops and returns the top of the call stack.
  /// @return A stack frame.
  std::shared_ptr<CallStackFrame> popTop() {
    streamStack.pop();
    callStack.pop();
    auto& callerFrame = callStack.top();
    return callerFrame;
  }

  void interpretStackFrame() {
    auto& frame = callStack.top();
    auto& stream = streamStack.top();

    while (stream->canRead()) {
      try {
        interpretToken(stream, frame);
      } catch (const KiwiError& e) {
        if (frame->isFlagSet(FrameFlags::InTry)) {
          frame->setErrorState(e);
        } else {
          ErrorHandler::handleError(e);
          if (!preservingMainStackFrame) {
            while (task.hasActiveTasks()) {}
            exit(1);
          }
        }
      } catch (const std::exception& e) {
        ErrorHandler::handleFatalError(e);
      }

      if (handleFrameFlags(frame)) {
        return;
      }
    }

    if (callStack.size() < 2) {
      return;
    }

    handleFrameExit(frame);
  }

  bool handleFrameFlags(std::shared_ptr<CallStackFrame>& frame) {
    if (frame->isErrorStateSet()) {
      ++streamStack.top()->position;
    } else if (frame->isLoopControlFlagSet() ||
               frame->isFlagSet(FrameFlags::ReturnFlag)) {
      if (callStack.size() > 1) {
        if (frame->isLoopControlFlagSet()) {
          handleLoopControl(frame);
          return true;
        } else if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
          handleFrameReturn(frame);
          return true;
        }
      } else {
        callStack.pop();
        return true;
      }
    }

    return false;
  }

  void handleFrameExit(std::shared_ptr<CallStackFrame>& frame) {
    auto returnValue = std::move(frame->returnValue);
    bool doUpdate = true;
    bool inObjectContext = frame->inObjectContext();

    if (inObjectContext && std::holds_alternative<k_int>(returnValue) &&
        std::get<k_int>(returnValue) == 0) {
      returnValue = std::move(frame->objectContext);
      doUpdate = false;
    }

    auto topVariables = std::move(frame->variables);
    auto callerFrame = popTop();

    if (inObjectContext) {
      callerFrame->returnValue = returnValue;
    }

    if (doUpdate) {
      InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
    }
  }

  void handleFrameReturn(std::shared_ptr<CallStackFrame>& frame) {
    auto returnValue = std::move(frame->returnValue);
    auto topVariables = std::move(frame->variables);
    auto callerFrame = popTop();

    callerFrame->returnValue = returnValue;

    if (callerFrame->isFlagSet(FrameFlags::SubFrame)) {
      callerFrame->setFlag(FrameFlags::ReturnFlag);
    }

    InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
  }

  void handleLoopControl(std::shared_ptr<CallStackFrame>& frame) {
    bool loopBreak = frame->isFlagSet(FrameFlags::LoopBreak);
    bool loopContinue = frame->isFlagSet(FrameFlags::LoopContinue);
    auto topVariables = std::move(frame->variables);
    auto callerFrame = popTop();

    InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);

    if (loopBreak) {
      callerFrame->setFlag(FrameFlags::LoopBreak);
    } else if (loopContinue) {
      callerFrame->setFlag(FrameFlags::LoopContinue);
    }
  }

  std::shared_ptr<CallStackFrame> buildSubFrame(
      std::shared_ptr<CallStackFrame> frame, bool isMethodInvocation = false) {
    auto subFrame = std::make_shared<CallStackFrame>();

    if (!isMethodInvocation) {
      for (const auto& pair : frame->variables) {
        subFrame->variables[pair.first] = pair.second;
      }
    }

    if (frame->inObjectContext()) {
      for (const auto& pair : frame->getObjectContext()->instanceVariables) {
        subFrame->variables[pair.first] = pair.second;
      }

      subFrame->setObjectContext(frame->getObjectContext());
    }

    return subFrame;
  }

  Method getLambda(std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CallStackFrame> frame) {
    switch (stream->current().getType()) {
      case TokenType::IDENTIFIER: {
        auto lambdaName = stream->current().getText();
        stream->next();  // Skip identifier.
        if (frame->hasAssignedLambda(lambdaName)) {
          return frame->getAssignedLambda(lambdaName);
        } else {
          auto tempStack(callStack);
          while (!tempStack.empty()) {
            auto& outerFrame = tempStack.top();
            if (outerFrame->hasAssignedLambda(lambdaName)) {
              return outerFrame->getAssignedLambda(lambdaName);
            }
            tempStack.pop();
          }
        }
      } break;

      case TokenType::LAMBDA:
        return interpretLambda(stream, frame);

      default:
        break;
    }

    throw InvalidOperationError(stream->current(), "Expected lambda.");
  }

  Method interpretLambda(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip "lambda"
    Method lambda;
    lambda.setName(InterpHelper::getTemporaryId());

    interpretMethodParameters(stream, frame, lambda);

    if (!stream->matchsub(SubTokenType::KW_Do)) {
      throw SyntaxError(stream->current(),
                        "Expected `do` in lambda expression.");
    }

    for (const auto& t : InterpHelper::collectBodyTokens(stream)) {
      lambda.addToken(t);
    }

    lambda.setFlag(MethodFlags::Lambda);

    return lambda;
  }

  void interpretHashLoop(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame,
                         Value& collectionValue, const bool& hasIndexVariable,
                         const std::string& itemVariableName,
                         const std::string& indexVariableName) {
    auto& collection = std::get<std::shared_ptr<Hash>>(collectionValue);
    auto loopTokens = InterpHelper::collectBodyTokens(stream);

    // Execute the loop
    for (const auto& key : collection->keys) {
      if (frame->isLoopControlFlagSet()) {
        if (frame->isFlagSet(FrameFlags::LoopBreak)) {
          break;
        } else if (frame->isFlagSet(FrameFlags::LoopContinue)) {
          frame->clearFlag(FrameFlags::LoopContinue);
          continue;
        }
      }

      frame->variables[indexVariableName] = key;
      if (hasIndexVariable) {
        frame->variables[itemVariableName] = collection->get(key);
      }

      auto loopStream = std::make_shared<TokenStream>(loopTokens);
      auto loopFrame = buildSubFrame(frame);
      callStack.push(loopFrame);
      streamStack.push(loopStream);

      interpretStackFrame();
    }
  }

  void interpretListLoop(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame,
                         Value& collectionValue, const bool& hasIndexVariable,
                         const std::string& itemVariableName,
                         const std::string& indexVariableName) {
    const auto& collection = std::get<std::shared_ptr<List>>(collectionValue);
    auto loopTokens = InterpHelper::collectBodyTokens(stream);

    // Execute the loop
    size_t index = 0;
    for (const auto& item : collection->elements) {
      if (frame->isLoopControlFlagSet()) {
        if (frame->isFlagSet(FrameFlags::LoopBreak)) {
          break;
        } else if (frame->isFlagSet(FrameFlags::LoopContinue)) {
          frame->clearFlag(FrameFlags::LoopContinue);
          continue;
        }
      }

      frame->variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame->variables[indexVariableName] = static_cast<int>(index);
      }

      auto loopStream = std::make_shared<TokenStream>(loopTokens);
      callStack.push(buildSubFrame(frame));
      streamStack.push(loopStream);

      interpretStackFrame();

      index++;
    }
  }

  void interpretForLoop(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    std::string itemVariableName, indexVariableName;
    bool hasIndexVariable = false;

    // Check if the loop includes an index variable.
    if (stream->current().getType() == TokenType::IDENTIFIER) {
      itemVariableName = stream->current().getText();
      stream->next();  // Skip the item variable name

      if (stream->current().getType() == TokenType::COMMA) {
        stream->next();  // Skip ','
        if (stream->current().getType() == TokenType::IDENTIFIER) {
          hasIndexVariable = true;
          indexVariableName =
              stream->current().getText();  // Get the index variable name
          stream->next();                   // Skip the index variable name
        }
      }
    }

    if (!stream->matchsub(SubTokenType::KW_In)) {
      throw SyntaxError(stream->current(),
                        "Expected 'in' after loop variables.");
    }

    auto collectionValue = interpretExpression(stream, frame);

    if (!stream->matchsub(SubTokenType::KW_Do)) {
      throw SyntaxError(stream->current(), "Expected `do` in `for` loop.");
    }

    if (std::holds_alternative<std::shared_ptr<List>>(collectionValue)) {
      interpretListLoop(stream, frame, collectionValue, hasIndexVariable,
                        itemVariableName, indexVariableName);
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(collectionValue)) {
      interpretHashLoop(stream, frame, collectionValue, hasIndexVariable,
                        indexVariableName, itemVariableName);
    } else {
      throw InvalidOperationError(stream->current(),
                                  "Term is not a List or Hash.");
    }

    frame->clearFlag(FrameFlags::LoopBreak);
    frame->clearFlag(FrameFlags::LoopContinue);
  }

  void interpretWhileLoop(std::shared_ptr<TokenStream> stream,
                          std::shared_ptr<CallStackFrame> frame) {
    std::vector<Token> conditionTokens;
    while (stream->canRead() &&
           stream->current().getSubType() != SubTokenType::KW_Do) {
      conditionTokens.push_back(stream->current());
      stream->next();
    }

    stream->next();  // Skip "do"

    auto loopTokens = InterpHelper::collectBodyTokens(stream);

    auto oldFrame = std::make_shared<CallStackFrame>(*frame);
    auto tempId = InterpHelper::getTemporaryId();
    auto tempAssignment =
        InterpHelper::getTemporaryAssignment(stream->current(), tempId);

    while (true) {
      if (frame->isLoopControlFlagSet()) {
        if (frame->isFlagSet(FrameFlags::LoopBreak)) {
          break;
        } else if (frame->isFlagSet(FrameFlags::LoopContinue)) {
          frame->clearFlag(FrameFlags::LoopContinue);
          continue;
        }
      }

      auto condition = tempAssignment;
      condition.insert(condition.end(), conditionTokens.begin(),
                       conditionTokens.end());  // Append conditionTokens

      auto conditionStream = std::make_shared<TokenStream>(condition);
      auto conditionFrame = buildSubFrame(frame);
      callStack.push(conditionFrame);
      streamStack.push(conditionStream);
      interpretAssignment(conditionStream, conditionFrame, "", true);

      if (conditionFrame->variables.find(tempId) ==
          conditionFrame->variables.end()) {
        throw SyntaxError(stream->current(),
                          "Invalid condition in while-loop.");
      }

      auto value = getVariable(conditionStream, conditionFrame, tempId);
      callStack.pop();

      if (!std::holds_alternative<bool>(value)) {
        throw ConversionError(stream->current());
      }

      if (!std::get<bool>(value)) {
        break;
      }

      auto codeStream = std::make_shared<TokenStream>(loopTokens);
      callStack.push(buildSubFrame(frame));
      streamStack.push(codeStream);

      interpretStackFrame();
      frame = callStack.top();
    }

    frame->clearFlag(FrameFlags::LoopBreak);
    frame->clearFlag(FrameFlags::LoopContinue);

    for (const auto& pair : frame->variables) {
      if (InterpHelper::shouldUpdateFrameVariables(pair.first, oldFrame)) {
        oldFrame->variables[pair.first] = pair.second;
      }
    }

    if (frame->inObjectContext()) {
      for (const auto& pair : frame->getObjectContext()->instanceVariables) {
        if (InterpHelper::shouldUpdateFrameVariables(pair.first, oldFrame)) {
          oldFrame->variables[pair.first] = pair.second;
        }
      }
    }

    frame = oldFrame;
  }

  void interpretLoop(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    const auto& loop = stream->current().getSubType();
    stream->next();  // Skip "while"|"for"

    switch (loop) {
      case SubTokenType::KW_While:
        interpretWhileLoop(stream, frame);
        break;

      case SubTokenType::KW_For:
        interpretForLoop(stream, frame);
        break;

      default:
        break;
    }
  }

  void interpretKeyword(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    const auto& keyword = stream->current().getSubType();
    switch (keyword) {
      case SubTokenType::KW_If:
        interpretConditional(stream, frame);
        break;

      case SubTokenType::KW_Async:
        interpretAsyncMethodDefinition(stream, frame);
        break;

      case SubTokenType::KW_Await:
        interpretAwait(stream, frame);
        break;

      case SubTokenType::KW_Method:
        interpretMethodDefinition(stream, frame);
        break;

      case SubTokenType::KW_Return:
        interpretReturn(stream, frame);
        break;

      case SubTokenType::KW_Exit:
        interpretExit(stream, frame);
        break;

      case SubTokenType::KW_PrintLn:
      case SubTokenType::KW_Print:
        interpretPrint(stream, frame, keyword == SubTokenType::KW_PrintLn);
        break;

      case SubTokenType::KW_Import:
        interpretImport(stream, frame);
        break;

      case SubTokenType::KW_Export:
        interpretExport(stream, frame);
        break;

      case SubTokenType::KW_Module:
        interpretModuleDefinition(stream);
        break;

      case SubTokenType::KW_Delete:
        interpretDelete(stream, frame);
        break;

      case SubTokenType::KW_Abstract:
      case SubTokenType::KW_Class:
        interpretClassDefinition(stream, frame);
        break;

      case SubTokenType::KW_This:
        interpretSelfInvocation(stream, frame);
        break;

      case SubTokenType::KW_Break:
        stream->next();  // Skip "break"
        frame->setFlag(FrameFlags::LoopBreak);
        break;

      case SubTokenType::KW_Next:
        stream->next();  // Skip "next"
        frame->setFlag(FrameFlags::LoopContinue);
        break;

      case SubTokenType::KW_Pass:
        stream->next();  // Skip "pass"
        break;

      case SubTokenType::KW_Try:
        stream->next();  // Skip "try"
        frame->setFlag(FrameFlags::InTry);
        break;

      case SubTokenType::KW_Catch:
        interpretCatch(stream, frame);
        break;

      default:
        if (Keywords.is_loop_keyword(keyword)) {
          interpretLoop(stream, frame);
        } else {
          throw UnrecognizedTokenError(
              stream->current(),
              "Unrecognized token `" + stream->current().getText() + "`.");
        }
        break;
    }
  }

  void interpretQualifiedIdentifier(std::shared_ptr<TokenStream> stream,
                                    std::string& identifier) {
    while (stream->peek().getType() == TokenType::QUALIFIER) {
      stream->next();  // Skip the identifier.
      stream->next();  // Skip the qualifier.
      identifier += Symbols.Qualifier + stream->current().getText();
    }
  }

  Value interpretIdentifierInvocation(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame,
                                      const std::string& identifier) {
    bool methodFound = hasMethod(frame, identifier),
         classFound = hasClass(identifier);

    if (methodFound || classFound) {
      if (classFound) {
        if (stream->peek().getType() != TokenType::DOT) {
          throw SyntaxError(stream->current(),
                            "Invalid syntax near `" + identifier + "`");
        }
        interpretClassMethodInvocation(stream, frame, identifier);
      } else if (methodFound) {
        switch (stream->peek().getType()) {
          case TokenType::COMMA:
          case TokenType::CLOSE_PAREN:
            if (frame->hasAssignedLambda(identifier)) {
              std::shared_ptr<LambdaRef> lambdaRef =
                  std::make_shared<LambdaRef>(identifier);
              return lambdaRef;
            } else {
              throw SyntaxError(stream->current(),
                                "Expected lambda reference.");
            }
            break;

          default:
            interpretMethodInvocation(stream, frame, identifier);
            break;
        }
      }

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);
        return value;
      }
    }

    throw UnknownIdentifierError(stream->current(), identifier);
  }

  Value interpretValueInvocation(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 Value& v) {
    while (stream->current().getType() == TokenType::DOT ||
           stream->current().getType() == TokenType::OPEN_BRACKET) {
      switch (stream->current().getType()) {
        case TokenType::DOT:
          v = interpretDotNotation(stream, frame, v);
          break;

        case TokenType::OPEN_BRACKET: {
          auto slice = interpretSliceIndex(stream, frame, v);
          v = interpretSlice(stream, v, slice);
        } break;

        default:
          break;
      }
    }
    return v;
  }

  Value interpretIdentifier(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame,
                            bool doAssignment = true) {
    auto tokenText = stream->current().getText();
    auto op = stream->current().getSubType();

    interpretQualifiedIdentifier(stream, tokenText);

    if (doAssignment && stream->peek().getType() == TokenType::OPERATOR &&
        (Operators.is_assignment_operator(stream->peek().getSubType()) ||
         stream->peek().getSubType() == SubTokenType::Ops_BitwiseLeftShift)) {
      interpretAssignment(stream, frame, tokenText);
      return 0;
    }

    if (hasVariable(frame, tokenText)) {
      if (stream->peek().getType() == TokenType::OPEN_BRACKET) {
        stream->next();
        interpretSliceAssignment(stream, frame, tokenText);
        return 0;
      }

      auto v = getVariable(stream, frame, tokenText);
      stream->next();

      if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
        auto lambdaRef = std::get<std::shared_ptr<LambdaRef>>(v)->identifier;
        if (frame->hasAssignedLambda(lambdaRef)) {
          return interpretMethodInvocation(stream, frame, lambdaRef);
        } else {
          throw InvalidOperationError(stream->current(),
                                      "Unknown Lambda `" + tokenText + "`.");
        }
      }

      v = interpretValueInvocation(stream, frame, v);

      return v;
    } else if (KiwiBuiltins.is_builtin_method(op)) {
      return interpretBuiltin(stream, frame, op);
    } else {
      return interpretIdentifierInvocation(stream, frame, tokenText);
    }

    throw UnknownIdentifierError(stream->current(), tokenText);
  }

  void interpretCatch(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // SKip "catch"

    std::string errorVariableName;
    Value errorValue;

    if (stream->current().getType() == TokenType::OPEN_PAREN) {
      InterpHelper::interpretParameterizedCatch(stream, frame,
                                                errorVariableName, errorValue);
    }

    std::vector<Token> catchTokens;
    int count = 1;
    while (stream->canRead() && count != 0) {
      if (stream->current().getSubType() == SubTokenType::KW_End) {
        --count;

        // Stop here.
        if (count == 0) {
          stream->next();  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(stream->current().getSubType())) {
        ++count;
      }

      catchTokens.push_back(stream->current());
      stream->next();
    }

    if (frame->isErrorStateSet()) {
      auto catchStream = std::make_shared<TokenStream>(catchTokens);
      auto catchFrame = buildSubFrame(frame);
      if (!errorVariableName.empty() &&
          std::holds_alternative<std::string>(errorValue)) {
        catchFrame->variables[errorVariableName] = errorValue;
      }

      callStack.push(catchFrame);
      streamStack.push(catchStream);
      interpretStackFrame();
      frame->clearFlag(FrameFlags::InTry);
      frame->clearErrorState();
    }
  }

  void interpretToken(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    if (frame->isErrorStateSet()) {
      if (stream->current().getType() == TokenType::KEYWORD &&
          stream->current().getSubType() == SubTokenType::KW_Catch) {
        interpretCatch(stream, frame);
      }
      return;
    }

    switch (stream->current().getType()) {
      case TokenType::COMMENT:
      case TokenType::COMMA:
        stream->next();  // Skip these.
        break;

      case TokenType::KEYWORD:
        interpretKeyword(stream, frame);
        break;

      case TokenType::IDENTIFIER:
        interpretIdentifier(stream, frame);
        break;

      case TokenType::CONDITIONAL:
        interpretConditional(stream, frame);
        break;

      default:
        throw UnrecognizedTokenError(
            stream->current(),
            "Unrecognized token `" + stream->current().getText() + "`.");
    }
  }

  bool hasHomedModule(const std::string& homeName,
                      const std::string& moduleName) {
    for (auto pair : modules) {
      if (pair.first == moduleName && pair.second.hasHome() &&
          pair.second.getHome() == homeName) {
        return true;
      }
    }

    return false;
  }

  bool hasModule(const std::string& name) const {
    return modules.find(name) != modules.end();
  }

  bool hasClass(const std::string& name) const {
    return classes.find(name) != classes.end();
  }

  bool hasMethod(std::shared_ptr<CallStackFrame> frame,
                 const std::string& name) {
    if (frame->inObjectContext()) {
      auto clazz = classes[frame->getObjectContext()->className];
      if (clazz.hasMethod(name)) {
        return true;
      }
    }

    if (frame->hasAssignedLambda(name)) {
      return true;
    }

    return methods.find(name) != methods.end();
  }

  bool hasVariable(std::shared_ptr<CallStackFrame> frame,
                   const std::string& name) const {
    if (frame->variables.find(name) != frame->variables.end()) {
      return true;  // Found in the current frame
    }

    if (frame->inObjectContext()) {
      if (frame->getObjectContext()->instanceVariables.find(name) !=
          frame->getObjectContext()->instanceVariables.end()) {
        return true;
      }
    }

    // Check in outer frames
    std::stack<std::shared_ptr<CallStackFrame>> tempStack(
        callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      auto& outerFrame = tempStack.top();
      if (outerFrame->variables.find(name) != outerFrame->variables.end()) {
        return true;  // Found in an outer frame
      }
      tempStack.pop();
    }

    return false;  // Not found in any scope
  }

  Module getHomedModule(std::shared_ptr<TokenStream> stream,
                        const std::string& homeName,
                        const std::string& moduleName) {
    for (auto pair : modules) {
      if (pair.first == moduleName && pair.second.hasHome() &&
          pair.second.getHome() == homeName) {
        return pair.second;
      }
    }

    throw ModuleUndefinedError(stream->current(), moduleName);
  }

  Module getModule(std::shared_ptr<TokenStream> stream,
                   const std::string& name) {
    if (hasModule(name)) {
      return modules[name];
    }

    throw ModuleUndefinedError(stream->current(), name);
  }

  Method getMethod(std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CallStackFrame> frame,
                   const std::string& name) {
    if (hasMethod(frame, name)) {
      if (frame->inObjectContext()) {
        auto clazz = classes[frame->getObjectContext()->className];
        if (clazz.hasMethod(name)) {
          return clazz.getMethod(name);
        }

        if (frame->hasAssignedLambda(name)) {
          return frame->getAssignedLambda(name);
        }
      }

      if (frame->hasAssignedLambda(name)) {
        return frame->getAssignedLambda(name);
      }

      return methods[name];
    }

    auto tempStack(callStack);
    while (!tempStack.empty()) {
      auto& outerFrame = tempStack.top();
      if (outerFrame->hasAssignedLambda(name)) {
        return outerFrame->getAssignedLambda(name);
      }
      tempStack.pop();
    }

    throw MethodUndefinedError(stream->current(), name);
  }

  Value getVariable(std::shared_ptr<TokenStream> stream,
                    std::shared_ptr<CallStackFrame> frame,
                    const std::string& name) {
    // Check in the current frame
    if (frame->variables.find(name) != frame->variables.end()) {
      return frame->variables[name];
    }

    if (frame->inObjectContext() &&
        frame->getObjectContext()->instanceVariables.find(name) !=
            frame->getObjectContext()->instanceVariables.end()) {
      return frame->getObjectContext()->instanceVariables[name];
    }

    // Check in outer frames
    auto tempStack(callStack);
    while (!tempStack.empty()) {
      auto& outerFrame = tempStack.top();
      if (outerFrame->variables.find(name) != outerFrame->variables.end()) {
        return outerFrame->variables[name];
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(stream->current(), name);
  }

  std::vector<Value> interpretArguments(std::shared_ptr<TokenStream> stream,
                                        std::shared_ptr<CallStackFrame> frame) {
    std::vector<Value> args;

    if (!stream->match(TokenType::OPEN_PAREN)) {
      throw SyntaxError(stream->current(),
                        "Expected open-parenthesis, `(`, near `" +
                            stream->current().getText() + "`.");
    }

    bool closeParenthesisFound = false;
    while (stream->canRead() && !closeParenthesisFound &&
           stream->current().getType() != TokenType::CLOSE_PAREN) {
      if (stream->current().getType() == TokenType::COMMA) {
        stream->next();
        continue;
      }

      if (stream->current().getType() == TokenType::TYPENAME) {
        args.push_back(stream->current().getText());
      } else if (hasClass(stream->current().getText())) {
        args.push_back(stream->current().getText());
      } else {
        args.push_back(interpretExpression(stream, frame));
      }

      if (stream->current().getType() == TokenType::CLOSE_PAREN) {
        closeParenthesisFound = true;
      }

      if (!closeParenthesisFound) {
        stream->next();
      }
    }

    if (stream->current().getType() == TokenType::CLOSE_PAREN) {
      stream->next();
    }

    return args;
  }

  void interpretModuleBuiltin(std::shared_ptr<TokenStream> stream,
                              const std::string& moduleName,
                              const SubTokenType& builtin,
                              std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Module_Home) {
      if (args.size() != 1) {
        throw BuiltinUnexpectedArgumentError(stream->current(),
                                             ModuleBuiltins.Home);
      }

      if (!std::holds_alternative<std::string>(args.at(0))) {
        throw SyntaxError(stream->current(), "Expected string value for `" +
                                                 ModuleBuiltins.Home +
                                                 "` builtin parameter.");
      }

      modules[moduleName].setHome(std::get<std::string>(args.at(0)));
      modules[moduleName].setName(moduleName);
    }
  }

  std::vector<std::string> collectMethodParameters(
      std::shared_ptr<TokenStream> stream,
      std::shared_ptr<CallStackFrame> frame, Method& method) {
    if (!stream->match(TokenType::OPEN_PAREN)) {
      throw SyntaxError(
          stream->current(),
          "Expected open-parenthesis, `(`, in method parameter set.");
    }

    // Interpret parameters.
    auto parameters = method.getParameters();
    int paramIndex = 0;

    bool closeParenthesisFound = false;
    while (stream->canRead() &&
           stream->current().getType() != TokenType::CLOSE_PAREN) {
      if (stream->current().getType() == TokenType::COMMA) {
        stream->next();
        continue;
      }

      auto paramValue = interpretExpression(stream, frame);

      if (stream->current().getType() == TokenType::CLOSE_PAREN) {
        closeParenthesisFound = true;
      }

      method.addParameterValue(parameters.at(paramIndex++), paramValue);

      if (!closeParenthesisFound) {
        stream->next();
      }
    }
    stream->next();  // Skip ")"

    return parameters;
  }

  std::shared_ptr<CallStackFrame> buildMethodInvocationStackFrame(
      std::shared_ptr<TokenStream> stream,
      std::shared_ptr<CallStackFrame> frame, Method& method) {
    auto parameters = collectMethodParameters(stream, frame, method);
    auto codeFrame = buildSubFrame(frame, true);

    for (const auto& pair : frame->lambdas) {
      // Check this.
      codeFrame->assignLambda(pair.first, pair.second);
    }

    // Check all parameters are passed.
    for (const std::string& parameterName : parameters) {
      if (!method.hasParameter(parameterName)) {
        auto param = method.getParameter(parameterName);

        if (!param.hasDefaultValue()) {
          throw ParameterMissingError(stream->current(), parameterName);
        }

        codeFrame->variables[parameterName] = std::move(param.getValue());
      } else {
        codeFrame->variables[parameterName] =
            std::move(method.getParameterValue(parameterName));
      }
    }
    codeFrame->setFlag(FrameFlags::SubFrame);
    return codeFrame;
  }

  Value interpretClassMethodInvocation(std::shared_ptr<TokenStream> stream,
                                       std::shared_ptr<CallStackFrame> frame,
                                       const std::string& className) {
    stream->next();  // Skip class name
    stream->next();  // Skip the "."

    auto methodName = stream->current().getText();
    stream->next();  // Skip the method name.

    auto clazz = classes[className];
    bool isInstantiation = false;

    if (methodName == Keywords.New) {
      if (clazz.isAbstract()) {
        throw InvalidOperationError(stream->current(),
                                    "Cannot instantiate an abstract class.");
      }

      if (!clazz.hasMethod(Keywords.Ctor)) {
        throw UnimplementedMethodError(stream->current(), className,
                                       Keywords.Ctor);
      }

      methodName = Keywords.Ctor;
      isInstantiation = true;
    } else if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(stream->current(), className, methodName);
    }

    auto method = clazz.getMethod(methodName);
    if (!method.isFlagSet(MethodFlags::Static) &&
        !method.isFlagSet(MethodFlags::Ctor)) {
      throw InvalidContextError(
          stream->current(),
          "The method `" + methodName +
              "` can only be invoked on an instance of class `" + className +
              "`.");
    }

    if (method.isFlagSet(MethodFlags::Private)) {
      if (!frame->inObjectContext() ||
          frame->getObjectContext()->className != className) {
        throw InvalidContextError(
            stream->current(),
            "Cannot invoke private method outside of object context.");
      }
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
    if (isInstantiation) {
      auto context = std::make_shared<Object>();
      context->className = className;
      codeFrame->setObjectContext(context);
    }
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    // Now interpret the method in its own context
    interpretStackFrame();

    if (!callStack.empty()) {
      return callStack.top()->returnValue;
    }

    throw EmptyStackError(stream->current());
  }

  Value interpretInstanceMethodInvocation(std::shared_ptr<TokenStream> stream,
                                          std::shared_ptr<CallStackFrame> frame,
                                          std::shared_ptr<Object>& object,
                                          const std::string& methodName,
                                          const SubTokenType& op,
                                          std::vector<Value>& parameters) {
    if (!hasClass(object->className)) {
      throw ClassUndefinedError(stream->current(), object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      if (KiwiBuiltins.is_builtin(op)) {
        return BuiltinInterpreter::execute(stream->current(), op, object,
                                           parameters);
      }
      throw UnimplementedMethodError(stream->current(), object->className,
                                     methodName);
    }

    auto method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame->inObjectContext()) {
      throw InvalidContextError(
          stream->current(),
          "Cannot invoke private method outside of object context.");
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame = buildSubFrame(frame);

    if (static_cast<int>(parameters.size()) != method.getParameterCount()) {
      throw ParameterCountMismatchError(stream->current(), methodName);
    }

    // Check all parameters are passed.
    int parameterIndex = 0;
    for (const auto& parameterName : method.getParameters()) {
      codeFrame->variables[parameterName] = parameters.at(parameterIndex++);
    }
    codeFrame->setFlag(FrameFlags::SubFrame);
    codeFrame->setObjectContext(object);
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();

    if (!callStack.empty()) {
      frame->clearFlag(FrameFlags::ReturnFlag);
      return callStack.top()->returnValue;
    }

    return {};
  }

  void interpretInstanceMethodInvocation(std::shared_ptr<TokenStream> stream,
                                         std::shared_ptr<CallStackFrame> frame,
                                         const std::string& instanceName) {
    stream->next();  // Skip the "."
    if (stream->current().getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          stream->current(),
          "Expected identifier in instance method invocation, instead got: `" +
              stream->current().getText() + "`");
    }

    auto methodName = stream->current().getText();
    stream->next();  // Skip the method name.

    auto object = std::get<std::shared_ptr<Object>>(
        getVariable(stream, frame, instanceName));

    if (!hasClass(object->className)) {
      throw ClassUndefinedError(stream->current(), object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(stream->current(), object->className,
                                     methodName);
    }

    auto method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame->inObjectContext()) {
      throw InvalidContextError(
          stream->current(),
          "Cannot invoke private method outside of object context.");
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
    codeFrame->setObjectContext(object);
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();
  }

  Value interpretThen(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame, k_int taskId) {
    auto then = interpretLambda(stream, frame);
    auto result = task.getTaskResult(taskId);

    std::string taskResult, taskIdName;
    auto hasIndexVariable = false;

    if (then.hasParameters()) {
      for (const auto& parameter : then.getParameters()) {
        if (taskResult.empty()) {
          taskResult = parameter;
        } else if (taskIdName.empty()) {
          taskIdName = parameter;
          hasIndexVariable = true;
        } else {
          throw InvalidOperationError(stream->current(),
                                      "Too many parameters in `then` lambda.");
        }
      }

      frame->variables[taskResult] = result;
      if (hasIndexVariable) {
        frame->variables[taskIdName] = static_cast<int>(taskId);
      }
    }

    auto loopStream = std::make_shared<TokenStream>(then.getCode());
    callStack.push(buildSubFrame(frame, true));
    streamStack.push(loopStream);
    interpretStackFrame();

    if (!callStack.empty()) {
      auto value = callStack.top()->returnValue;
      frame->clearFlag(FrameFlags::ReturnFlag);

      return value;
    }

    return {};
  }

  Value interpretMethodInvocation(std::shared_ptr<TokenStream> stream,
                                  std::shared_ptr<CallStackFrame> frame,
                                  const std::string& name, bool await = false) {
    if (stream->current().getType() != TokenType::OPEN_PAREN) {
      stream->next();  // Skip the name.
    }

    auto method = getMethod(stream, frame, name);
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
    if (frame->inObjectContext()) {
      codeFrame->setObjectContext(frame->getObjectContext());
    }

    if (method.isFlagSet(MethodFlags::Async)) {
      auto taskId = interpretAsyncMethodInvocation(codeFrame, method);

      if (!await && stream->current().getSubType() == SubTokenType::KW_Then) {
        return interpretThen(stream, frame, taskId);
      } else if (await &&
                 stream->current().getSubType() != SubTokenType::KW_Then) {
        return taskId;
      }

      throw SyntaxError(stream->current(),
                        "Invalid syntax in asynchronous invocation.");
    }

    callStack.push(codeFrame);
    streamStack.push(std::make_shared<TokenStream>(method.getCode()));

    interpretStackFrame();

    if (!callStack.empty()) {
      return callStack.top()->returnValue;
    }

    return {};
  }

  std::vector<Parameter> getParameterSet(
      std::shared_ptr<TokenStream> stream,
      std::shared_ptr<CallStackFrame> frame) {
    if (!stream->match(TokenType::OPEN_PAREN)) {
      throw SyntaxError(
          stream->current(),
          "Expected open-parenthesis, `(`, in parameter set expression.");
    }

    std::unordered_set<std::string> paramNames;
    std::vector<Parameter> params;

    while (stream->canRead() &&
           stream->current().getType() != TokenType::CLOSE_PAREN) {
      auto paramName = stream->current().getText();
      if (stream->current().getType() == TokenType::IDENTIFIER) {
        if (paramNames.find(paramName) != paramNames.end()) {
          throw SyntaxError(stream->current(),
                            "The parameter `" + paramName +
                                "` was specified more than once.");
        }

        paramNames.insert(paramName);
        stream->next();

        if (stream->current().getType() == TokenType::OPERATOR &&
            stream->current().getText() == Operators.Assign) {
          stream->next();  // Skip "=".
          Parameter optionalParam(paramName,
                                  interpretExpression(stream, frame));
          params.push_back(optionalParam);
          continue;
        }

        Parameter param(paramName);
        params.push_back(param);

        continue;
      }
      stream->next();
    }

    if (!stream->match(TokenType::CLOSE_PAREN)) {
      throw SyntaxError(
          stream->current(),
          "Expected close-parenthesis, `)`, in parameter set expression.");
    }

    return params;
  }

  void interpretMethodParameters(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 Method& method) {
    for (const auto& param : getParameterSet(stream, frame)) {
      method.addParameter(param);
    }
  }

  Method interpretMethodDeclaration(std::shared_ptr<TokenStream> stream,
                                    std::shared_ptr<CallStackFrame> frame) {
    Method method;

    while (stream->canRead() &&
           stream->current().getSubType() != SubTokenType::KW_Method) {
      switch (stream->current().getSubType()) {
        case SubTokenType::KW_Abstract:
          method.setFlag(MethodFlags::Abstract);
          break;
        case SubTokenType::KW_Override:
          method.setFlag(MethodFlags::Override);
          break;
        case SubTokenType::KW_Private:
          method.setFlag(MethodFlags::Private);
          break;
        case SubTokenType::KW_Static:
          method.setFlag(MethodFlags::Static);
          break;
        default:
          break;
      }
      stream->next();
    }
    stream->next();  // Skip "def"

    auto name = stream->current().getText();
    method.setName(name);
    stream->next();  // Skip the name.
    interpretMethodParameters(stream, frame, method);
    int counter = 1;

    if (method.isFlagSet(MethodFlags::Abstract)) {
      return method;
    }

    while (stream->canRead() && counter > 0) {
      if (stream->current().getSubType() == SubTokenType::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(stream->current().getSubType())) {
        ++counter;
      }

      method.addToken(stream->current());
      stream->next();

      if (stream->current().getType() == TokenType::STREAM_END) {
        throw SyntaxError(stream->current(),
                          "Invalid method declaration `" + name + "`");
      }
    }

    return method;
  }

  void interpretAsyncMethodDefinition(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip "async"

    if (stream->current().getSubType() != SubTokenType::KW_Method) {
      throw SyntaxError(stream->current(),
                        "Expected method definition after `async`.");
    }

    auto method = interpretMethodDefinition(stream, frame);
    method.setFlag(MethodFlags::Async);
    methods[method.getName()] = method;
  }

  Method interpretMethodDefinition(std::shared_ptr<TokenStream> stream,
                                   std::shared_ptr<CallStackFrame> frame) {
    auto method = interpretMethodDeclaration(stream, frame);
    auto name = method.getName();
    std::string moduleName;

    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    if (!moduleName.empty()) {
      name = moduleName + Symbols.Qualifier + name;
    }

    if (Keywords.is_keyword(name)) {
      throw IllegalNameError(stream->current(), name);
    }

    method.setName(name);
    methods[name] = method;

    return method;
  }

  void interpretExit(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    bool hasValue = InterpHelper::hasReturnValue(stream);
    stream->next();  // Skip "exit"

    Value returnValue;

    if (hasValue) {
      returnValue = interpretExpression(stream, frame);
    }

    if (std::holds_alternative<k_int>(returnValue)) {
      int exitCode = static_cast<int>(std::get<k_int>(returnValue));
      exit(exitCode);
    } else {
      exit(1);
    }

    frame->returnValue = returnValue;
    frame->setFlag(FrameFlags::ReturnFlag);
  }

  void interpretReturn(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    bool hasValue = InterpHelper::hasReturnValue(stream);
    stream->next();  // Skip "return"

    if (hasValue) {
      frame->returnValue = interpretExpression(stream, frame);
      frame->setFlag(FrameFlags::ReturnFlag);
    }
  }

  SliceIndex interpretSliceIndex(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 Value& listValue) {
    if (!stream->match(TokenType::OPEN_BRACKET)) {
      throw SyntaxError(stream->current(),
                        "Expected open-bracket, `[`, for list access.");
    }

    SliceIndex slice;
    slice.indexOrStart = 0;

    if (std::holds_alternative<std::shared_ptr<List>>(listValue)) {
      slice.stopIndex = static_cast<int>(
          std::get<std::shared_ptr<List>>(listValue)->elements.size());
    } else if (std::holds_alternative<std::string>(listValue)) {
      slice.stopIndex =
          static_cast<int>(std::get<std::string>(listValue).size());
    }

    slice.stepValue = 1;
    slice.isSlice = false;

    // Detect if slicing or single index
    if (stream->peek().getType() == TokenType::COLON ||
        stream->current().getType() == TokenType::COLON) {
      slice.isSlice = true;
      if (stream->current().getType() != TokenType::COLON) {
        slice.indexOrStart = interpretExpression(stream, frame);
      }

      if (stream->peek().getType() == TokenType::COLON) {
        stream->next();  // Skip colon for start:stop:
      }

      if (stream->current().getType() == TokenType::COLON) {
        stream->next();  // Skip colon for start::step or ::step
        if (stream->current().getType() != TokenType::CLOSE_BRACKET) {
          slice.stopIndex = interpretExpression(stream, frame);
        }
      }

      if (stream->current().getType() == TokenType::COLON) {
        stream->next();  // Skip colon for ::step
        if (stream->current().getType() != TokenType::CLOSE_BRACKET) {
          slice.stepValue = interpretExpression(stream, frame);
        }
      }
    } else if (stream->current().getType() == TokenType::QUALIFIER) {
      stream->next();
      slice.isSlice = true;
      if (stream->current().getType() != TokenType::CLOSE_BRACKET) {
        slice.stepValue = interpretExpression(stream, frame);
      }
    } else {
      // Single index
      slice.indexOrStart = interpretExpression(stream, frame);
    }

    if (!stream->match(TokenType::CLOSE_BRACKET)) {
      throw SyntaxError(stream->current(),
                        "Expected close-bracket, `]`, for list access.");
    }

    return slice;
  }

  Value interpretKeyOrIndex(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    if (!stream->match(TokenType::OPEN_BRACKET)) {
      throw SyntaxError(stream->current(),
                        "Expected open-bracket, `[`, in key or index access.");
    }

    auto output = interpretExpression(stream, frame);

    if (!stream->match(TokenType::CLOSE_BRACKET)) {
      throw SyntaxError(stream->current(),
                        "Expected close-bracket, `]`, in key or index access.");
    }

    return output;
  }

  std::string interpretKey(std::shared_ptr<TokenStream> stream,
                           std::shared_ptr<CallStackFrame> frame) {
    auto output = interpretKeyOrIndex(stream, frame);

    if (!std::holds_alternative<std::string>(output)) {
      throw SyntaxError(stream->current(), "Hash key must be a string value.");
    }

    return std::get<std::string>(output);
  }

  int interpretIndex(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    auto output = interpretKeyOrIndex(stream, frame);

    if (!std::holds_alternative<k_int>(output)) {
      throw SyntaxError(stream->current(),
                        "List index must be an integer value.");
    }

    return std::get<k_int>(output);
  }

  Value interpretHashElementAccess(std::shared_ptr<TokenStream> stream,
                                   std::shared_ptr<CallStackFrame> frame,
                                   Value& value) {
    auto key = interpretKey(stream, frame);
    auto hash = std::get<std::shared_ptr<Hash>>(value);

    if (!hash->hasKey(key)) {
      throw HashKeyError(stream->current(), key);
    }

    return hash->get(key);
  }

  Value interpretSlice(std::shared_ptr<TokenStream> stream, Value& value,
                       SliceIndex& slice) {
    if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      auto list = std::get<std::shared_ptr<List>>(value);
      return InterpHelper::interpretListSlice(stream, slice, list);
    } else if (std::holds_alternative<std::string>(value)) {
      auto string = std::get<std::string>(value);
      auto list = std::make_shared<List>();

      for (const char& c : string) {
        list->elements.push_back(std::string(1, c));
      }

      auto sliced = InterpHelper::interpretListSlice(stream, slice, list);
      std::ostringstream sv;

      if (std::holds_alternative<std::shared_ptr<List>>(sliced)) {
        auto slicedlist = std::get<std::shared_ptr<List>>(sliced);
        for (auto it = slicedlist->elements.begin();
             it != slicedlist->elements.end(); ++it) {
          sv << Serializer::serialize(*it);
        }
      } else {
        sv << Serializer::serialize(sliced);
      }

      return sv.str();
    }

    throw ConversionError(
        stream->current(),
        "Expected a `List` or a `String` for slice operation.");
  }

  Value interpretSlice(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame,
                       const std::string& name) {
    auto value = getVariable(stream, frame, name);
    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      return interpretHashElementAccess(stream, frame, value);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value) &&
        !std::holds_alternative<std::string>(value)) {
      throw InvalidOperationError(
          stream->current(), "`" + name + "` is not a `List` or a `String`.");
    }

    auto slice = interpretSliceIndex(stream, frame, value);
    return interpretSlice(stream, value, slice);
  }

  void interpretAppendToList(std::shared_ptr<TokenStream> stream,
                             std::shared_ptr<CallStackFrame> frame,
                             Value& listValue,
                             const std::string& listVariableName) {
    Value variableValue;
    try {
      variableValue = getVariable(stream, frame, listVariableName);
    } catch (const VariableUndefinedError& e) {
      throw VariableUndefinedError(stream->current(), listVariableName);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(variableValue)) {
      throw InvalidOperationError(stream->current(),
                                  "`" + listVariableName + "` is not a list.");
    }

    const auto& listPtr = std::get<std::shared_ptr<List>>(variableValue);
    listPtr->elements.push_back(listValue);
  }

  std::shared_ptr<List> interpretRange(std::shared_ptr<TokenStream> stream,
                                       std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip the "["

    auto startValue = interpretExpression(stream, frame);

    if (!stream->match(TokenType::RANGE)) {
      throw RangeError(stream->current(),
                       "Expected range separator, `..`, in range expression.");
    }

    auto stopValue = interpretExpression(stream, frame);

    if (!stream->match(TokenType::CLOSE_BRACKET)) {
      throw RangeError(stream->current(),
                       "Expected close-bracket, `]`, in range expression.");
    }

    if (!std::holds_alternative<k_int>(startValue)) {
      throw RangeError(stream->current(),
                       "A range start value must be an integer.");
    }

    if (!std::holds_alternative<k_int>(stopValue)) {
      throw RangeError(stream->current(),
                       "A range stop value must be an integer.");
    }

    auto start = std::get<k_int>(startValue), stop = std::get<k_int>(stopValue);
    auto step = stop < start ? -1 : 1;
    auto i = start;

    auto list = std::make_shared<List>();
    for (; i != stop; i += step) {
      list->elements.push_back(i);
    }
    list->elements.push_back(i);

    return list;
  }

  std::shared_ptr<Hash> interpretHash(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip the "{"

    auto hash = std::make_shared<Hash>();

    while (stream->canRead() &&
           stream->current().getType() != TokenType::CLOSE_BRACE) {
      auto keyValue = interpretExpression(stream, frame);

      if (!std::holds_alternative<std::string>(keyValue)) {
        throw SyntaxError(stream->current(),
                          "Hash key must be a string value.");
      }

      if (stream->current().getType() == TokenType::COLON) {
        stream->next();  // Skip the ":"
        hash->add(std::get<std::string>(keyValue),
                  interpretExpression(stream, frame));
      }

      if (stream->current().getType() == TokenType::COMMA) {
        stream->next();
      }
    }

    if (stream->current().getType() == TokenType::CLOSE_BRACE) {
      stream->next();  // Skip the "}"
    }

    return hash;
  }

  std::shared_ptr<List> interpretList(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    auto list = std::make_shared<List>();
    stream->next();  // Skip "["
    int bracketCount = 1;

    while (stream->canRead() && bracketCount > 0) {
      switch (stream->current().getType()) {
        case TokenType::OPEN_BRACKET:
          ++bracketCount;
          list->elements.push_back(interpretExpression(stream, frame));
          break;

        case TokenType::CLOSE_BRACKET: {
          --bracketCount;

          // Stop here.
          if (bracketCount == 0) {
            stream->next();  // Skip "]"
            //break;
          }
        } break;

        case TokenType::COMMA:
          stream->next();
          break;

        default:
          list->elements.push_back(interpretExpression(stream, frame));
          if (stream->current().getType() == TokenType::COMMA ||
              stream->current().getType() == TokenType::CLOSE_BRACKET) {
            continue;
          }

          stream->next();
          break;
      }
    }

    return list;
  }

  void interpretConditional(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    if (!stream->matchsub(SubTokenType::KW_If)) {
      throw SyntaxError(stream->current(),
                        "Invalid conditional. Expected `" + Keywords.If +
                            "` keyword, instead got: `" +
                            stream->current().getText() + "`");
    }

    // Eagerly evaluate the If conditions.
    auto value = interpretExpression(stream, frame);

    if (!std::holds_alternative<bool>(value)) {
      throw ConversionError(stream->current());
    }

    bool shortCircuitIf = std::get<bool>(value);
    Conditional conditional;
    conditional.getIfStatement().setEvaluation(shortCircuitIf);

    int ifCount = 1;
    bool shortCircuitElseIf = false;
    auto building = SubTokenType::KW_If;

    while (stream->canRead() && ifCount > 0) {
      auto subType = stream->current().getSubType();
      if (Keywords.is_block_keyword(subType)) {
        ++ifCount;
      } else if (subType == SubTokenType::KW_End && ifCount > 0) {
        --ifCount;

        // Stop here.
        if (ifCount == 0) {
          stream->next();
          continue;
        }
      } else if (ifCount == 1 && subType == SubTokenType::KW_Else) {
        stream->next();
        building = SubTokenType::KW_Else;
      } else if (ifCount == 1 && subType == SubTokenType::KW_ElseIf) {
        stream->next();
        building = SubTokenType::KW_ElseIf;

        conditional.addElseIfStatement();

        // No need to evaluate if the previous condition is true.
        if (shortCircuitIf || shortCircuitElseIf) {
          continue;
        }

        // Eagerly evaluate ElseIf conditions.
        value = interpretExpression(stream, frame);

        if (!std::holds_alternative<bool>(value)) {
          throw ConversionError(stream->current());
        }

        bool elseIfValue = std::get<bool>(value);
        conditional.getElseIfStatement().setEvaluation(elseIfValue);

        if (elseIfValue) {
          // Don't evaluate future ElseIf branches.
          shortCircuitElseIf = true;
        }
      }

      // Distribute tokens to be executed.
      if (shortCircuitIf && building == SubTokenType::KW_If) {
        conditional.getIfStatement().addToken(stream->current());
      } else if (!shortCircuitIf && building == SubTokenType::KW_ElseIf) {
        conditional.getElseIfStatement().addToken(stream->current());
      } else if (!shortCircuitIf && !shortCircuitElseIf &&
                 building == SubTokenType::KW_Else) {
        conditional.getElseStatement().addToken(stream->current());
      }

      stream->next();
    }

    if (conditional.getIfStatement().isExecutable()) {
      execute(frame, conditional.getIfStatement().getCode());
    } else if (conditional.canExecuteElseIf()) {
      for (auto& elseIf : conditional.getElseIfStatements()) {
        if (elseIf.isExecutable()) {
          execute(frame, elseIf.getCode());
          break;
        }
      }
    } else {
      execute(frame, conditional.getElseStatement().getCode());
    }
  }

  void execute(std::shared_ptr<CallStackFrame> frame,
               const std::vector<Token>& executableTokens) {
    if (executableTokens.empty()) {
      return;
    }

    callStack.push(buildSubFrame(frame));
    streamStack.push(std::make_shared<TokenStream>(executableTokens));
    interpretStackFrame();
  }

  void execute(std::shared_ptr<CallStackFrame> frame,
               std::shared_ptr<TokenStream> stream) {
    callStack.push(buildSubFrame(frame));
    streamStack.push(stream);
    interpretStackFrame();
  }

  Value interpretSelfInvocation(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    if (!frame->inObjectContext()) {
      throw InvalidContextError(stream->current(),
                                "Invalid context for keyword `this`.");
    }

    stream->next();  // Skip "this"
    if (!stream->match(TokenType::DOT)) {
      throw SyntaxError(stream->current(),
                        "Invalid syntax near keyword `this`.");
    }

    Value value;

    if (stream->current().getType() == TokenType::IDENTIFIER) {
      switch (stream->peek().getType()) {
        case TokenType::OPERATOR:
          value = interpretAssignment(stream, frame, "", false, true);
          break;

        case TokenType::OPEN_PAREN:
          value = interpretMethodInvocation(stream, frame,
                                            stream->current().getText());
          break;

        default:
          break;
      }
    }

    return value;
  }

  void interpretClassDefinition(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    bool isAbstract =
        stream->current().getSubType() == SubTokenType::KW_Abstract;

    while (stream->canRead() &&
           stream->current().getSubType() != SubTokenType::KW_Class) {
      stream->next();
    }
    stream->next();  // Skip "class"

    auto className = stream->current().getText();
    stream->next();  // Skip class name.

    if (classes.find(className) != classes.end()) {
      throw ClassRedefinitionError(stream->current(), className);
    }

    auto baseClassName = InterpHelper::interpretBaseClass(stream);

    Class clazz;
    if (isAbstract) {
      clazz.setAbstract();
    }
    clazz.setClassName(className);
    clazz.setBaseClassName(baseClassName);

    if (!baseClassName.empty()) {
      if (classes.find(baseClassName) == classes.end()) {
        throw ClassUndefinedError(stream->current(), baseClassName);
      }

      // inherit methods from base class.
      for (auto& pair : classes[baseClassName].getMethods()) {
        clazz.addMethod(pair.second);
      }
    }

    int counter = 1;
    while (stream->canRead() && counter > 0) {
      auto token = stream->current();
      auto st = token.getSubType();

      if (st == SubTokenType::KW_End) {
        --counter;
        if (counter == 0) {
          stream->next();  // Skip "end"
          break;
        }
      }

      if (st == SubTokenType::KW_Private &&
          stream->peek().getType() == TokenType::OPEN_PAREN) {
        stream->next();  // Skip "private"

        for (const auto& privateVar : getParameterSet(stream, frame)) {
          clazz.addPrivateVariable(privateVar);
        }

        continue;
      }

      if (st == SubTokenType::KW_Abstract || st == SubTokenType::KW_Override ||
          st == SubTokenType::KW_Method || st == SubTokenType::KW_Private ||
          st == SubTokenType::KW_Static) {

        auto method = interpretMethodDeclaration(stream, frame);

        if (method.getName() == Keywords.Ctor) {
          method.setFlag(MethodFlags::Ctor);
        }

        if (clazz.hasMethod(method.getName())) {
          auto classMethod = clazz.getMethod(method.getName());
          if (!method.isFlagSet(MethodFlags::Override) &&
              classMethod.isFlagSet(MethodFlags::Abstract)) {
            throw SyntaxError(token,
                              "The class, `" + className +
                                  "` has an abstract definition for `" +
                                  method.getName() +
                                  "` and the `override` keyword is missing.");
          }
        }

        clazz.addMethod(method);
      } else {
        stream->next();
      }
    }

    // Check for unimplemented abstract methods.
    if (!clazz.isAbstract()) {
      for (const auto& pair : clazz.getMethods()) {
        if (pair.second.isFlagSet(MethodFlags::Abstract)) {
          throw UnimplementedMethodError(stream->current(), className,
                                         pair.second.getName());
        }
      }
    }

    classes[className] = std::move(clazz);
  }

  void interpretModuleDefinition(std::shared_ptr<TokenStream> stream) {
    stream->next();  // Skip "module"

    auto name = stream->current().getText();
    if (hasModule(name)) {
      // WIP: Mixins?
    }

    stream->next();  // Skip the module name.

    Module module;
    int counter = 1;
    while (stream->canRead() && counter > 0) {
      if (stream->current().getSubType() == SubTokenType::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(stream->current().getSubType())) {
        ++counter;
      }

      module.addToken(stream->current());
      stream->next();
    }

    modules[name] = std::move(module);
  }

  std::string interpretModuleImport(std::shared_ptr<TokenStream> stream,
                                    const std::string& home,
                                    const std::string& name) {
    stream->next();  // Skip the name.

    moduleStack.push(name);
    auto module = hasHomedModule(home, name)
                      ? getHomedModule(stream, home, name)
                      : getModule(stream, name);

    auto codeStream = std::make_shared<TokenStream>(module.getCode());
    auto codeFrame = std::make_shared<CallStackFrame>();
    callStack.push(codeFrame);
    streamStack.push(codeStream);
    interpretStackFrame();
    moduleStack.pop();
    return name;
  }

  std::string interpretExternalImport(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    auto scriptNameValue = interpretExpression(stream, frame);
    if (!std::holds_alternative<std::string>(scriptNameValue)) {
      throw ConversionError(stream->current(),
                            "Expected a string for `import` statement.");
    }

    auto scriptName = std::get<std::string>(scriptNameValue);
    auto scriptNameKiwi = scriptName;
    if (!String::endsWith(scriptName, ".kiwi") &&
        !String::endsWith(scriptName, "🥝")) {
      scriptName += ".kiwi";
      scriptNameKiwi += ".🥝";
    }

    auto scriptPath = File::getLocalPath(scriptName);
    auto kiwiScriptPath = File::getLocalPath(scriptNameKiwi);
    if (!File::fileExists(scriptPath) && !File::fileExists(kiwiScriptPath)) {
      throw FileNotFoundError(scriptName);
    }

    auto content = File::fileExists(kiwiScriptPath)
                       ? File::readFile(kiwiScriptPath)
                       : File::readFile(scriptPath);
    if (content.empty()) {
      return "";
    }

    Lexer lexer(scriptPath, content);
    execute(frame, lexer.getTokenStream());

    // Check if a module was imported.
    if (std::holds_alternative<std::string>(frame->returnValue)) {
      auto moduleName = std::get<std::string>(frame->returnValue);

      if (!hasModule(moduleName)) {
        moduleName.clear();
      }

      return moduleName;
    }

    return "";
  }

  void interpretModuleAlias(std::shared_ptr<TokenStream> stream,
                            const std::string& moduleName) {
    stream->next();  // Skip "as"

    if (stream->current().getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(stream->current(), "Expected identifier for alias.");
    }

    auto alias = stream->current().getText();
    stream->next();  // Skip the alias

    auto search = moduleName + Symbols.Qualifier;

    if (hasClass(alias)) {
      throw InvalidOperationError(
          stream->current(), "The module alias `" + alias + "` is in use.");
    }

    Class clazz;
    clazz.setClassName(alias);

    for (auto pair : methods) {
      auto name = pair.first;
      if (String::beginsWith(name, search)) {
        auto method = pair.second;
        method.setFlag(MethodFlags::Static);
        method.setName(String::replace(name, search, ""));
        clazz.addMethod(method);
      }
    }

    classes[alias] = clazz;

    for (auto pair : clazz.getMethods()) {
      methods.erase(pair.first);
    }

    modules.erase(moduleName);
  }

  void interpretExport(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // skip the "export"

    auto moduleName = stream->current().getText();
    auto moduleHome = InterpHelper::interpretModuleHome(moduleName, stream);

    if (hasModule(moduleName)) {
      moduleName = interpretModuleImport(stream, moduleHome, moduleName);
      frame->returnValue = moduleName;
      frame->setFlag(FrameFlags::ReturnFlag);
    } else {
      throw InvalidOperationError(stream->current(),
                                  "Invalid export `" + moduleName + "`");
    }
  }

  void interpretImport(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // skip the "import"

    auto tokenText = stream->current().getText();
    auto moduleName = tokenText;
    auto moduleHome = InterpHelper::interpretModuleHome(moduleName, stream);

    if (hasModule(moduleName)) {
      moduleName = interpretModuleImport(stream, moduleHome, moduleName);
    } else {
      moduleName = interpretExternalImport(stream, frame);
    }

    if (stream->current().getSubType() == SubTokenType::KW_As) {
      interpretModuleAlias(stream, moduleName);
    }
  }

  void interpretDeleteHashKey(std::shared_ptr<TokenStream> stream,
                              std::shared_ptr<CallStackFrame> frame,
                              const std::string& name, Value& value) {
    auto key = interpretKey(stream, frame);
    auto hash = std::get<std::shared_ptr<Hash>>(value);

    if (!hash->hasKey(key)) {
      throw HashKeyError(stream->current(), key);
    }

    hash->remove(key);
    frame->variables[name] = hash;
  }

  void interpretDeleteListIndex(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame,
                                const std::string& name, Value& value) {
    int index = interpretIndex(stream, frame);
    auto list = std::get<std::shared_ptr<List>>(value);

    if (index < 0 || index >= static_cast<int>(list->elements.size())) {
      throw RangeError(stream->current(), "List index out of range.");
    }

    list->elements.erase(list->elements.begin() + index);
    frame->variables[name] = list;
  }

  void interpretDelete(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    stream->next();  // Skip "delete"

    if (stream->current().getType() == TokenType::IDENTIFIER) {
      auto name = stream->current().getText();
      stream->next();

      if (!hasVariable(frame, name)) {
        throw VariableUndefinedError(stream->current(), name);
      }

      auto value = getVariable(stream, frame, name);

      if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
        interpretDeleteHashKey(stream, frame, name, value);
      } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
        interpretDeleteListIndex(stream, frame, name, value);
      }

      return;
    }

    throw SyntaxError(stream->current(),
                      "Cannot delete from a non-variable value.");
  }

  void interpretPrint(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame,
                      bool printNewLine = false) {
    stream->next();  // skip the "print"

    auto value = interpretExpression(stream, frame);

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      if (!SILENCE) {
        std::cout << interpolateObject(stream, frame, value);
      }
    } else {
      if (!SILENCE) {
        std::cout << Serializer::serialize(value);
      }
    }

    if (printNewLine) {
      if (!SILENCE) {
        std::cout << std::endl;
      }
    }
  }

  Value interpretBracketExpression(std::shared_ptr<TokenStream> stream,
                                   std::shared_ptr<CallStackFrame> frame) {
    if (InterpHelper::isRangeExpression(stream)) {
      return interpretRange(stream, frame);
    } else if (InterpHelper::isListExpression(stream)) {
      return interpretList(stream, frame);
    }

    throw SyntaxError(stream->current(), "Invalid bracket expression.");
  }

  Value interpretBuiltin(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame,
                         const SubTokenType& builtin) {
    stream->next();  // Skip the name.

    auto args = interpretArguments(stream, frame);

    if (ModuleBuiltins.is_builtin(builtin)) {
      if (moduleStack.empty()) {
        throw InvalidContextError(stream->current(),
                                  "Expected a module context.");
      }
      auto moduleName = moduleStack.top();
      interpretModuleBuiltin(stream, moduleName, builtin, args);
      return 0;
    }

    frame->returnValue =
        BuiltinInterpreter::execute(stream->current(), builtin, args, kiwiArgs);
    return frame->returnValue;
  }

  Value interpretListSum(std::shared_ptr<TokenStream> stream,
                         const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    if (stream->current().getType() == TokenType::CLOSE_PAREN) {
      stream->next();
      return sum_listvalue(list);
    } else {
      throw SyntaxError(stream->current(), "Expected a close-parenthesis.");
    }

    return {};
  }

  Value interpretListMin(std::shared_ptr<TokenStream> stream,
                         const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    if (stream->current().getType() == TokenType::CLOSE_PAREN) {
      stream->next();
      if (list->elements.empty()) {
        throw EmptyListError(stream->current());
      }

      return min_listvalue(list);
    } else {
      throw SyntaxError(stream->current(), "Expected a close-parenthesis.");
    }

    return {};
  }

  Value interpretListMax(std::shared_ptr<TokenStream> stream,
                         const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    if (stream->current().getType() == TokenType::CLOSE_PAREN) {
      stream->next();  // Skip ")"

      if (list->elements.empty()) {
        throw EmptyListError(stream->current());
      }

      return max_listvalue(list);
    } else {
      throw SyntaxError(stream->current(), "Expected a close-parenthesis.");
    }

    return {};
  }

  Value interpretListSort(std::shared_ptr<TokenStream> stream,
                          const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    if (stream->current().getType() == TokenType::CLOSE_PAREN) {
      sortList(*list);
      stream->next();
    } else {
      throw SyntaxError(stream->current(), "Expected a close-parenthesis.");
    }

    return list;
  }

  Value interpretLambdaNone(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame,
                            const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (stream->current().getType() == CLOSE_PAREN) {
      stream->next();  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          stream->current(),
          "Expected a lambda in `" + ListBuiltins.None + "` builtin.");
    }

    std::string itemVariableName, indexVariableName;
    auto hasIndexVariable = false;
    for (const auto& parameter : lambda.getParameters()) {
      if (itemVariableName.empty()) {
        itemVariableName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(stream->current(),
                                             ListBuiltins.None);
      }
    }

    auto found = false;

    size_t index = 0;
    for (const auto& item : list->elements) {
      frame->variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame->variables[indexVariableName] = static_cast<int>(index);
      }

      auto loopStream = std::make_shared<TokenStream>(lambda.getCode());
      callStack.push(buildSubFrame(frame, true));
      streamStack.push(loopStream);
      interpretStackFrame();

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);

        if (std::holds_alternative<bool>(value) && std::get<bool>(value)) {
          found = true;
          break;
        }
      }
      index++;
    }

    return !found;
  }

  Value interpretLambdaMap(std::shared_ptr<TokenStream> stream,
                           std::shared_ptr<CallStackFrame> frame,
                           const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (stream->current().getType() == CLOSE_PAREN) {
      stream->next();  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          stream->current(),
          "Expected a lambda in `" + ListBuiltins.Map + "` builtin.");
    }

    std::string itemVariableName, indexVariableName;
    bool hasIndexVariable = false;

    for (const auto& parameter : lambda.getParameters()) {
      if (itemVariableName.empty()) {
        itemVariableName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(stream->current(),
                                             ListBuiltins.Map);
      }
    }

    auto mappedList = std::make_shared<List>();
    size_t index = 0;

    for (const auto& item : list->elements) {
      frame->variables[itemVariableName] = item;

      if (hasIndexVariable) {
        frame->variables[indexVariableName] = static_cast<int>(index);
      }

      auto loopStream = std::make_shared<TokenStream>(lambda.getCode());
      callStack.push(buildSubFrame(frame, true));
      streamStack.push(loopStream);
      interpretStackFrame();

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);
        mappedList->elements.push_back(value);
      }

      index++;
    }

    return mappedList;
  }

  Value interpretLambdaReduce(std::shared_ptr<TokenStream> stream,
                              std::shared_ptr<CallStackFrame> frame,
                              const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    auto accumulator = interpretExpression(stream, frame);

    if (stream->current().getType() != TokenType::LAMBDA) {
      stream->next();
    }

    if (stream->current().getType() == TokenType::COMMA) {
      stream->next();
    }

    auto lambda = getLambda(stream, frame);

    if (stream->current().getType() == CLOSE_PAREN) {
      stream->next();  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          stream->current(),
          "Expected a lambda in `" + ListBuiltins.Reduce + "` builtin.");
    }

    std::string accumulatorName, indexVariableName;
    bool hasIndexVariable = false;
    for (const auto& parameter : lambda.getParameters()) {
      if (accumulatorName.empty()) {
        accumulatorName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(stream->current(),
                                             ListBuiltins.Reduce);
      }
    }

    size_t index = 0;

    for (const auto& item : list->elements) {
      frame->variables[accumulatorName] = accumulator;
      if (hasIndexVariable) {
        frame->variables[indexVariableName] = item;
      }

      auto loopStream = std::make_shared<TokenStream>(lambda.getCode());
      callStack.push(buildSubFrame(frame, true));
      streamStack.push(loopStream);
      interpretStackFrame();

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);
        accumulator = value;
      }
      index++;
    }

    return accumulator;
  }

  Value interpretLambdaSelect(std::shared_ptr<TokenStream> stream,
                              std::shared_ptr<CallStackFrame> frame,
                              const std::shared_ptr<List>& list) {
    stream->next();  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (stream->current().getType() == CLOSE_PAREN) {
      stream->next();  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          stream->current(),
          "Expected a lambda in `" + ListBuiltins.Select + "` builtin.");
    }

    std::string itemVariableName, indexVariableName;
    auto hasIndexVariable = false;
    for (const auto& parameter : lambda.getParameters()) {
      if (itemVariableName.empty()) {
        itemVariableName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(stream->current(),
                                             ListBuiltins.Select);
      }
    }

    auto filteredList = std::make_shared<List>();

    size_t index = 0;
    for (const auto& item : list->elements) {
      frame->variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame->variables[indexVariableName] = static_cast<int>(index);
      }

      auto loopStream = std::make_shared<TokenStream>(lambda.getCode());
      callStack.push(buildSubFrame(frame, true));
      streamStack.push(loopStream);
      interpretStackFrame();

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);

        if (std::holds_alternative<bool>(value) && std::get<bool>(value)) {
          filteredList->elements.push_back(item);
        }
      }
      index++;
    }

    return filteredList;
  }

  Value interpretStringToHash(std::shared_ptr<TokenStream> stream,
                              std::shared_ptr<CallStackFrame> frame,
                              const std::string& input) {
    if (!stream->match(TokenType::OPEN_PAREN)) {
      throw SyntaxError(stream->current(),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }

    if (!stream->match(TokenType::CLOSE_PAREN)) {
      throw SyntaxError(stream->current(),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }

    return interpolateString(stream, frame, input);
  }

  Value interpretObjectToHash(std::shared_ptr<TokenStream> stream,
                              const std::shared_ptr<Object>& object) {
    if (!stream->match(TokenType::OPEN_PAREN)) {
      throw SyntaxError(stream->current(),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }

    if (!stream->match(TokenType::CLOSE_PAREN)) {
      throw SyntaxError(stream->current(),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }

    auto hash = std::make_shared<Hash>();
    auto clazz = classes[object->className];
    for (const auto& pair : object->instanceVariables) {
      if (clazz.hasPrivateVariable(pair.first)) {
        continue;
      }

      hash->add(pair.first, pair.second);
    }

    return hash;
  }

  Value interpretSpecializedBuiltin(std::shared_ptr<TokenStream> stream,
                                    std::shared_ptr<CallStackFrame> frame,
                                    const SubTokenType& builtin,
                                    const Value& value) {
    if (!std::holds_alternative<std::string>(value)) {
      throw InvalidOperationError(stream->current(), "Expected type String.");
    }

    auto input = std::get<std::string>(value);

    if (builtin == SubTokenType::Builtin_List_ToH) {
      return interpretStringToHash(stream, frame, input);
    }

    throw UnknownBuiltinError(stream->current(), "");
  }

  Value interpretSpecializedObjectBuiltin(std::shared_ptr<TokenStream> stream,
                                          const SubTokenType& builtin,
                                          const Value& value) {
    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      throw InvalidOperationError(stream->current(), "Expected type Object.");
    }

    auto object = std::get<std::shared_ptr<Object>>(value);

    if (builtin == SubTokenType::Builtin_List_ToH) {
      return interpretObjectToHash(stream, object);
    }

    throw UnknownBuiltinError(stream->current(), "");
  }

  Value interpretSpecializedListBuiltin(std::shared_ptr<TokenStream> stream,
                                        std::shared_ptr<CallStackFrame> frame,
                                        const SubTokenType& builtin,
                                        const Value& value) {
    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(stream->current(), "Expected type List.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);

    switch (builtin) {
      case SubTokenType::Builtin_List_Select:
        return interpretLambdaSelect(stream, frame, list);

      case SubTokenType::Builtin_List_Map:
        return interpretLambdaMap(stream, frame, list);

      case SubTokenType::Builtin_List_Reduce:
        return interpretLambdaReduce(stream, frame, list);

      case SubTokenType::Builtin_List_None:
        return interpretLambdaNone(stream, frame, list);

      case SubTokenType::Builtin_List_Sort:
        return interpretListSort(stream, list);

      case SubTokenType::Builtin_List_Min:
        return interpretListMin(stream, list);

      case SubTokenType::Builtin_List_Max:
        return interpretListMax(stream, list);

      case SubTokenType::Builtin_List_Sum:
        return interpretListSum(stream, list);

      default:
        throw UnknownBuiltinError(stream->current(), "");
    }
  }

  Value interpretDotNotation(std::shared_ptr<TokenStream> stream,
                             std::shared_ptr<CallStackFrame> frame,
                             Value& value) {
    if (stream->current().getType() == TokenType::DOT) {
      stream->next();
    }

    auto opText = stream->current().getText();
    auto op = stream->current().getSubType();
    stream->next();

    bool isObject = false;

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      auto object = std::get<std::shared_ptr<Object>>(value);
      auto clazz = classes[object->className];
      isObject = true;

      if (object->instanceVariables.find(opText) !=
          object->instanceVariables.end()) {
        if (clazz.hasPrivateVariable(opText)) {
          throw InvalidContextError(stream->current(),
                                    "Cannot access private instance variable "
                                    "outside of object context.");
        }

        return object->instanceVariables[opText];
      }
    }

    if (stream->current().getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(stream->current(),
                        "Expected open-parenthesis, `(`, to invoke builtin or "
                        "method using dot-notation.");
    }

    if (ListBuiltins.is_builtin(op)) {
      if (op == SubTokenType::Builtin_List_ToH) {
        if (isObject) {
          return interpretSpecializedObjectBuiltin(stream, op, value);
        } else {
          return interpretSpecializedBuiltin(stream, frame, op, value);
        }
      } else {
        return interpretSpecializedListBuiltin(stream, frame, op, value);
      }
    }

    auto args = interpretArguments(stream, frame);

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      return interpretInstanceMethodInvocation(
          stream, frame, std::get<std::shared_ptr<Object>>(value), opText, op,
          args);
    }

    return BuiltinInterpreter::execute(stream->current(), op, value, args);
  }

  Value parseExpression(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    Value value = parseLogicalOr(stream, frame);

    if (stream->current().getType() == TokenType::QUESTION) {
      stream->next();  // Skip the '?'

      auto trueBranch = parseExpression(stream, frame);

      if (!stream->match(TokenType::COLON)) {
        throw SyntaxError(stream->current(),
                          "Expected ':' in ternary operation.");
      }

      auto falseBranch =
          parseExpression(stream, frame);  // Parse the false branch

      if (!std::holds_alternative<bool>(value)) {
        throw ConversionError(
            stream->current(),
            "Ternary condition must be a boolean expression.");
      }

      return std::get<bool>(value) ? trueBranch : falseBranch;
    }

    return value;
  }

  Value parseLogicalOr(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    auto left = parseLogicalAnd(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_Or) {
      stream->next();  // Skip "||"
      auto right = parseLogicalAnd(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs || rhs;
      } else {
        throw ConversionError(stream->current(),
                              "Expected a `Boolean` expression.");
      }
    }
    return left;
  }

  Value parseLogicalAnd(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseOr(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_And) {
      stream->next();  // Skip "&&"
      auto right = parseBitwiseOr(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs && rhs;
      } else {
        throw ConversionError(stream->current(),
                              "Expected a `Boolean` expression.");
      }
    }
    return left;
  }

  Value parseBitwiseOr(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseXor(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_BitwiseOr) {
      stream->next();
      auto right = parseBitwiseXor(stream, frame);
      left = std::visit(BitwiseOrVisitor(stream->current()), left, right);
    }

    return left;
  }

  Value parseBitwiseXor(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseAnd(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_BitwiseXor) {
      stream->next();
      auto right = parseBitwiseAnd(stream, frame);
      left = std::visit(BitwiseXorVisitor(stream->current()), left, right);
    }

    return left;
  }

  Value parseBitwiseAnd(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseEquality(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_BitwiseAnd) {
      stream->next();  // Skip "&"
      auto right = parseEquality(stream, frame);
      left = std::visit(BitwiseAndVisitor(stream->current()), left, right);
    }

    return left;
  }

  Value parseEquality(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseComparison(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_Equal ||
           stream->current().getSubType() == SubTokenType::Ops_NotEqual) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseComparison(stream, frame);

      switch (op) {
        case SubTokenType::Ops_Equal:
          left = std::visit(EqualityVisitor(), left, right);
          break;

        case SubTokenType::Ops_NotEqual:
          left = std::visit(InequalityVisitor(), left, right);
          break;

        default:
          break;
      }
    }
    return left;
  }

  Value parseComparison(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitshift(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_GreaterThan ||
           stream->current().getSubType() ==
               SubTokenType::Ops_GreaterThanOrEqual ||
           stream->current().getSubType() == SubTokenType::Ops_LessThan ||
           stream->current().getSubType() ==
               SubTokenType::Ops_LessThanOrEqual) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseBitshift(stream, frame);

      switch (op) {
        case SubTokenType::Ops_LessThan:
          left = std::visit(LessThanVisitor(), left, right);
          break;

        case SubTokenType::Ops_LessThanOrEqual:
          left = std::visit(LessThanOrEqualVisitor(), left, right);
          break;

        case SubTokenType::Ops_GreaterThan:
          left = std::visit(GreaterThanVisitor(), left, right);
          break;

        case SubTokenType::Ops_GreaterThanOrEqual:
          left = std::visit(GreaterThanOrEqualVisitor(), left, right);
          break;

        default:
          break;
      }
    }

    return left;
  }

  Value parseBitshift(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseAddition(stream, frame);

    while (
        stream->current().getSubType() == SubTokenType::Ops_BitwiseLeftShift ||
        stream->current().getSubType() == SubTokenType::Ops_BitwiseRightShift) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseAddition(stream, frame);

      switch (op) {
        case SubTokenType::Ops_BitwiseLeftShift:
          left = std::visit(BitwiseLeftShiftVisitor(stream->current()), left,
                            right);
          break;

        case SubTokenType::Ops_BitwiseRightShift:
          left = std::visit(BitwiseRightShiftVisitor(stream->current()), left,
                            right);
          break;

        default:
          break;
      }
    }

    return left;
  }

  Value parseAddition(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseMultiplication(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_Add ||
           stream->current().getSubType() == SubTokenType::Ops_Subtract) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseMultiplication(stream, frame);

      switch (op) {
        case SubTokenType::Ops_Add:
          left = std::visit(AddVisitor(stream->current()), left, right);
          break;

        case SubTokenType::Ops_Subtract:
          left = std::visit(SubtractVisitor(stream->current()), left, right);
          break;

        default:
          break;
      }
    }

    return left;
  }

  Value parseMultiplication(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto left = parseUnary(stream, frame);

    while (stream->current().getSubType() == SubTokenType::Ops_Multiply ||
           stream->current().getSubType() == SubTokenType::Ops_Divide ||
           stream->current().getSubType() == SubTokenType::Ops_Modulus ||
           stream->current().getSubType() == SubTokenType::Ops_Exponent) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseUnary(stream, frame);

      switch (op) {
        case SubTokenType::Ops_Multiply:
          left = std::visit(MultiplyVisitor(stream->current()), left, right);
          break;

        case SubTokenType::Ops_Divide:
          left = std::visit(DivideVisitor(stream->current()), left, right);
          break;

        case SubTokenType::Ops_Modulus:
          left = std::visit(ModuloVisitor(stream->current()), left, right);
          break;

        case SubTokenType::Ops_Exponent:
          left = std::visit(PowerVisitor(stream->current()), left, right);
          break;

        default:
          break;
      }
    }

    return left;
  }

  Value parseUnary(std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CallStackFrame> frame) {
    while (stream->current().getSubType() == SubTokenType::Ops_Not ||
           stream->current().getSubType() == SubTokenType::Ops_Subtract ||
           stream->current().getSubType() == SubTokenType::Ops_BitwiseNot) {
      auto op = stream->current().getSubType();
      stream->next();  // Skip operator
      auto right = parseUnary(stream, frame);

      switch (op) {
        case SubTokenType::Ops_Not:
          if (std::holds_alternative<bool>(right)) {
            return !std::get<bool>(right);
          } else if (std::holds_alternative<k_int>(right)) {
            return std::get<k_int>(right) == 0 ? 1 : 0;
          } else if (std::holds_alternative<double>(right)) {
            return std::get<double>(right) == 0;
          } else if (std::holds_alternative<std::string>(right)) {
            return std::get<std::string>(right).empty();
          } else if (std::holds_alternative<std::shared_ptr<List>>(right)) {
            return std::get<std::shared_ptr<List>>(right)->elements.empty();
          } else if (std::holds_alternative<std::shared_ptr<Hash>>(right)) {
            return std::get<std::shared_ptr<Hash>>(right)->keys.empty();
          }

          throw ConversionError(stream->current(),
                                "Expected a `Boolean` expression.");
          break;

        case SubTokenType::Ops_Subtract:
          if (std::holds_alternative<k_int>(right)) {
            return -std::get<k_int>(right);
          } else if (std::holds_alternative<double>(right)) {
            return -std::get<double>(right);
          } else {
            throw ConversionError(
                stream->current(),
                "Unary minus applied to a non-numeric value.");
          }
          break;

        case SubTokenType::Ops_BitwiseNot:
          return std::visit(BitwiseNotVisitor(stream->current()), right);

        default:
          break;
      }
    }

    auto primary = parsePrimary(stream, frame);
    return interpretValueInvocation(stream, frame, primary);
  }

  Value parsePrimary(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    auto current = stream->current();
    auto& value = current.getValue();
    if (std::holds_alternative<k_int>(value) ||
        std::holds_alternative<double>(value) ||
        std::holds_alternative<bool>(value)) {
      stream->next();
      return value;
    }

    switch (current.getType()) {
      case TokenType::OPEN_BRACE:
        return interpretHash(stream, frame);

      case TokenType::OPEN_PAREN: {
        stream->next();  // Skip "("
        auto result = interpretExpression(stream, frame);

        if (stream->current().getType() == TokenType::CLOSE_PAREN) {
          stream->next();  // Skip ")"
        }

        return result;
      } break;

      case TokenType::OPEN_BRACKET:
        return interpretBracketExpression(stream, frame);

      case TokenType::IDENTIFIER:
        if (stream->peek().getType() == TokenType::OPEN_BRACKET) {
          auto variableName = current.getText();
          stream->next();  // Skip the identifier.
          return interpretSlice(stream, frame, variableName);
        }
        return interpretIdentifier(stream, frame, false);

      default:
        if (current.getSubType() == SubTokenType::KW_This) {
          return interpretSelfInvocationTerm(stream, frame);
        } else if (std::holds_alternative<std::string>(value)) {
          value = interpolateString(stream, frame);
          stream->next();
          return value;
        }
        break;
    }

    return 0;  // Default value.
  }

  Value interpretExpression(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto result = parseExpression(stream, frame);
    return result;//nterpretValueInvocation(stream, frame, result);
  }

  Value interpretSelfInvocationTerm(std::shared_ptr<TokenStream> stream,
                                    std::shared_ptr<CallStackFrame> frame) {
    if (!frame->inObjectContext()) {
      throw InvalidContextError(stream->current(),
                                "Invalid context for keyword `this`.");
    }

    if (!stream->match(TokenType::DOT)) {
      return frame->getObjectContext();
    }

    if (stream->current().getType() != TokenType::IDENTIFIER) {
      throw InvalidOperationError(
          stream->current(), "Syntax error near `this`. Missing identifier.");
    }
    auto identifier = stream->current().getText();
    stream->next();  // Skip the identifier

    auto clazz = classes[frame->getObjectContext()->className];

    if (clazz.hasMethod(identifier)) {
      interpretInstanceMethodInvocation(stream, frame,
                                        frame->getObjectContext()->identifier);

      if (!callStack.empty()) {
        return callStack.top()->returnValue;
      }
    } else if (frame->getObjectContext()->instanceVariables.find(identifier) !=
               frame->getObjectContext()->instanceVariables.end()) {
      return frame->getObjectContext()->instanceVariables[identifier];
    }

    throw UnimplementedMethodError(stream->current(), clazz.getClassName(),
                                   identifier);
  }

  std::string interpolateObject(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame,
                                Value& value) {
    auto object = std::get<std::shared_ptr<Object>>(value);
    auto clazz = classes[object->className];

    if (!clazz.hasMethod(KiwiBuiltins.ToS)) {
      return Serializer::basic_serialize_object(object);
    }

    std::vector<Value> parameters;

    // Should probably check that an overridden to_s() actually returns a string.
    return Serializer::serialize(interpretInstanceMethodInvocation(
        stream, frame, object, KiwiBuiltins.ToS, SubTokenType::Builtin_Kiwi_ToS,
        parameters));
  }

  Value interpolateString(std::shared_ptr<TokenStream> stream,
                          std::shared_ptr<CallStackFrame> frame,
                          const std::string& input) {
    if (input[0] == '@') {
      std::string name = input.substr(1);
      if (hasVariable(frame, name)) {
        return getVariable(stream, frame, name);
      }
    }

    auto tempId = InterpHelper::getTemporaryId();
    std::vector<Token> tempAssignment =
        InterpHelper::getTemporaryAssignment(stream->current(), tempId);

    Lexer lexer("", input);
    auto tokens = lexer.getAllTokens();
    tempAssignment.insert(tempAssignment.end(), tokens.begin(), tokens.end());

    auto tempStream = std::make_shared<TokenStream>(tempAssignment);
    auto tempFrame = buildSubFrame(frame);
    interpretAssignment(tempStream, tempFrame, "", true);

    if (tempFrame->variables.find(tempId) == tempFrame->variables.end()) {
      throw SyntaxError(stream->current(),
                        "Invalid string interpolation: `" + input + "`");
    }

    return getVariable(stream, tempFrame, tempId);
  }

  std::string interpolateString(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    auto input = stream->current().getText();
    std::ostringstream sv;

    for (size_t i = 0; i < input.length(); ++i) {
      char c = input[i];

      if (c == '$' && i + 1 < input.length() && input[i + 1] == '{') {
        i += 2;  // Skip "${"
        size_t start = i;
        int braceCount = 1;
        while (i < input.length() && braceCount > 0) {
          if (input[i] == '{') {
            ++braceCount;
          } else if (input[i] == '}') {
            --braceCount;
          }
          ++i;
        }
        if (braceCount == 0) {
          --i;  // Go back to the closing brace
          auto value =
              interpolateString(stream, frame, input.substr(start, i - start));
          if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
            sv << Serializer::serialize(value);
          } else {
            sv << interpolateObject(stream, frame, value);
          }
        } else {
          throw SyntaxError(
              stream->current(),
              "Unmatched braces in string interpolation: `" + input + "`");
        }
      } else if (c == '\\') {
        // Handle escape sequences
        if (i + 1 < input.length()) {
          switch (input[i + 1]) {
            case 't':
              sv << '\t';
              break;
            case 'n':
              sv << '\n';
              break;
            case 'r':
              sv << '\r';
              break;
            default:
              sv << input[i + 1];
              break;
          }
          i++;
        }
      } else {
        sv << c;
      }
    }

    return sv.str();
  }

  void interpretLambdaAssignment(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 const std::string& name,
                                 const SubTokenType& op) {
    if (op != SubTokenType::Ops_Assign) {
      throw InvalidOperationError(
          stream->current(),
          "Expected assignment operator in lambda assignment.");
    }

    Method lambda = interpretLambda(stream, frame);
    lambda.setName(name);
    frame->assignLambda(name, lambda);
  }

  void interpretHashElementAssignment(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame,
                                      const std::string& name, Value& value) {
    if (!stream->match(TokenType::OPEN_BRACKET)) {
      throw SyntaxError(
          stream->current(),
          "Expected open-bracket, `[`, in hash element assignment.");
    }

    auto keyValue = interpretExpression(stream, frame);

    if (!std::holds_alternative<std::string>(keyValue)) {
      throw SyntaxError(stream->current(), "Hash key must be a string value.");
    }

    if (!stream->match(TokenType::CLOSE_BRACKET)) {
      throw SyntaxError(
          stream->current(),
          "Expected close-bracket, `]`, in hash element assignment.");
    }

    if (!stream->matchsub(SubTokenType::Ops_Assign)) {
      throw InvalidOperationError(stream->current(),
                                  "Expected assignment operator.");
    }

    auto elementValue = interpretExpression(stream, frame);
    auto hashValue = std::get<std::shared_ptr<Hash>>(value);
    hashValue->add(std::get<std::string>(keyValue), elementValue);

    frame->variables[name] = hashValue;
  }

  void interpretSliceAssignment(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame,
                                const std::string& name) {
    auto value = getVariable(stream, frame, name);

    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      interpretHashElementAssignment(stream, frame, name, value);
      return;
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(stream->current(),
                                  "`" + name + "` is not a list.");
    }

    // Parse slice parameters (start:stop:step)
    auto slice = interpretSliceIndex(stream, frame, value);

    auto st = stream->current().getSubType();
    // Expect assignment operator next
    bool insertOp = st == SubTokenType::Ops_BitwiseLeftShiftAssign;
    bool simpleAssignOp = st == SubTokenType::Ops_Assign;
    if (!insertOp && !simpleAssignOp) {
      throw SyntaxError(stream->current(),
                        "Expected assignment operator in slice assignment.");
    }
    stream->next();  // Move past the assignment operator

    auto rhsValues = interpretExpression(stream, frame);

    InterpHelper::updateListSlice(stream, insertOp,
                                  std::get<std::shared_ptr<List>>(value), slice,
                                  Serializer::convert_value_to_list(rhsValues));
  }

  std::string interpretAssignment(std::shared_ptr<TokenStream> stream,
                                  std::shared_ptr<CallStackFrame> frame,
                                  const std::string& identifier = "",
                                  bool isTemporary = false,
                                  bool isInstanceVariable = false) {
    std::string name;

    if (!identifier.empty()) {
      name = identifier;
    } else if (stream->current().getType() == TokenType::IDENTIFIER) {
      name = stream->current().getText();
    }

    stream->next();  // Skip the identifier.

    switch (stream->current().getType()) {
      case TokenType::OPERATOR: {
        auto op = stream->current().getSubType();
        stream->next();

        if (Operators.is_assignment_operator(op) ||
            op == SubTokenType::Ops_BitwiseLeftShift) {
          interpretAssignment(stream, name, op, frame, isTemporary,
                              isInstanceVariable);
        }
      } break;

      case TokenType::OPEN_BRACKET:
        if (!hasVariable(frame, name)) {
          throw VariableUndefinedError(stream->current(), name);
        }

        if (!InterpHelper::isSliceAssignmentExpression(stream)) {
          throw SyntaxError(stream->current(),
                            "Invalid slice-assignment expression.");
        }

        interpretSliceAssignment(stream, frame, name);
        break;

      case TokenType::DOT:
        if (hasVariable(frame, name)) {
          auto value = getVariable(stream, frame, name);
          if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
            interpretInstanceMethodInvocation(stream, frame, name);
            return name;
          } else {
            throw InvalidOperationError(
                stream->current(), "Unsupported operation on `" + name + "`");
          }
        }

        throw VariableUndefinedError(stream->current(), name);
        break;

      default:
        break;
    }

    return name;
  }

  void interpretAssignment(std::shared_ptr<TokenStream> stream,
                           const std::string& name, const SubTokenType& op,
                           std::shared_ptr<CallStackFrame> frame,
                           bool isTemporary = false,
                           bool isInstanceVariable = false) {
    Value value;

    switch (stream->current().getType()) {
      case TokenType::LAMBDA:
        interpretLambdaAssignment(stream, frame, name, op);
        return;

      case TokenType::KEYWORD:
        if (stream->current().getSubType() == SubTokenType::KW_Await) {
          value = interpretAwait(stream, frame);
        } else {
          throw SyntaxError(stream->current(),
                            "Invalid syntax in assignment of `" + name + "`.");
        }
        break;

      default:
        value = interpretExpression(stream, frame);
        break;
    }

    if (op == SubTokenType::Ops_Assign) {
      if (!isTemporary && isInstanceVariable && frame->inObjectContext()) {
        frame->getObjectContext()->instanceVariables[name] = value;
      } else {
        if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
          auto object = std::get<std::shared_ptr<Object>>(value);
          object->identifier = name;
          value = object;
        }
        frame->variables[name] = value;
      }

      switch (stream->peek().getType()) {
        case TokenType::CLOSE_PAREN:
        case TokenType::CLOSE_BRACKET:
          stream->next();
          break;

        default:
          break;
      }

      return;
    }

    if (!hasVariable(frame, name)) {
      throw VariableUndefinedError(stream->current(), name);
    }

    if (op == SubTokenType::Ops_BitwiseLeftShift) {
      interpretAppendToList(stream, frame, value, name);
      return;
    }

    auto currentValue = getVariable(stream, frame, name);
    frame->variables[name] =
        InterpHelper::interpretAssignOp(stream, op, currentValue, value);
  }
};

#endif
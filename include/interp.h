#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <unordered_map>
#include <stack>
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
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"
#include "eventloop.h"
#include "globals.h"
#include "interp_builtin.h"
#include "interp_helper.h"
#include "stackframe.h"

class Interpreter {
 public:
  Interpreter(Logger& logger) : logger(logger) { eventLoop.startLoop(); }

  ~Interpreter() { eventLoop.stopLoop(); }

  void setKiwiArgs(const std::unordered_map<std::string, std::string>& args) {
    kiwiArgs = args;
  }

  int interpretKiwi(const std::string& kiwiCode) {
    Lexer lexer("", kiwiCode);
    return interpret(lexer);
  }

  int interpretScript(const std::string& path) {
    auto content = FileIO::readFile(path);
    if (content.empty()) {
      return -1;
    }

    auto cwd = FileIO::getCurrentDirectory();
    auto scriptDir = FileIO::getParentPath(path);
    FileIO::setCurrentDirectory(scriptDir);

    Lexer lexer(path, content);
    int result = interpret(lexer);

    FileIO::setCurrentDirectory(cwd);
    return result;
  }

  void preserveMainStackFrame() { preservingMainStackFrame = true; }

 private:
  Logger& logger;
  std::unordered_map<std::string, std::vector<std::string>> files;
  std::unordered_map<std::string, Method> methods;
  std::unordered_map<std::string, Module> modules;
  std::unordered_map<std::string, Class> classes;
  std::unordered_map<std::string, std::string> kiwiArgs;
  std::stack<std::shared_ptr<CallStackFrame>> callStack;
  std::stack<std::shared_ptr<TokenStream>> streamStack;
  std::stack<std::string> moduleStack;
  bool preservingMainStackFrame = false;
  EventLoop eventLoop;

  int interpret(Lexer& lexer) {
    files[lexer.getFile()] = lexer.getLines();
    auto stream = std::make_shared<TokenStream>(lexer.getAllTokens());
    return interpret(stream);
  }

  int interpret(std::shared_ptr<TokenStream> stream) {
    if (stream->empty()) {
      return 0;
    }

    auto mainFrame = std::make_shared<CallStackFrame>();
    callStack.push(mainFrame);
    streamStack.push(stream);

    interpretStackFrame();

    if (!preservingMainStackFrame && callStack.size() == 1) {
      callStack.pop();
    }

    return 0;
  }

  Token current(std::shared_ptr<TokenStream> stream) {
    if (stream->position >= stream->tokens.size()) {
      return Token::createStreamEnd();
    }
    return stream->tokens.at(stream->position);
  }

  void next(std::shared_ptr<TokenStream> stream) {
    if (stream->position < stream->tokens.size()) {
      stream->position++;
    }
  }

  Token peek(std::shared_ptr<TokenStream> stream) {
    size_t nextPosition = stream->position + 1;
    if (nextPosition < stream->tokens.size()) {
      return stream->tokens[nextPosition];
    } else {
      return Token::createStreamEnd();
    }
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

    while (stream->position < stream->tokens.size()) {
      auto& token = stream->tokens.at(stream->position);

      try {
        interpretToken(stream, token, frame);
      } catch (const KiwiError& e) {
        if (frame->isFlagSet(FrameFlags::InTry)) {
          frame->setErrorState(e);
        } else {
          ErrorHandler::handleError(e, files);
          exit(1);
        }
      } catch (const std::exception& e) {
        ErrorHandler::handleFatalError(e);
      }

      if (frame->isErrorStateSet()) {
        ++stream->position;
        continue;
      }

      if (frame->isFlagSet(FrameFlags::LoopBreak) ||
          frame->isFlagSet(FrameFlags::LoopContinue)) {
        if (callStack.size() > 1) {
          handleLoopControl(frame);
          return;
        }
      }

      if (frame->isFlagSet(FrameFlags::ReturnFlag)) {
        if (callStack.size() > 1) {
          handleFrameReturn(frame);
        } else {
          // If this is the main frame, just pop it
          callStack.pop();
        }

        return;
      }
    }

    if (callStack.size() < 2) {
      return;
    }

    handleFrameExit(frame);
  }

  void handleFrameExit(std::shared_ptr<CallStackFrame>& frame) {
    auto returnValue = std::move(frame->returnValue);
    bool doUpdate = true;
    bool inObjectContext = frame->inObjectContext();

    if (inObjectContext && std::holds_alternative<long long>(returnValue) &&
        std::get<long long>(returnValue) == 0) {
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
    Method lambda;

    if (current(stream).getType() == TokenType::IDENTIFIER) {
      auto lambdaName = current(stream).getText();
      next(stream);  // Skip identifier.
      if (frame->hasAssignedLambda(lambdaName)) {
        lambda = frame->getAssignedLambda(lambdaName);
      }
    } else if (current(stream).getType() == TokenType::LAMBDA) {
      lambda = interpretLambda(stream, frame);
    }

    return lambda;
  }

  Method interpretLambda(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "lambda"
    Method lambda;
    lambda.setName(InterpHelper::getTemporaryId());

    interpretMethodParameters(lambda, stream, frame);

    if (current(stream).getText() != Keywords.Do) {
      throw SyntaxError(current(stream), "Expected `do` in lambda expression.");
    }
    next(stream);  // Skip "do"

    std::vector<Token> lambdaTokens;
    InterpHelper::collectBodyTokens(lambdaTokens, stream);
    for (Token t : lambdaTokens) {
      lambda.addToken(t);
    }

    lambda.setFlag(MethodFlags::Lambda);

    return lambda;
  }

  void interpretHashLoop(std::shared_ptr<TokenStream> stream,
                         Value& collectionValue, const bool& hasIndexVariable,
                         const std::string& itemVariableName,
                         const std::string& indexVariableName,
                         std::shared_ptr<CallStackFrame> frame) {
    auto& collection = std::get<std::shared_ptr<Hash>>(collectionValue);

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, stream);

    // Execute the loop
    for (const auto& key : collection->keys) {
      if (frame->isFlagSet(FrameFlags::LoopBreak)) {
        break;
      }

      if (frame->isFlagSet(FrameFlags::LoopContinue)) {
        frame->clearFlag(FrameFlags::LoopContinue);
        continue;
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
                         Value& collectionValue, const bool& hasIndexVariable,
                         const std::string& itemVariableName,
                         const std::string& indexVariableName,
                         std::shared_ptr<CallStackFrame> frame) {
    auto& collection = std::get<std::shared_ptr<List>>(collectionValue);

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, stream);

    // Execute the loop
    size_t index = 0;
    for (const auto& item : collection->elements) {
      if (frame->isFlagSet(FrameFlags::LoopBreak)) {
        break;
      }

      if (frame->isFlagSet(FrameFlags::LoopContinue)) {
        frame->clearFlag(FrameFlags::LoopContinue);
        continue;
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
    if (current(stream).getType() == TokenType::DECLVAR) {
      next(stream);
      itemVariableName = current(stream).getText();
      next(stream);  // Skip the item variable name

      if (current(stream).getType() == TokenType::COMMA) {
        next(stream);  // Skip ','
        if (current(stream).getType() == TokenType::DECLVAR) {
          hasIndexVariable = true;
          next(stream);
          indexVariableName =
              current(stream)
                  .getText();  // Get the index variable name after '@'
          next(stream);        // Skip the index variable name
        }
      }
    }

    if (current(stream).getText() != Keywords.In) {
      throw SyntaxError(current(stream), "Expected 'in' after loop variables.");
    }

    next(stream);  // Skip "in"

    auto tokenTerm = current(stream);
    auto collectionValue = interpretExpression(stream, frame);

    if (peek(stream).getText() != Keywords.Do &&
        current(stream).getText() != Keywords.Do) {
      throw SyntaxError(current(stream), "Expected `do` in `for` loop.");
    }

    if (peek(stream).getText() == Keywords.Do) {
      next(stream);
    }

    if (current(stream).getText() == Keywords.Do) {
      next(stream);
    }

    if (std::holds_alternative<std::shared_ptr<List>>(collectionValue)) {
      interpretListLoop(stream, collectionValue, hasIndexVariable,
                        itemVariableName, indexVariableName, frame);
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(collectionValue)) {
      interpretHashLoop(stream, collectionValue, hasIndexVariable,
                        indexVariableName, itemVariableName, frame);
    } else {
      throw InvalidOperationError(tokenTerm, "Term is not a List or Hash.");
    }

    frame->clearFlag(FrameFlags::LoopBreak);
    frame->clearFlag(FrameFlags::LoopContinue);
  }

  void interpretWhileLoop(std::shared_ptr<TokenStream> stream,
                          std::shared_ptr<CallStackFrame> frame) {
    std::vector<Token> conditionTokens;
    while (stream->canRead() && current(stream).getText() != Keywords.Do) {
      auto t = current(stream);
      conditionTokens.push_back(current(stream));
      next(stream);
    }

    next(stream);  // Skip "do"
    auto tokenTerm = current(stream);

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, stream);

    auto oldFrame = std::make_shared<CallStackFrame>(*frame);
    auto tempId = InterpHelper::getTemporaryId();
    auto tempAssignment =
        InterpHelper::getTemporaryAssignment(tokenTerm, tempId);
    // Interpret the condition.
    while (true) {
      if (frame->isFlagSet(FrameFlags::LoopBreak)) {
        break;
      }

      if (frame->isFlagSet(FrameFlags::LoopContinue)) {
        frame->clearFlag(FrameFlags::LoopContinue);
        continue;
      }

      auto condition = conditionTokens;
      auto it = condition.begin() + 0;
      condition.insert(it, tempAssignment.begin(), tempAssignment.end());

      auto conditionStream = std::make_shared<TokenStream>(condition);
      auto conditionFrame = buildSubFrame(frame);
      callStack.push(conditionFrame);
      streamStack.push(conditionStream);
      interpretAssignment(conditionStream, conditionFrame, true);

      if (conditionFrame->variables.find(tempId) ==
          conditionFrame->variables.end()) {
        throw SyntaxError(current(stream), "Invalid condition in while-loop.");
      }

      auto value = getVariable(conditionStream, tempId, conditionFrame);
      callStack.pop();

      if (!std::holds_alternative<bool>(value)) {
        throw ConversionError(current(stream));
      }

      // Stop here.
      if (!std::get<bool>(value)) {
        break;
      }

      auto codeStream = std::make_shared<TokenStream>(loopTokens);
      callStack.push(buildSubFrame(frame));
      streamStack.push(codeStream);

      // Interpret the loop code.
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
    const auto& loop = current(stream).getText();
    next(stream);  // Skip "while"|"for"

    if (loop == Keywords.While) {
      interpretWhileLoop(stream, frame);
    } else if (loop == Keywords.For) {
      interpretForLoop(stream, frame);
    }
  }

  void interpretKiwiKeyword(std::shared_ptr<TokenStream> stream,
                            const Token& token,
                            std::shared_ptr<CallStackFrame> frame,
                            const std::string& keyword) {
    if (keyword == Keywords.If) {
      interpretConditional(stream, frame);
    } else if (keyword == Keywords.DeclVar) {
      interpretAssignment(stream, frame);
    } else if (Keywords.is_loop_keyword(keyword)) {
      interpretLoop(stream, frame);
    } else if (keyword == Keywords.Method) {
      interpretMethodDefinition(stream, frame);
    } else if (keyword == Keywords.Return) {
      interpretReturn(stream, frame);
    } else if (keyword == Keywords.Exit) {
      interpretExit(stream, frame);
    } else if (keyword == Keywords.PrintLn || keyword == Keywords.Print) {
      interpretPrint(stream, frame, keyword == Keywords.PrintLn);
    } else if (keyword == Keywords.Import) {
      interpretImport(stream, frame);
    } else if (keyword == Keywords.Export) {
      interpretExport(stream, frame);
    } else if (keyword == Keywords.Module) {
      interpretModuleDefinition(stream);
    } else if (keyword == Keywords.Delete) {
      interpretDelete(stream, frame);
    } else if (keyword == Keywords.Abstract || keyword == Keywords.Class) {
      interpretClassDefinition(stream, frame);
    } else if (keyword == Keywords.This) {
      interpretSelfInvocation(stream, frame);
    } else if (keyword == Keywords.Break) {
      next(stream);  // Skip "break"
      frame->setFlag(FrameFlags::LoopBreak);
    } else if (keyword == Keywords.Next) {
      next(stream);  // Skip "next"
      frame->setFlag(FrameFlags::LoopContinue);
    } else if (keyword == Keywords.Try) {
      next(stream);  // Skip "try"
      frame->setFlag(FrameFlags::InTry);
    } else if (keyword == Keywords.Pass) {
      next(stream);  // Skip "pass"
    } else if (keyword == Keywords.Catch) {
      interpretCatch(stream, frame);
    } else {
      throw UnrecognizedTokenError(
          token, "Unrecognized token `" + token.getText() + "`.");
    }
  }

  void interpretKeyword(std::shared_ptr<TokenStream> stream, const Token& token,
                        std::shared_ptr<CallStackFrame> frame) {
    const auto& keyword = token.getText();

    interpretKiwiKeyword(stream, token, frame, keyword);
  }

  void interpretQualifiedIdentifier(std::shared_ptr<TokenStream> stream,
                                    Token& token, std::string& identifier) {
    while (peek(stream).getType() == TokenType::QUALIFIER) {
      next(stream);  // Skip the identifier.
      next(stream);  // Skip the qualifier.
      token = current(stream);
      identifier += Symbols.Qualifier + token.getText();
    }
  }

  void interpretIdentifierOperation(std::shared_ptr<TokenStream> stream,
                                    const std::string& identifier,
                                    std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip identifier.
    auto op = current(stream).getText();
    next(stream);

    if (current(stream).getType() == TokenType::LAMBDA) {
      interpretLambdaAssignment(stream, identifier, op, frame);
    } else {
      throw InvalidOperationError(
          current(stream), "Invalid operation near `" + identifier + "`.");
    }
  }

  Value interpretIdentifier(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto token = current(stream);
    auto tokenText = token.getText();

    interpretQualifiedIdentifier(stream, token, tokenText);

    bool methodFound = hasMethod(tokenText, frame),
         classFound = hasClass(tokenText);

    if (methodFound || classFound) {
      if (classFound) {
        if (peek(stream).getType() != TokenType::DOT) {
          throw SyntaxError(current(stream),
                            "Invalid syntax near `" + tokenText + "`");
        }
        interpretClassMethodInvocation(stream, tokenText, frame);
      } else if (methodFound) {
        if (peek(stream).getType() == TokenType::COMMA ||
            peek(stream).getType() == TokenType::CLOSE_PAREN) {
          if (frame->hasAssignedLambda(tokenText)) {
            std::shared_ptr<LambdaRef> lambdaRef =
                std::make_shared<LambdaRef>(tokenText);
            return lambdaRef;
          } else {
            throw SyntaxError(current(stream), "Expected lambda reference.");
          }
        } else {
          interpretMethodInvocation(stream, tokenText, frame);
        }
      }

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);
        return value;
      }
    }

    if (hasVariable(tokenText, frame)) {
      auto v = getVariable(stream, tokenText, frame);
      next(stream);

      if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
        auto lambdaRef = std::get<std::shared_ptr<LambdaRef>>(v)->identifier;
        if (frame->hasAssignedLambda(lambdaRef)) {
          return interpretMethodInvocation(stream, lambdaRef, frame);
        } else {
          throw InvalidOperationError(token,
                                      "Unknown Lambda `" + tokenText + "`.");
        }
      }

      if (current(stream).getType() == TokenType::DOT) {
        return interpretDotNotation(stream, v, frame);
      }

      return v;
    } else if (KiwiBuiltins.is_builtin_method(tokenText)) {
      return interpretBuiltin(stream, tokenText, frame);
    } else if (peek(stream).getType() == TokenType::OPERATOR) {
      interpretIdentifierOperation(stream, tokenText, frame);
      return 0;
    }

    throw UnknownIdentifierError(token, tokenText);
  }

  void interpretCatch(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // SKip "catch"

    std::string errorVariableName;
    Value errorValue;

    if (current(stream).getType() == TokenType::OPEN_PAREN) {
      InterpHelper::interpretParameterizedCatch(stream, frame,
                                                errorVariableName, errorValue);
    }

    std::vector<Token> catchTokens;
    int count = 1;
    while (stream->canRead() && count != 0) {
      if (current(stream).getText() == Keywords.End) {
        --count;

        // Stop here.
        if (count == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getText())) {
        ++count;
      }

      catchTokens.push_back(current(stream));
      next(stream);
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

  void interpretToken(std::shared_ptr<TokenStream> stream, const Token& token,
                      std::shared_ptr<CallStackFrame> frame) {
    if (frame->isErrorStateSet()) {
      if (token.getType() == TokenType::KEYWORD &&
          token.getText() == Keywords.Catch) {
        interpretCatch(stream, frame);
      }
      return;
    }

    switch (token.getType()) {
      case TokenType::COMMENT:
      case TokenType::COMMA:
        next(stream);  // Skip these.
        break;

      case TokenType::KEYWORD:
      case TokenType::DECLVAR:
        interpretKeyword(stream, token, frame);
        break;

      case TokenType::IDENTIFIER:
        interpretIdentifier(stream, frame);
        break;

      case TokenType::CONDITIONAL:
        interpretConditional(stream, frame);
        break;

      default:
        throw UnrecognizedTokenError(
            token, "Unrecognized token `" + token.getText() + "`.");
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

  bool hasModule(const std::string& name) {
    return modules.find(name) != modules.end();
  }

  bool hasClass(const std::string& name) {
    return classes.find(name) != classes.end();
  }

  bool hasMethod(const std::string& name,
                 std::shared_ptr<CallStackFrame> frame) {
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

  bool hasVariable(const std::string& name,
                   std::shared_ptr<CallStackFrame> frame) {
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

    throw ModuleUndefinedError(current(stream), moduleName);
  }

  Module getModule(std::shared_ptr<TokenStream> stream,
                   const std::string& name) {
    if (hasModule(name)) {
      return modules[name];
    }

    throw ModuleUndefinedError(current(stream), name);
  }

  Method getMethod(std::shared_ptr<TokenStream> stream, const std::string& name,
                   std::shared_ptr<CallStackFrame> frame) {
    if (hasMethod(name, frame)) {
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

    throw MethodUndefinedError(current(stream), name);
  }

  Value getVariable(std::shared_ptr<TokenStream> stream,
                    const std::string& name,
                    std::shared_ptr<CallStackFrame> frame) {
    // Check in the current frame
    if (frame->variables.find(name) != frame->variables.end()) {
      Value value = frame->variables[name];
      return value;
    }

    if (frame->inObjectContext()) {
      if (frame->getObjectContext()->instanceVariables.find(name) !=
          frame->getObjectContext()->instanceVariables.end()) {
        Value value = frame->getObjectContext()->instanceVariables[name];
        return value;
      }
    }

    // Check in outer frames
    std::stack<std::shared_ptr<CallStackFrame>> tempStack(
        callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      auto& outerFrame = tempStack.top();
      if (outerFrame->variables.find(name) != outerFrame->variables.end()) {
        Value value = outerFrame->variables[name];
        return value;  // Found in an outer frame
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(current(stream), name);
  }

  std::vector<Value> interpretArguments(std::shared_ptr<TokenStream> stream,
                                        std::shared_ptr<CallStackFrame> frame) {
    std::vector<Value> args;

    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, near `" +
                            current(stream).getText() + "`.");
    }
    next(stream);  // Skip "("

    bool closeParenthesisFound = false;
    while (stream->canRead() && !closeParenthesisFound &&
           current(stream).getType() != TokenType::CLOSE_PAREN) {
      if (current(stream).getType() == TokenType::COMMA) {
        next(stream);
        continue;
      }

      auto subTokenTerm = current(stream);
      Value argValue;

      if (subTokenTerm.getType() == TokenType::TYPENAME) {
        argValue = subTokenTerm.getText();
      } else if (hasClass(subTokenTerm.getText())) {
        argValue = subTokenTerm.getText();
      } else {
        argValue = interpretExpression(stream, frame);
      }

      if (current(stream).getType() == TokenType::CLOSE_PAREN) {
        closeParenthesisFound = true;
      }

      args.push_back(argValue);

      if (!closeParenthesisFound) {
        next(stream);
      }
    }

    if (current(stream).getType() == TokenType::CLOSE_PAREN) {
      next(stream);
    }

    return args;
  }

  void interpretModuleBuiltin(std::shared_ptr<TokenStream> stream,
                              const std::string moduleName,
                              const std::string& builtin,
                              std::vector<Value>& args) {
    if (builtin == ModuleBuiltins.Home) {
      if (args.size() != 1) {
        throw BuiltinUnexpectedArgumentError(current(stream), builtin);
      }

      if (!std::holds_alternative<std::string>(args.at(0))) {
        throw SyntaxError(
            current(stream),
            "Expected string value for `" + builtin + "` builtin parameter.");
      }
      std::string value = std::get<std::string>(args.at(0));

      modules[moduleName].setHome(value);
      modules[moduleName].setName(moduleName);
    }
  }

  std::vector<std::string> collectMethodParameters(
      std::shared_ptr<TokenStream> stream, Token& tokenTerm, Method& method,
      std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected open-parenthesis, `(`, in method parameter set.");
    }
    next(stream);  // Skip "("

    // Interpret parameters.
    auto parameters = method.getParameters();
    int paramIndex = 0;

    bool closeParenthesisFound = false;
    while (stream->canRead() &&
           current(stream).getType() != TokenType::CLOSE_PAREN) {
      if (current(stream).getType() == TokenType::COMMA) {
        next(stream);
        continue;
      }

      auto paramName = parameters.at(paramIndex++);
      tokenTerm = current(stream);
      auto paramValue = interpretExpression(stream, frame);
      if (peek(stream).getType() == TokenType::CLOSE_PAREN) {
        next(stream);
        closeParenthesisFound = true;
      } else if (current(stream).getType() == TokenType::CLOSE_PAREN) {
        closeParenthesisFound = true;
      }
      method.addParameterValue(paramName, paramValue);
      if (!closeParenthesisFound) {
        next(stream);
      }
    }
    next(stream);  // Skip ")"

    return parameters;
  }

  std::shared_ptr<CallStackFrame> buildMethodInvocationStackFrame(
      std::shared_ptr<TokenStream> stream, Token& tokenTerm, Method& method,
      std::shared_ptr<CallStackFrame> frame) {
    auto parameters = collectMethodParameters(stream, tokenTerm, method, frame);
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
          throw ParameterMissingError(tokenTerm, parameterName);
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
                                       const std::string& className,
                                       std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip class name
    next(stream);  // Skip the "."

    auto methodName = current(stream).getText();
    next(stream);  // Skip the method name.

    auto clazz = classes[className];
    auto tokenTerm = current(stream);
    auto context = std::make_shared<Object>();
    bool isInstantiation = false;

    if (methodName == Keywords.New) {
      if (clazz.isAbstract()) {
        throw InvalidOperationError(tokenTerm,
                                    "Cannot instantiate an abstract class.");
      }

      if (!clazz.hasMethod(Keywords.Ctor)) {
        throw UnimplementedMethodError(tokenTerm, className, Keywords.Ctor);
      }

      methodName = Keywords.Ctor;
      isInstantiation = true;
    } else if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(tokenTerm, className, methodName);
    }

    auto method = clazz.getMethod(methodName);
    if (!method.isFlagSet(MethodFlags::Static) &&
        !method.isFlagSet(MethodFlags::Ctor)) {
      throw InvalidContextError(
          tokenTerm, "The method `" + methodName +
                         "` can only be invoked on an instance of class `" +
                         className + "`.");
    }

    if (method.isFlagSet(MethodFlags::Private)) {
      if (!frame->inObjectContext() ||
          frame->getObjectContext()->className != className) {
        throw InvalidContextError(
            current(stream),
            "Cannot invoke private method outside of object context.");
      }
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame =
        buildMethodInvocationStackFrame(stream, tokenTerm, method, frame);
    if (isInstantiation) {
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

    throw EmptyStackError(current(stream));
  }

  Value interpretInstanceMethodInvocation(
      std::shared_ptr<TokenStream> stream, std::shared_ptr<Object>& object,
      const std::string& methodName, std::vector<Value>& parameters,
      std::shared_ptr<CallStackFrame> frame) {
    if (!hasClass(object->className)) {
      throw ClassUndefinedError(current(stream), object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      if (KiwiBuiltins.is_builtin(methodName)) {
        return BuiltinInterpreter::execute(current(stream), methodName, object,
                                           parameters);
      }
      throw UnimplementedMethodError(current(stream), object->className,
                                     methodName);
    }

    auto method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame->inObjectContext()) {
      throw InvalidContextError(
          current(stream),
          "Cannot invoke private method outside of object context.");
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame = buildSubFrame(frame);

    if (static_cast<int>(parameters.size()) != method.getParameterCount()) {
      throw ParameterCountMismatchError(current(stream), methodName);
    }

    // Check all parameters are passed.
    int parameterIndex = 0;
    for (const std::string& parameterName : method.getParameters()) {
      codeFrame->variables[parameterName] = parameters.at(parameterIndex++);
    }
    codeFrame->setFlag(FrameFlags::SubFrame);
    codeFrame->setObjectContext(object);
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();

    Value value;
    if (!callStack.empty()) {
      value = callStack.top()->returnValue;
      frame->clearFlag(FrameFlags::ReturnFlag);
    }

    return value;
  }

  void interpretInstanceMethodInvocation(
      std::shared_ptr<TokenStream> stream, const std::string& instanceName,
      std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip the "."
    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          current(stream),
          "Expected identifier in instance method invocation, instead got: `" +
              current(stream).getText() + "`");
    }
    auto tokenTerm = current(stream);
    auto methodName = tokenTerm.getText();
    next(stream);  // Skip the method name.

    auto instanceValue = getVariable(stream, instanceName, frame);
    auto object = std::get<std::shared_ptr<Object>>(instanceValue);

    if (!hasClass(object->className)) {
      throw ClassUndefinedError(tokenTerm, object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(tokenTerm, object->className, methodName);
    }

    auto method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame->inObjectContext()) {
      throw InvalidContextError(
          tokenTerm, "Cannot invoke private method outside of object context.");
    }

    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame =
        buildMethodInvocationStackFrame(stream, tokenTerm, method, frame);
    codeFrame->setObjectContext(object);
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();
  }

  Value interpretMethodInvocation(std::shared_ptr<TokenStream> stream,
                                  const std::string& name,
                                  std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      next(stream);  // Skip the name.
    }

    auto method = getMethod(stream, name, frame);
    auto tokenTerm = current(stream);
    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame =
        buildMethodInvocationStackFrame(stream, tokenTerm, method, frame);
    if (frame->inObjectContext()) {
      codeFrame->setObjectContext(frame->getObjectContext());
    }
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();

    Value value;

    if (!callStack.empty()) {
      value = callStack.top()->returnValue;
    }

    return value;
  }

  std::vector<Parameter> getParameterSet(
      std::shared_ptr<TokenStream> stream,
      std::shared_ptr<CallStackFrame> frame) {
    auto tokenTerm = current(stream);
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected open-parenthesis, `(`, in parameter set expression.");
    }
    next(stream);  // Skip "("

    std::unordered_set<std::string> paramNames;
    std::vector<Parameter> params;

    while (stream->canRead() &&
           current(stream).getType() != TokenType::CLOSE_PAREN) {
      auto paramToken = current(stream);
      auto paramName = paramToken.getText();
      if (paramToken.getType() == TokenType::IDENTIFIER) {
        if (paramNames.find(paramName) != paramNames.end()) {
          throw SyntaxError(paramToken, "The parameter `" + paramName +
                                            "` was specified more than once.");
        }

        paramNames.insert(paramName);
        next(stream);

        if (current(stream).getType() == TokenType::OPERATOR &&
            current(stream).getText() == Operators.Assign) {
          next(stream);  // Skip "=".
          auto paramValue = interpretExpression(stream, frame);
          Parameter optionalParam(paramName, paramValue);
          params.push_back(optionalParam);
          continue;
        }

        Parameter param(paramName);
        params.push_back(param);

        continue;
      }
      next(stream);
    }

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected close-parenthesis, `)`, in parameter set expression.");
    }
    next(stream);  // Skip ")"

    return params;
  }

  void interpretMethodParameters(Method& method,
                                 std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame) {
    for (const auto& param : getParameterSet(stream, frame)) {
      method.addParameter(param);
    }
  }

  Method interpretMethodDeclaration(std::shared_ptr<TokenStream> stream,
                                    std::shared_ptr<CallStackFrame> frame) {
    Method method;

    while (stream->canRead() && current(stream).getText() != Keywords.Method) {
      if (current(stream).getText() == Keywords.Abstract) {
        method.setFlag(MethodFlags::Abstract);
      } else if (current(stream).getText() == Keywords.Override) {
        method.setFlag(MethodFlags::Override);
      } else if (current(stream).getText() == Keywords.Private) {
        method.setFlag(MethodFlags::Private);
      } else if (current(stream).getText() == Keywords.Static) {
        method.setFlag(MethodFlags::Static);
      }
      next(stream);
    }
    next(stream);  // Skip "def"

    auto name = current(stream).getText();
    method.setName(name);
    next(stream);  // Skip the name.
    interpretMethodParameters(method, stream, frame);
    int counter = 1;

    if (method.isFlagSet(MethodFlags::Abstract)) {
      return method;
    }

    while (stream->canRead() && counter > 0) {
      if (current(stream).getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getText())) {
        ++counter;
      }

      auto codeToken = current(stream);
      method.addToken(codeToken);
      next(stream);

      if (current(stream).getType() == TokenType::STREAM_END) {
        throw SyntaxError(current(stream),
                          "Invalid method declaration `" + name + "`");
      }
    }

    return method;
  }

  void interpretMethodDefinition(std::shared_ptr<TokenStream> stream,
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
      Token tokenTerm = current(stream);
      throw IllegalNameError(tokenTerm, name);
    }

    method.setName(name);
    methods[name] = method;
  }

  void interpretExit(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    bool hasValue = InterpHelper::hasReturnValue(stream);
    next(stream);  // Skip "exit"

    Value returnValue;

    if (hasValue) {
      returnValue = interpretExpression(stream, frame);
    }

    if (std::holds_alternative<long long>(returnValue)) {
      int exitCode = static_cast<int>(std::get<long long>(returnValue));
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
    next(stream);  // Skip "return"

    Value returnValue;

    if (hasValue) {
      returnValue = interpretExpression(stream, frame);
    }

    frame->returnValue = returnValue;
    frame->setFlag(FrameFlags::ReturnFlag);
  }

  std::string interpretAssignment(std::shared_ptr<TokenStream> stream,
                                  std::shared_ptr<CallStackFrame> frame,
                                  bool isTemporary = false,
                                  bool isInstanceVariable = false) {
    std::string name;
    if (current(stream).getType() == TokenType::DECLVAR) {
      next(stream);  // Skip the "@"
    }

    if (current(stream).getType() == TokenType::IDENTIFIER) {
      name = current(stream).toString();
      next(stream);

      if (current(stream).getType() == TokenType::OPERATOR) {
        auto op = current(stream).toString();
        next(stream);

        if (Operators.is_assignment_operator(op) ||
            op == Operators.ListAppend) {
          interpretAssignment(stream, name, op, frame, isTemporary,
                              isInstanceVariable);
        }
      } else if (current(stream).getType() == TokenType::OPEN_BRACKET) {
        if (!hasVariable(name, frame)) {
          throw VariableUndefinedError(current(stream), name);
        }

        if (!InterpHelper::isSliceAssignmentExpression(stream)) {
          throw SyntaxError(current(stream),
                            "Invalid slice-assignment expression.");
        }

        interpretSliceAssignment(stream, frame, name);
      } else if (current(stream).getType() == TokenType::DOT) {
        if (hasVariable(name, frame)) {
          auto value = getVariable(stream, name, frame);
          if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
            interpretInstanceMethodInvocation(stream, name, frame);
            return name;
          } else {
            throw InvalidOperationError(
                current(stream), "Unsupported operation on `" + name + "`");
          }
        }

        throw VariableUndefinedError(current(stream), name);
      }
    }

    return name;
  }

  void interpretHashElementAssignment(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame,
                                      const std::string& name, Value& value) {
    if (current(stream).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(
          current(stream),
          "Expected open-bracket, `[`, in hash element assignment.");
    }
    next(stream);  // Skip "["

    auto keyValue = interpretExpression(stream, frame);

    if (!std::holds_alternative<std::string>(keyValue)) {
      throw SyntaxError(current(stream), "Hash key must be a string value.");
    }

    auto key = std::get<std::string>(keyValue);

    if (peek(stream).getType() == TokenType::CLOSE_BRACKET) {
      next(stream);
    }

    if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(
          current(stream),
          "Expected close-bracket, `]`, in hash element assignment.");
    }
    next(stream);

    if (current(stream).getText() != Operators.Assign) {
      throw InvalidOperationError(current(stream),
                                  "Expected assignment operator.");
    }
    next(stream);

    auto elementValue = interpretExpression(stream, frame);
    auto hashValue = std::get<std::shared_ptr<Hash>>(value);
    hashValue->add(key, elementValue);

    frame->variables[name] = hashValue;
  }

  void interpretSliceAssignment(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame,
                                const std::string& name) {
    auto value = getVariable(stream, name, frame);

    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      interpretHashElementAssignment(stream, frame, name, value);
      return;
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(current(stream),
                                  "`" + name + "` is not a list.");
    }

    // Parse slice parameters (start:stop:step)
    auto slice = interpretSliceIndex(stream, frame, value);

    if (peek(stream).getType() == TokenType::OPERATOR) {
      next(stream);
    }

    // Expect assignment operator next
    bool insertOp = current(stream).getText() == Operators.ListInsert;
    bool simpleAssignOp = current(stream).getText() == Operators.Assign;
    if (!insertOp && !simpleAssignOp) {
      throw SyntaxError(current(stream),
                        "Expected assignment operator in slice assignment.");
    }
    next(stream);  // Move past the assignment operator

    auto rhsValues = interpretExpression(stream, frame);

    // Get the target list to update
    auto targetListPtr = std::get<std::shared_ptr<List>>(value);

    auto rhsList = Serializer::convert_value_to_list(rhsValues);
    InterpHelper::updateListSlice(stream, insertOp, targetListPtr, slice,
                                  rhsList);
  }

  SliceIndex interpretSliceIndex(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 Value& listValue) {
    if (current(stream).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(current(stream),
                        "Expected open-bracket, `[`, for list access.");
    }
    next(stream);  // Skip "["

    if (!std::holds_alternative<std::shared_ptr<List>>(listValue)) {
      throw InvalidOperationError(current(stream), "Expected a list type for list access operation.");
    }
    auto listPtr = std::get<std::shared_ptr<List>>(listValue);

    SliceIndex slice;
    slice.indexOrStart = 0;
    slice.stopIndex = static_cast<int>(listPtr->elements.size());
    slice.stepValue = 1;
    slice.isSlice = false;

    // Detect if slicing or single index
    if (peek(stream).getType() == TokenType::COLON ||
        current(stream).getType() == TokenType::COLON) {
      slice.isSlice = true;
      if (current(stream).getType() != TokenType::COLON) {
        slice.indexOrStart = interpretExpression(stream, frame);
      }

      if (peek(stream).getType() == TokenType::COLON) {
        next(stream);  // Skip colon for start:stop:
      }

      if (current(stream).getType() == TokenType::COLON) {
        next(stream);  // Skip colon for start::step or ::step
        if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
          slice.stopIndex = interpretExpression(stream, frame);
        }
      }

      if (current(stream).getType() == TokenType::COLON) {
        next(stream);  // Skip colon for ::step
        if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
          slice.stepValue = interpretExpression(stream, frame);
        }
      }
    } else if (current(stream).getType() == TokenType::QUALIFIER) {
      next(stream);
      slice.isSlice = true;
      if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
        slice.stepValue = interpretExpression(stream, frame);
      }
    } else {
      // Single index
      slice.indexOrStart = interpretExpression(stream, frame);
    }

    if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(current(stream),
                        "Expected close-bracket, `]`, for list access.");
    }
    next(stream);

    return slice;
  }

  Value interpretKeyOrIndex(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(current(stream),
                        "Expected open-bracket, `[`, in key or index access.");
    }
    next(stream);  // Skip "["

    auto output = interpretExpression(stream, frame);

    if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(current(stream),
                        "Expected close-bracket, `]`, in key or index access.");
    }
    next(stream);  // Skip "]"

    return output;
  }

  std::string interpretKey(std::shared_ptr<TokenStream> stream,
                           std::shared_ptr<CallStackFrame> frame) {
    auto output = interpretKeyOrIndex(stream, frame);

    if (!std::holds_alternative<std::string>(output)) {
      throw SyntaxError(current(stream), "Hash key must be a string value.");
    }

    return std::get<std::string>(output);
  }

  int interpretIndex(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    auto output = interpretKeyOrIndex(stream, frame);

    if (!std::holds_alternative<long long>(output)) {
      throw SyntaxError(current(stream),
                        "List index must be an integer value.");
    }

    return std::get<long long>(output);
  }

  Value interpretHashElementAccess(std::shared_ptr<TokenStream> stream,
                                   std::shared_ptr<CallStackFrame> frame,
                                   Value& value) {
    auto tokenTerm = current(stream);
    auto key = interpretKey(stream, frame);
    auto hash = std::get<std::shared_ptr<Hash>>(value);

    if (!hash->hasKey(key)) {
      throw HashKeyError(current(stream), key);
    }

    return hash->get(key);
  }

  Value interpretSlice(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame,
                       const std::string& name) {
    auto value = getVariable(stream, name, frame);
    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      return interpretHashElementAccess(stream, frame, value);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(current(stream),
                                  "`" + name + "` is not a list.");
    }

    auto slice = interpretSliceIndex(stream, frame, value);
    auto list = std::get<std::shared_ptr<List>>(value);

    return InterpHelper::interpretListSlice(stream, slice, list);
  }

  void interpretAppendToList(std::shared_ptr<TokenStream> stream,
                             std::shared_ptr<CallStackFrame> frame,
                             Value& listValue,
                             const std::string& listVariableName) {
    auto tokenTerm = current(stream);

    Value variableValue;
    try {
      variableValue = getVariable(stream, listVariableName, frame);
    } catch (const VariableUndefinedError& e) {
      throw VariableUndefinedError(tokenTerm, listVariableName);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(variableValue)) {
      throw InvalidOperationError(tokenTerm,
                                  "`" + listVariableName + "` is not a list.");
    }

    auto& listPtr = std::get<std::shared_ptr<List>>(variableValue);
    listPtr->elements.push_back(listValue);
  }

  std::shared_ptr<List> interpretRange(std::shared_ptr<TokenStream> stream,
                                       std::shared_ptr<CallStackFrame> frame) {
    auto list = std::make_shared<List>();

    next(stream);  // Skip the "["

    auto startValue = interpretExpression(stream, frame);

    if (current(stream).getType() != TokenType::RANGE) {
      throw RangeError(current(stream),
                       "Expected range separator, `..`, in range expression.");
    }

    next(stream);  // Skip the ".."

    auto stopValue = interpretExpression(stream, frame);

    if (current(stream).getType() != TokenType::CLOSE_BRACKET) {
      throw RangeError(current(stream),
                       "Expected close-bracket, `]`, in range expression.");
    }
    next(stream);  // Skip the "]"

    if (!std::holds_alternative<long long>(startValue)) {
      throw RangeError(current(stream),
                       "A range start value must be an integer.");
    }

    if (!std::holds_alternative<long long>(stopValue)) {
      throw RangeError(current(stream),
                       "A range stop value must be an integer.");
    }

    int start = std::get<long long>(startValue),
        stop = std::get<long long>(stopValue);
    int step = stop < start ? -1 : 1;
    int i = start;

    for (; i != stop; i += step) {
      list->elements.push_back(i);
    }
    list->elements.push_back(i);

    return list;
  }

  std::shared_ptr<Hash> interpretHash(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip the "{"

    auto hash = std::make_shared<Hash>();

    while (stream->canRead() &&
           current(stream).getType() != TokenType::CLOSE_BRACE) {
      auto keyValue = interpretExpression(stream, frame);

      if (!std::holds_alternative<std::string>(keyValue)) {
        throw SyntaxError(current(stream), "Hash key must be a string value.");
      }

      auto key = std::get<std::string>(keyValue);

      if (current(stream).getType() == TokenType::COLON) {
        next(stream);  // Skip the ":"
        auto value = interpretExpression(stream, frame);
        hash->add(key, value);
      }

      if (current(stream).getType() == TokenType::COMMA) {
        next(stream);
      }
    }

    if (current(stream).getType() == TokenType::CLOSE_BRACE) {
      next(stream);  // Skip the "}"
    }

    return hash;
  }

  std::shared_ptr<List> interpretList(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame) {
    auto list = std::make_shared<List>();
    next(stream);  // Skip "["
    int bracketCount = 1;

    while (stream->canRead() && bracketCount > 0) {
      if (current(stream).getType() == TokenType::OPEN_BRACKET) {
        ++bracketCount;
        // It's another list.
        auto value = interpretExpression(stream, frame);
        list->elements.push_back(value);
        continue;
      } else if (current(stream).getType() == TokenType::CLOSE_BRACKET) {
        --bracketCount;

        // Stop here.
        if (bracketCount == 0) {
          next(stream);  // Skip "]"
          break;
        }
      } else if (current(stream).getType() == TokenType::COMMA) {
        next(stream);
        continue;
      } else {
        auto value = interpretExpression(stream, frame);
        list->elements.push_back(value);
        continue;
      }

      if (current(stream).getType() == TokenType::COMMA ||
          current(stream).getType() == TokenType::CLOSE_BRACKET) {
        continue;
      }

      next(stream);
    }

    return list;
  }

  void interpretConditional(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getText() != Keywords.If) {
      throw SyntaxError(current(stream), "Invalid conditional. Expected `" +
                                             Keywords.If +
                                             "` keyword, instead got: `" +
                                             current(stream).getText() + "`");
    }

    next(stream);  // Skip "if"

    Conditional conditional;
    int ifCount = 1;
    bool shortCircuitIf = false;
    bool shortCircuitElseIf = false;
    auto building = Keywords.If;

    // Eagerly evaluate the If conditions.
    auto value = interpretExpression(stream, frame);

    if (!std::holds_alternative<bool>(value)) {
      throw ConversionError(current(stream));
    }

    shortCircuitIf = std::get<bool>(value);
    conditional.getIfStatement().setEvaluation(shortCircuitIf);

    while (stream->canRead() && ifCount > 0) {
      if (Keywords.is_block_keyword(current(stream).getText())) {
        ++ifCount;
      } else if (current(stream).getText() == Keywords.End && ifCount > 0) {
        --ifCount;

        // Stop here.
        if (ifCount == 0) {
          next(stream);
          continue;
        }
      } else if (current(stream).getText() == Keywords.Else && ifCount == 1) {
        next(stream);
        building = Keywords.Else;
      } else if (current(stream).getText() == Keywords.ElseIf && ifCount == 1) {
        next(stream);
        building = Keywords.ElseIf;

        conditional.addElseIfStatement();

        // No need to evaluate if the previous condition is true.
        if (shortCircuitIf || shortCircuitElseIf) {
          continue;
        }

        // Eagerly evaluate ElseIf conditions.
        value = interpretExpression(stream, frame);

        if (!std::holds_alternative<bool>(value)) {
          throw ConversionError(current(stream));
        }

        bool elseIfValue = std::get<bool>(value);

        if (elseIfValue) {
          // Don't evaluate future ElseIf branches.
          shortCircuitElseIf = true;
        }

        conditional.getElseIfStatement().setEvaluation(elseIfValue);
      }

      // Distribute tokens to be executed.
      Token bodyToken = current(stream);
      if (shortCircuitIf && building == Keywords.If) {
        conditional.getIfStatement().addToken(bodyToken);
      } else if (!shortCircuitIf && building == Keywords.ElseIf) {
        conditional.getElseIfStatement().addToken(bodyToken);
      } else if (!shortCircuitIf && !shortCircuitElseIf &&
                 building == Keywords.Else) {
        conditional.getElseStatement().addToken(bodyToken);
      }

      next(stream);
    }

    // Evaluate the Conditional object.
    std::vector<Token> executableTokens;

    if (conditional.getIfStatement().isExecutable()) {
      executableTokens = conditional.getIfStatement().getCode();
    } else if (conditional.canExecuteElseIf()) {
      for (ElseIfStatement elseIf : conditional.getElseIfStatements()) {
        if (elseIf.isExecutable()) {
          executableTokens = elseIf.getCode();
          break;
        }
      }
    } else {
      executableTokens = conditional.getElseStatement().getCode();
    }

    // Execute
    if (!executableTokens.empty()) {
      auto executableStream = std::make_shared<TokenStream>(executableTokens);
      auto codeFrame = buildSubFrame(frame);
      callStack.push(codeFrame);
      streamStack.push(executableStream);
      interpretStackFrame();
    }
  }

  Value interpretSelfInvocation(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    if (!frame->inObjectContext()) {
      throw InvalidContextError(current(stream),
                                "Invalid context for keyword `this`.");
    }

    next(stream);  // Skip "this"
    if (current(stream).getType() != TokenType::DOT) {
      throw SyntaxError(current(stream), "Invalid syntax near keyword `this`.");
    }
    next(stream);  // Skip "."

    Value value;

    if (current(stream).getType() == TokenType::DECLVAR) {
      value = interpretAssignment(stream, frame, false, true);
    } else if (current(stream).getType() == TokenType::IDENTIFIER &&
               peek(stream).getType() == TokenType::OPEN_PAREN) {
      value =
          interpretMethodInvocation(stream, current(stream).getText(), frame);
    }

    return value;
  }

  void interpretClassDefinition(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    bool isAbstract = current(stream).getText() == Keywords.Abstract;
    std::string moduleName;
    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    while (stream->canRead() && current(stream).getText() != Keywords.Class) {
      next(stream);
    }
    next(stream);  // Skip "class"

    Token tokenTerm = current(stream);
    auto className = current(stream).getText();
    next(stream);  // Skip class name.

    if (classes.find(className) != classes.end()) {
      throw ClassRedefinitionError(tokenTerm, className);
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
        throw ClassUndefinedError(tokenTerm, baseClassName);
      }

      // inherit methods from base class.
      auto baseClass = classes[baseClassName];
      for (auto& pair : baseClass.getMethods()) {
        clazz.addMethod(pair.second);
      }
    }

    int counter = 1;
    while (stream->canRead() && counter > 0) {
      auto tokenText = current(stream).getText();
      if (tokenText == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (tokenText == Keywords.Abstract ||
                 tokenText == Keywords.Override ||
                 tokenText == Keywords.Method ||
                 tokenText == Keywords.Private ||
                 tokenText == Keywords.Static) {
        if (tokenText == Keywords.Private) {
          if (peek(stream).getType() == TokenType::OPEN_PAREN) {
            next(stream);  // Skip "private"
            for (auto privateVar : getParameterSet(stream, frame)) {
              clazz.addPrivateVariable(privateVar);
            }
          }
        }

        auto method = interpretMethodDeclaration(stream, frame);

        if (method.getName() == Keywords.Ctor) {
          method.setFlag(MethodFlags::Ctor);
        }

        if (clazz.hasMethod(method.getName())) {
          auto classMethod = clazz.getMethod(method.getName());
          if (!method.isFlagSet(MethodFlags::Override) &&
              classMethod.isFlagSet(MethodFlags::Abstract)) {
            throw SyntaxError(current(stream),
                              "The class, `" + className +
                                  "` has an abstract definition for `" +
                                  method.getName() +
                                  "` and the `override` keyword is missing.");
          }
        }

        clazz.addMethod(method);
        continue;
      }

      next(stream);
    }

    // Check for unimplemented abstract methods.
    if (!clazz.isAbstract()) {
      for (const auto& pair : clazz.getMethods()) {
        if (pair.second.isFlagSet(MethodFlags::Abstract)) {
          throw UnimplementedMethodError(current(stream), className,
                                         pair.second.getName());
        }
      }
    }

    classes[className] = std::move(clazz);
  }

  void interpretModuleDefinition(std::shared_ptr<TokenStream> stream) {
    next(stream);  // Skip "module"

    auto name = current(stream).getText();
    if (hasModule(name)) {
      // WIP: Mixins?
    }

    next(stream);  // Skip the module name.

    Module module;
    int counter = 1;
    while (stream->canRead() && counter > 0) {
      if (current(stream).getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getText())) {
        ++counter;
      }

      Token codeToken = current(stream);
      module.addToken(codeToken);
      next(stream);
    }

    modules[name] = std::move(module);
  }

  std::string interpretModuleImport(std::shared_ptr<TokenStream> stream,
                                    const std::string& home,
                                    const std::string& name) {
    next(stream);  // Skip the name.

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
      throw ConversionError(current(stream),
                            "Expected a string for `import` statement.");
    }

    auto scriptName = std::get<std::string>(scriptNameValue);
    if (!Strings::ends_with(scriptName, ".kiwi")) {
      scriptName += ".kiwi";
    }

    auto scriptPath = FileIO::getLocalPath(scriptName);
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    auto content = FileIO::readFile(scriptPath);
    if (content.empty()) {
      return "";
    }

    Lexer lexer(scriptPath, content);
    files[scriptPath] = lexer.getLines();
    auto scriptStream = lexer.getTokenStream();
    callStack.push(buildSubFrame(frame));
    streamStack.push(scriptStream);
    interpretStackFrame();

    std::string moduleName;

    // Check if a module was imported.
    auto returnValue = frame->returnValue;
    if (std::holds_alternative<std::string>(returnValue)) {
      moduleName = std::get<std::string>(returnValue);

      if (!hasModule(moduleName)) {
        moduleName.clear();
      }
    }

    return moduleName;
  }

  void interpretModuleAlias(std::shared_ptr<TokenStream> stream,
                            const std::string& moduleName) {
    next(stream);  // Skip "as"

    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(current(stream), "Expected identifier for alias.");
    }

    auto alias = current(stream).getText();
    next(stream);  // Skip the alias

    auto search = moduleName + Symbols.Qualifier;

    if (hasClass(alias)) {
      throw InvalidOperationError(
          current(stream), "The module alias `" + alias + "` is in use.");
    }

    Class clazz;
    clazz.setClassName(alias);

    for (auto pair : methods) {
      auto name = pair.first;
      if (Strings::begins_with(name, search)) {
        auto method = pair.second;
        method.setFlag(MethodFlags::Static);
        method.setName(Strings::replace(name, search, ""));
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
    next(stream);  // skip the "export"

    auto moduleName = current(stream).getText();
    auto moduleHome = InterpHelper::interpretModuleHome(moduleName, stream);

    if (hasModule(moduleName)) {
      moduleName = interpretModuleImport(stream, moduleHome, moduleName);
      frame->returnValue = moduleName;
      frame->setFlag(FrameFlags::ReturnFlag);
    } else {
      throw InvalidOperationError(current(stream),
                                  "Invalid export `" + moduleName + "`");
    }
  }

  void interpretImport(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // skip the "import"

    auto tokenText = current(stream).getText();
    auto moduleName = tokenText;
    auto moduleHome = InterpHelper::interpretModuleHome(moduleName, stream);

    if (hasModule(moduleName)) {
      moduleName = interpretModuleImport(stream, moduleHome, moduleName);
    } else {
      moduleName = interpretExternalImport(stream, frame);
    }

    if (current(stream).getText() == Keywords.As) {
      interpretModuleAlias(stream, moduleName);
    }
  }

  Value interpretVariableValue(std::shared_ptr<TokenStream> stream,
                               std::string name,
                               std::shared_ptr<CallStackFrame> frame) {
    if (!hasVariable(name, frame)) {
      throw KiwiError(current(stream), "Unknown variable `" + name + "`");
    }

    return interpretExpression(stream, frame);
  }

  void interpretDeleteHashKey(std::shared_ptr<TokenStream> stream,
                              std::shared_ptr<CallStackFrame> frame,
                              const std::string& name, Value& value) {
    auto key = interpretKey(stream, frame);
    auto hash = std::get<std::shared_ptr<Hash>>(value);

    if (!hash->hasKey(key)) {
      throw HashKeyError(current(stream), key);
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
      throw RangeError(current(stream), "List index out of range.");
    }

    list->elements.erase(list->elements.begin() + index);
    frame->variables[name] = list;
  }

  void interpretDelete(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "delete"

    if (current(stream).getType() == TokenType::DECLVAR) {
      next(stream);
      auto name = current(stream).getText();

      if (!hasVariable(name, frame)) {
        throw VariableUndefinedError(current(stream), name);
      }

      auto value = getVariable(stream, name, frame);
      next(stream);

      if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
        interpretDeleteHashKey(stream, frame, name, value);
      } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
        interpretDeleteListIndex(stream, frame, name, value);
      }

      return;
    }

    throw SyntaxError(current(stream),
                      "Cannot delete from a non-variable value.");
  }

  void interpretPrint(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame,
                      bool printNewLine = false) {
    next(stream);  // skip the "print"

    Value value = interpretExpression(stream, frame);

    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      std::cout << Serializer::serialize(value);
    } else {
      std::cout << interpolateObject(stream, value, frame);
    }

    if (printNewLine) {
      std::cout << std::endl;
    }
  }

  Value interpretBracketExpression(std::shared_ptr<TokenStream> stream,
                                   std::shared_ptr<CallStackFrame> frame) {
    if (InterpHelper::isRangeExpression(stream)) {
      return interpretRange(stream, frame);
    } else if (InterpHelper::isListExpression(stream)) {
      return interpretList(stream, frame);
    }

    throw SyntaxError(current(stream), "Invalid bracket expression.");
  }

  Value interpretBuiltin(std::shared_ptr<TokenStream> stream,
                         const std::string& builtin,
                         std::shared_ptr<CallStackFrame> frame) {
    auto tokenTerm = current(stream);
    next(stream);  // Skip the name.

    auto args = interpretArguments(stream, frame);

    if (ModuleBuiltins.is_builtin(builtin)) {
      if (moduleStack.empty()) {
        throw InvalidContextError(
            tokenTerm, "Builtin `" + builtin +
                           "` is illegal outside of a module context.");
      }
      auto moduleName = moduleStack.top();
      interpretModuleBuiltin(stream, moduleName, builtin, args);
      return 0;
    }

    frame->returnValue =
        BuiltinInterpreter::execute(tokenTerm, builtin, args, kiwiArgs);
    return frame->returnValue;
  }

  Value interpretLambdaMap(std::shared_ptr<TokenStream> stream,
                           const std::shared_ptr<List>& list,
                           std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
          "Expected a lambda in `" + SpecializedBuiltins.Map + "` builtin.");
    }

    std::string itemVariableName, indexVariableName;
    bool hasIndexVariable = false;

    for (std::string parameter : lambda.getParameters()) {
      if (itemVariableName.empty()) {
        itemVariableName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(current(stream),
                                             SpecializedBuiltins.Map);
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
                              const std::shared_ptr<List>& list,
                              std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "("

    auto accumulator = interpretExpression(stream, frame);

    if (current(stream).getType() != TokenType::LAMBDA) {
      next(stream);
    }

    if (current(stream).getType() == TokenType::COMMA) {
      next(stream);
    }

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
          "Expected a lambda in `" + SpecializedBuiltins.Reduce + "` builtin.");
    }

    std::string accumulatorName, indexVariableName;
    bool hasIndexVariable = false;
    for (std::string parameter : lambda.getParameters()) {
      if (accumulatorName.empty()) {
        accumulatorName = parameter;
      } else if (indexVariableName.empty()) {
        indexVariableName = parameter;
        hasIndexVariable = true;
      } else {
        throw BuiltinUnexpectedArgumentError(current(stream),
                                             SpecializedBuiltins.Reduce);
      }
    }

    auto mappedList = std::make_shared<List>();
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
                              const std::shared_ptr<List>& list,
                              std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
          "Expected a lambda in `" + SpecializedBuiltins.Select + "` builtin.");
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
        throw BuiltinUnexpectedArgumentError(current(stream),
                                             SpecializedBuiltins.Select);
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
                              const std::string& input,
                              std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(stream);  // Skip "("

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(stream);

    return interpolateString(stream, frame, input);
  }

  Value interpretObjectToHash(std::shared_ptr<TokenStream> stream,
                              const std::shared_ptr<Object>& object) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(stream);  // Skip "("

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(stream);

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
                                    const std::string& builtin,
                                    const Value& value,
                                    std::shared_ptr<CallStackFrame> frame) {
    if (!std::holds_alternative<std::string>(value)) {
      throw InvalidOperationError(
          current(stream), "Specialized builtin `" + builtin +
                               "` is illegal for type `" +
                               Serializer::get_value_type_string(value) + "`.");
    }

    auto input = std::get<std::string>(value);

    if (builtin == SpecializedBuiltins.ToH) {
      return interpretStringToHash(stream, input, frame);
    }

    throw UnknownBuiltinError(current(stream), builtin);
  }

  Value interpretSpecializedObjectBuiltin(std::shared_ptr<TokenStream> stream,
                                          const std::string& builtin,
                                          const Value& value) {
    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      throw InvalidOperationError(
          current(stream), "Specialized object builtin `" + builtin +
                               "` is illegal for type `" +
                               Serializer::get_value_type_string(value) + "`.");
    }

    auto object = std::get<std::shared_ptr<Object>>(value);

    if (builtin == SpecializedBuiltins.ToH) {
      return interpretObjectToHash(stream, object);
    }

    throw UnknownBuiltinError(current(stream), builtin);
  }

  Value interpretSpecializedListBuiltin(std::shared_ptr<TokenStream> stream,
                                        const std::string& builtin,
                                        const Value& value,
                                        std::shared_ptr<CallStackFrame> frame) {
    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(
          current(stream), "Specialized list builtin `" + builtin +
                               "` is illegal for type `" +
                               Serializer::get_value_type_string(value) + "`.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);

    if (builtin == SpecializedBuiltins.Select) {
      return interpretLambdaSelect(stream, list, frame);
    } else if (builtin == SpecializedBuiltins.Map) {
      return interpretLambdaMap(stream, list, frame);
    } else if (builtin == SpecializedBuiltins.Reduce) {
      return interpretLambdaReduce(stream, list, frame);
    }

    throw UnknownBuiltinError(current(stream), builtin);
  }

  Value interpretDotNotation(std::shared_ptr<TokenStream> stream, Value& value,
                             std::shared_ptr<CallStackFrame> frame) {
    if (peek(stream).getType() == TokenType::DOT) {
      next(stream);
    }
    if (current(stream).getType() == TokenType::DOT) {
      next(stream);
    }

    auto tokenTerm = current(stream);
    auto op = tokenTerm.getText();
    next(stream);

    bool isObject = false;

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      auto object = std::get<std::shared_ptr<Object>>(value);
      auto clazz = classes[object->className];
      isObject = true;

      if (object->instanceVariables.find(op) !=
          object->instanceVariables.end()) {
        if (clazz.hasPrivateVariable(op)) {
          throw InvalidContextError(tokenTerm,
                                    "Cannot access private instance variable "
                                    "outside of object context.");
        }

        return object->instanceVariables[op];
      }
    }

    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, to invoke builtin or "
                        "method using dot-notation.");
    }

    if (SpecializedBuiltins.is_builtin(op)) {
      if (op == SpecializedBuiltins.ToH) {
        if (isObject) {
          return interpretSpecializedObjectBuiltin(stream, op, value);
        } else {
          return interpretSpecializedBuiltin(stream, op, value, frame);
        }
      } else {
        return interpretSpecializedListBuiltin(stream, op, value, frame);
      }
    }

    auto args = interpretArguments(stream, frame);

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      auto object = std::get<std::shared_ptr<Object>>(value);
      return interpretInstanceMethodInvocation(stream, object, op, args, frame);
    }

    return BuiltinInterpreter::execute(current(stream), op, value, args);
  }

  Value parseExpression(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    Value value = parseLogicalOr(stream, frame);

    if (current(stream).getType() == TokenType::QUESTION) {
      next(stream);  // Skip the '?'

      auto trueBranch = parseExpression(stream, frame);

      if (current(stream).getType() != TokenType::COLON) {
        throw SyntaxError(current(stream),
                          "Expected ':' in ternary operation.");
      }
      next(stream);  // Skip the ':'

      auto falseBranch =
          parseExpression(stream, frame);  // Parse the false branch

      if (!std::holds_alternative<bool>(value)) {
        throw ConversionError(
            current(stream), "Ternary condition must be a boolean expression.");
      }

      return std::get<bool>(value) ? trueBranch : falseBranch;
    }

    return value;
  }

  Value parseLogicalOr(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    auto left = parseLogicalAnd(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           token.getText() == Operators.Or) {
      next(stream);  // Skip "||"
      auto right = parseLogicalAnd(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs || rhs;
        token = current(stream);
      } else {
        throw ConversionError(current(stream),
                              "Expected a `Boolean` expression.");
      }
    }
    return left;
  }

  Value parseLogicalAnd(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseOr(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           token.getText() == Operators.And) {
      next(stream);  // Skip "&&"
      auto right = parseBitwiseOr(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs && rhs;
        token = current(stream);
      } else {
        throw ConversionError(current(stream),
                              "Expected a `Boolean` expression.");
      }
    }
    return left;
  }

  Value parseBitwiseOr(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseXor(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           token.getText() == Operators.BitwiseOr) {
      next(stream);
      auto right = parseBitwiseXor(stream, frame);
      left = std::visit(BitwiseOrVisitor(token), left, right);
      token = current(stream);
    }

    return left;
  }

  Value parseBitwiseXor(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseAnd(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           token.getText() == Operators.BitwiseXor) {
      next(stream);
      auto right = parseBitwiseAnd(stream, frame);
      left = std::visit(BitwiseXorVisitor(token), left, right);
      token = current(stream);
    }

    return left;
  }

  Value parseBitwiseAnd(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseEquality(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           token.getText() == Operators.BitwiseAnd) {
      next(stream);  // Skip "&"
      auto right = parseEquality(stream, frame);
      left = std::visit(BitwiseAndVisitor(token), left, right);
      token = current(stream);
    }

    return left;
  }

  Value parseEquality(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseComparison(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.Equal ||
            token.getText() == Operators.NotEqual)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseComparison(stream, frame);

      if (op == Operators.Equal) {
        left = std::visit(EqualityVisitor(token), left, right);
      } else if (op == Operators.NotEqual) {
        left = std::visit(InequalityVisitor(token), left, right);
      }

      token = current(stream);
    }
    return left;
  }

  Value parseComparison(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitshift(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.GreaterThan ||
            token.getText() == Operators.GreaterThanOrEqual ||
            token.getText() == Operators.LessThan ||
            token.getText() == Operators.LessThanOrEqual)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseBitshift(stream, frame);

      if (op == Operators.LessThan) {
        left = std::visit(LessThanVisitor(token), left, right);
      } else if (op == Operators.LessThanOrEqual) {
        left = std::visit(LessThanOrEqualVisitor(token), left, right);
      } else if (op == Operators.GreaterThan) {
        left = std::visit(GreaterThanVisitor(token), left, right);
      } else if (op == Operators.GreaterThanOrEqual) {
        left = std::visit(GreaterThanOrEqualVisitor(token), left, right);
      }

      token = current(stream);
    }

    return left;
  }

  Value parseBitshift(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseAddition(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.BitwiseLeftShift ||
            token.getText() == Operators.BitwiseRightShift)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseAddition(stream, frame);

      if (op == Operators.BitwiseLeftShift) {
        left = std::visit(BitwiseLeftShiftVisitor(token), left, right);
      } else if (op == Operators.BitwiseRightShift) {
        left = std::visit(BitwiseRightShiftVisitor(token), left, right);
      }

      token = current(stream);
    }

    return left;
  }

  Value parseAddition(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseMultiplication(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.Add ||
            token.getText() == Operators.Subtract)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseMultiplication(stream, frame);

      if (op == Operators.Add) {
        left = std::visit(AddVisitor(token), left, right);
      } else if (op == Operators.Subtract) {
        left = std::visit(SubtractVisitor(token), left, right);
      }

      token = current(stream);
    }

    return left;
  }

  Value parseMultiplication(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto left = parseUnary(stream, frame);
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.Multiply ||
            token.getText() == Operators.Divide ||
            token.getText() == Operators.Modulus ||
            token.getText() == Operators.Exponent)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseUnary(stream, frame);

      if (op == Operators.Multiply) {
        left = std::visit(MultiplyVisitor(token), left, right);
      } else if (op == Operators.Divide) {
        left = std::visit(DivideVisitor(token), left, right);
      } else if (op == Operators.Modulus) {
        left = std::visit(ModuloVisitor(token), left, right);
      } else if (op == Operators.Exponent) {
        left = std::visit(PowerVisitor(token), left, right);
      }

      token = current(stream);
    }

    return left;
  }

  Value parseUnary(std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CallStackFrame> frame) {
    auto token = current(stream);

    while (token.getType() == TokenType::OPERATOR &&
           (token.getText() == Operators.Not ||
            token.getText() == Operators.Subtract ||
            token.getText() == Operators.BitwiseNot)) {
      auto op = token.getText();
      next(stream);  // Skip operator
      auto right = parseUnary(stream, frame);

      if (op == Operators.Not) {
        if (std::holds_alternative<bool>(right)) {
          bool rhs = std::get<bool>(right);
          return !rhs;
        } else if (std::holds_alternative<long long>(right)) {
          int rhs = std::get<long long>(right);
          if (rhs == 0) {
            return 1;
          } else if (rhs == 1) {
            return 0;
          }
        }

        throw ConversionError(current(stream),
                              "Expected a `Boolean` expression.");
      } else if (op == Operators.Subtract) {
        if (std::holds_alternative<long long>(right)) {
          return -std::get<long long>(right);
        } else if (std::holds_alternative<double>(right)) {
          return -std::get<double>(right);
        } else {
          throw ConversionError(current(stream),
                                "Unary minus applied to a non-numeric value.");
        }
      } else if (op == Operators.BitwiseNot) {
        return std::visit(BitwiseNotVisitor(current(stream)), right);
      }

      token = current(stream);
    }

    return parsePrimary(stream, frame);
  }

  Value parsePrimary(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    // Implement parsing of numbers, parentheses, identifiers, etc.
    if (current(stream).getType() == TokenType::DECLVAR) {
      next(stream);
    }

    if (current(stream).getType() == TokenType::OPEN_BRACE) {
      return interpretHash(stream, frame);
    }

    if (current(stream).getType() == TokenType::IDENTIFIER &&
        peek(stream).getType() == TokenType::OPEN_BRACKET) {
      auto variableName = current(stream).getText();
      next(stream);  // Skip the identifier.
      return interpretSlice(stream, frame, variableName);
    }

    if (current(stream).getType() == TokenType::OPEN_PAREN) {
      next(stream);  // Skip "("
      auto result = interpretExpression(stream, frame);

      if (current(stream).getType() == TokenType::CLOSE_PAREN) {
        next(stream);  // Skip ")"
      }

      return result;
    } else if (current(stream).getType() == TokenType::OPEN_BRACKET) {
      auto result = interpretBracketExpression(stream, frame);
      return result;
    } else if (current(stream).getType() == TokenType::IDENTIFIER) {
      return interpretIdentifier(stream, frame);
    } else if (current(stream).getType() == TokenType::KEYWORD &&
               current(stream).getText() == Keywords.This) {
      return interpretSelfInvocationTerm(stream, frame);
    } else if (current(stream).getValueType() == ValueType::Boolean) {
      auto value = current(stream).toBoolean();
      next(stream);
      return value;
    } else if (current(stream).getValueType() == ValueType::Double) {
      auto value = current(stream).toDouble();
      next(stream);
      return value;
    } else if (current(stream).getValueType() == ValueType::Integer) {
      auto value = current(stream).toInteger();
      next(stream);
      return value;
    } else if (current(stream).getValueType() == ValueType::String) {
      auto value = interpolateString(stream, frame);
      next(stream);
      return value;
    }

    return 0;  // Default value.
  }

  Value interpretExpression(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto result = parseExpression(stream, frame);

    while (current(stream).getType() == TokenType::DOT || current(stream).getType() == TokenType::OPEN_BRACKET) {
      if (current(stream).getType() == TokenType::DOT) {
        result = interpretDotNotation(stream, result, frame);
      } else if (current(stream).getType() == TokenType::OPEN_BRACKET) {
        if (!InterpHelper::isSliceAssignmentExpression(stream)) {
          throw SyntaxError(current(stream),
                            "Invalid slice-assignment expression.");
        }

        auto slice = interpretSliceIndex(stream, frame, result);
        auto list = std::get<std::shared_ptr<List>>(result);

        result = InterpHelper::interpretListSlice(stream, slice, list);
      }
    }

    return result;
  }

  Value interpretSimpleValueType(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getValueType() == ValueType::Boolean) {
      return current(stream).toBoolean();
    } else if (current(stream).getValueType() == ValueType::Double) {
      return current(stream).toDouble();
    } else if (current(stream).getValueType() == ValueType::Integer) {
      return current(stream).toInteger();
    } else if (current(stream).getValueType() == ValueType::String) {
      return interpolateString(stream, frame);
    }

    throw ConversionError(current(stream));
  }

  Value interpretSelfInvocationTerm(std::shared_ptr<TokenStream> stream,
                                    std::shared_ptr<CallStackFrame> frame) {
    if (!frame->inObjectContext()) {
      throw InvalidContextError(current(stream),
                                "Invalid context for keyword `this`.");
    }

    if (peek(stream).getType() == TokenType::DOT) {
      next(stream);
    }

    if (current(stream).getType() != TokenType::DOT) {
      return frame->getObjectContext();
    }
    next(stream);  // Skip the "."

    if (current(stream).getType() == TokenType::DECLVAR) {
      next(stream);  // Skip the "@"
    }

    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw InvalidOperationError(
          current(stream), "Syntax error near `this`. Missing identifier.");
    }
    auto identifier = current(stream).getText();
    next(stream);  // Skip the identifier

    auto clazz = classes[frame->getObjectContext()->className];

    if (clazz.hasMethod(identifier)) {
      interpretInstanceMethodInvocation(
          stream, frame->getObjectContext()->identifier, frame);

      if (!callStack.empty()) {
        return callStack.top()->returnValue;
      }
    } else if (frame->getObjectContext()->instanceVariables.find(identifier) !=
               frame->getObjectContext()->instanceVariables.end()) {
      return frame->getObjectContext()->instanceVariables[identifier];
    }

    throw UnimplementedMethodError(current(stream), clazz.getClassName(),
                                   identifier);
  }

  std::string interpolateObject(std::shared_ptr<TokenStream> stream,
                                Value& value,
                                std::shared_ptr<CallStackFrame> frame) {
    auto object = std::get<std::shared_ptr<Object>>(value);
    auto clazz = classes[object->className];

    if (!clazz.hasMethod(KiwiBuiltins.ToS)) {
      return Serializer::basic_serialize_object(object);
    }

    auto toString = clazz.getMethod(KiwiBuiltins.ToS);
    std::vector<Value> parameters;
    Value returnValue = interpretInstanceMethodInvocation(
        stream, object, KiwiBuiltins.ToS, parameters, frame);

    // Should probably check that an overridden to_s() actually returns a string.
    return Serializer::serialize(returnValue);
  }

  Value interpolateString(std::shared_ptr<TokenStream> stream,
                          std::shared_ptr<CallStackFrame> frame,
                          const std::string& input) {
    if (input[0] == '@') {
      std::string name = input.substr(1);
      if (hasVariable(name, frame)) {
        return getVariable(stream, name, frame);
      }
    }

    Token tempToken = Token::createEmpty();
    auto tempId = InterpHelper::getTemporaryId();
    auto tempAssignment =
        InterpHelper::getTemporaryAssignment(tempToken, tempId);
    Lexer lexer("", input);
    for (Token t : lexer.getAllTokens()) {
      tempAssignment.push_back(t);
    }

    auto tempStream = std::make_shared<TokenStream>(tempAssignment);
    auto tempFrame = buildSubFrame(frame);
    interpretAssignment(tempStream, tempFrame, true);

    if (tempFrame->variables.find(tempId) == tempFrame->variables.end()) {
      throw SyntaxError(current(stream),
                        "Invalid string interpolation: `" + input + "`");
    }

    Value value = getVariable(stream, tempId, tempFrame);

    tempAssignment.clear();

    return value;
  }

  std::string interpolateString(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    std::ostringstream sv;
    std::string input = current(stream).getText();
    std::string builder;
    int i = 0, size = input.length();
    char c = '\0', next = '\0';
    bool canPeek = false;
    int interpCount = 0;

    while (i < size) {
      canPeek = i + 1 < size;
      c = input[i];
      next = canPeek ? input[i + 1] : '\0';

      switch (c) {
        case '$':
          if (canPeek && next == '{') {
            ++interpCount;
            i += 2;  // Skip "${"
            continue;
          }
          break;

        case '}':
          if (interpCount > 0) {
            --interpCount;

            if (interpCount > 0) {
              throw SyntaxError(
                  current(stream),
                  "Invalid syntax in string interpolation: `" + input + "`");
            }

            if (!builder.empty()) {
              Value interpolatedValue =
                  interpolateString(stream, frame, builder);
              if (!std::holds_alternative<std::shared_ptr<Object>>(
                      interpolatedValue)) {
                sv << Serializer::serialize(interpolatedValue);
              } else {
                sv << interpolateObject(stream, interpolatedValue, frame);
              }
              builder.clear();
              ++i;
            }

            continue;
          }
          break;

        case '\\':
          if (canPeek) {
            if (next == 't') {
              sv << "\t";
              i += 2;
              continue;
            } else if (next == 'n') {
              sv << "\n";
              i += 2;
              continue;
            }
          }
          break;
      }

      if (interpCount == 0) {
        sv << c;
      } else {
        builder += c;
      }

      ++i;
    }

    std::string output = sv.str();

    return output;
  }

  void interpretLambdaAssignment(std::shared_ptr<TokenStream> stream,
                                 const std::string& name, std::string op,
                                 std::shared_ptr<CallStackFrame> frame) {
    if (op != Operators.Assign) {
      throw InvalidOperationError(
          current(stream),
          "Expected assignment operator in lambda assignment.");
    }

    Method lambda = interpretLambda(stream, frame);
    lambda.setName(name);
    frame->assignLambda(name, lambda);
  }

  void interpretAssignment(std::shared_ptr<TokenStream> stream,
                           std::string& name, std::string& op,
                           std::shared_ptr<CallStackFrame> frame,
                           bool isTemporary = false,
                           bool isInstanceVariable = false) {
    if (current(stream).getType() == TokenType::LAMBDA) {
      interpretLambdaAssignment(stream, name, op, frame);
      return;
    }

    Value value = interpretExpression(stream, frame);

    if (op == Operators.Assign) {
      if (!isTemporary && isInstanceVariable && frame->inObjectContext()) {
        frame->getObjectContext()->instanceVariables[name] = value;
      } else {
        if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
          std::shared_ptr<Object> object =
              std::get<std::shared_ptr<Object>>(value);
          object->identifier = name;
          value = object;
        }
        frame->variables[name] = value;
      }
      Token nextToken = peek(stream);
      if (nextToken.getType() == TokenType::CLOSE_PAREN ||
          nextToken.getType() == TokenType::CLOSE_BRACKET) {
        next(stream);
      }
      return;
    }

    if (!hasVariable(name, frame)) {
      throw VariableUndefinedError(current(stream), name);
    }

    if (op == Operators.ListAppend) {
      interpretAppendToList(stream, frame, value, name);
      return;
    }

    Value currentValue = getVariable(stream, name, frame);
    frame->variables[name] =
        InterpHelper::interpretAssignOp(stream, op, currentValue, value);
  }
};

#endif
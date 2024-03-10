#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <unordered_map>
#include <stack>
#include "errors/error.h"
#include "errors/handler.h"
#include "errors/state.h"
#include "k_int.h"
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
#include "parsing/tokentype.h"
#include "parsing/keywords.h"
#include "util/file.h"
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
      try {
        interpretToken(stream, frame);
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
    Method lambda;

    if (current(stream).getType() == TokenType::IDENTIFIER) {
      auto lambdaName = current(stream).getText();
      next(stream);  // Skip identifier.
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
    } else if (current(stream).getType() == TokenType::LAMBDA) {
      return interpretLambda(stream, frame);
    }

    throw InvalidOperationError(current(stream), "Expected lambda.");
  }

  Method interpretLambda(std::shared_ptr<TokenStream> stream,
                         std::shared_ptr<CallStackFrame> frame) {
    next(stream);  // Skip "lambda"
    Method lambda;
    lambda.setName(InterpHelper::getTemporaryId());

    interpretMethodParameters(stream, frame, lambda);

    if (current(stream).getSubType() != SubTokenType::KW_Do) {
      throw SyntaxError(current(stream), "Expected `do` in lambda expression.");
    }
    next(stream);  // Skip "do"

    std::vector<Token> lambdaTokens;
    InterpHelper::collectBodyTokens(lambdaTokens, stream);
    for (const auto& t : lambdaTokens) {
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
                         std::shared_ptr<CallStackFrame> frame,
                         Value& collectionValue, const bool& hasIndexVariable,
                         const std::string& itemVariableName,
                         const std::string& indexVariableName) {
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
    if (current(stream).getType() == TokenType::IDENTIFIER) {
      itemVariableName = current(stream).getText();
      next(stream);  // Skip the item variable name

      if (current(stream).getType() == TokenType::COMMA) {
        next(stream);  // Skip ','
        if (current(stream).getType() == TokenType::IDENTIFIER) {
          hasIndexVariable = true;
          indexVariableName =
              current(stream).getText();  // Get the index variable name
          next(stream);                   // Skip the index variable name
        }
      }
    }

    if (current(stream).getSubType() != SubTokenType::KW_In) {
      throw SyntaxError(current(stream), "Expected 'in' after loop variables.");
    }

    next(stream);  // Skip "in"

    auto collectionValue = interpretExpression(stream, frame);

    if (peek(stream).getSubType() != SubTokenType::KW_Do &&
        current(stream).getSubType() != SubTokenType::KW_Do) {
      throw SyntaxError(current(stream), "Expected `do` in `for` loop.");
    }

    if (peek(stream).getSubType() == SubTokenType::KW_Do) {
      next(stream);
    }

    if (current(stream).getSubType() == SubTokenType::KW_Do) {
      next(stream);
    }

    if (std::holds_alternative<std::shared_ptr<List>>(collectionValue)) {
      interpretListLoop(stream, frame, collectionValue, hasIndexVariable,
                        itemVariableName, indexVariableName);
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(collectionValue)) {
      interpretHashLoop(stream, frame, collectionValue, hasIndexVariable,
                        indexVariableName, itemVariableName);
    } else {
      throw InvalidOperationError(current(stream),
                                  "Term is not a List or Hash.");
    }

    frame->clearFlag(FrameFlags::LoopBreak);
    frame->clearFlag(FrameFlags::LoopContinue);
  }

  void interpretWhileLoop(std::shared_ptr<TokenStream> stream,
                          std::shared_ptr<CallStackFrame> frame) {
    std::vector<Token> conditionTokens;
    while (stream->canRead() &&
           current(stream).getSubType() != SubTokenType::KW_Do) {
      conditionTokens.push_back(current(stream));
      next(stream);
    }

    next(stream);  // Skip "do"

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, stream);

    auto oldFrame = std::make_shared<CallStackFrame>(*frame);
    auto tempId = InterpHelper::getTemporaryId();
    auto tempAssignment =
        InterpHelper::getTemporaryAssignment(current(stream), tempId);
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
      interpretAssignment(conditionStream, conditionFrame, "", true);

      if (conditionFrame->variables.find(tempId) ==
          conditionFrame->variables.end()) {
        throw SyntaxError(current(stream), "Invalid condition in while-loop.");
      }

      auto value = getVariable(conditionStream, conditionFrame, tempId);
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
    const auto& loop = current(stream).getSubType();
    next(stream);  // Skip "while"|"for"

    if (loop == SubTokenType::KW_While) {
      interpretWhileLoop(stream, frame);
    } else if (loop == SubTokenType::KW_For) {
      interpretForLoop(stream, frame);
    }
  }

  void interpretKiwiKeyword(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame,
                            const SubTokenType& keyword) {
    if (keyword == SubTokenType::KW_If) {
      interpretConditional(stream, frame);
    } else if (Keywords.is_loop_keyword(keyword)) {
      interpretLoop(stream, frame);
    } else if (keyword == SubTokenType::KW_Method) {
      interpretMethodDefinition(stream, frame);
    } else if (keyword == SubTokenType::KW_Return) {
      interpretReturn(stream, frame);
    } else if (keyword == SubTokenType::KW_Exit) {
      interpretExit(stream, frame);
    } else if (keyword == SubTokenType::KW_PrintLn ||
               keyword == SubTokenType::KW_Print) {
      interpretPrint(stream, frame, keyword == SubTokenType::KW_PrintLn);
    } else if (keyword == SubTokenType::KW_Import) {
      interpretImport(stream, frame);
    } else if (keyword == SubTokenType::KW_Export) {
      interpretExport(stream, frame);
    } else if (keyword == SubTokenType::KW_Module) {
      interpretModuleDefinition(stream);
    } else if (keyword == SubTokenType::KW_Delete) {
      interpretDelete(stream, frame);
    } else if (keyword == SubTokenType::KW_Abstract ||
               keyword == SubTokenType::KW_Class) {
      interpretClassDefinition(stream, frame);
    } else if (keyword == SubTokenType::KW_This) {
      interpretSelfInvocation(stream, frame);
    } else if (keyword == SubTokenType::KW_Break) {
      next(stream);  // Skip "break"
      frame->setFlag(FrameFlags::LoopBreak);
    } else if (keyword == SubTokenType::KW_Next) {
      next(stream);  // Skip "next"
      frame->setFlag(FrameFlags::LoopContinue);
    } else if (keyword == SubTokenType::KW_Try) {
      next(stream);  // Skip "try"
      frame->setFlag(FrameFlags::InTry);
    } else if (keyword == SubTokenType::KW_Pass) {
      next(stream);  // Skip "pass"
    } else if (keyword == SubTokenType::KW_Catch) {
      interpretCatch(stream, frame);
    } else {
      throw UnrecognizedTokenError(
          current(stream),
          "Unrecognized token `" + current(stream).getText() + "`.");
    }
  }

  void interpretKeyword(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    const auto& keyword = current(stream).getSubType();

    interpretKiwiKeyword(stream, frame, keyword);
  }

  void interpretQualifiedIdentifier(std::shared_ptr<TokenStream> stream,
                                    std::string& identifier) {
    while (peek(stream).getType() == TokenType::QUALIFIER) {
      next(stream);  // Skip the identifier.
      next(stream);  // Skip the qualifier.
      identifier += Symbols.Qualifier + current(stream).getText();
    }
  }

  Value interpretIdentifierInvocation(std::shared_ptr<TokenStream> stream,
                                      std::shared_ptr<CallStackFrame> frame,
                                      const std::string& identifier) {
    bool methodFound = hasMethod(frame, identifier),
         classFound = hasClass(identifier);

    if (methodFound || classFound) {
      if (classFound) {
        if (peek(stream).getType() != TokenType::DOT) {
          throw SyntaxError(current(stream),
                            "Invalid syntax near `" + identifier + "`");
        }
        interpretClassMethodInvocation(stream, frame, identifier);
      } else if (methodFound) {
        if (peek(stream).getType() == TokenType::COMMA ||
            peek(stream).getType() == TokenType::CLOSE_PAREN) {
          if (frame->hasAssignedLambda(identifier)) {
            std::shared_ptr<LambdaRef> lambdaRef =
                std::make_shared<LambdaRef>(identifier);
            return lambdaRef;
          } else {
            throw SyntaxError(current(stream), "Expected lambda reference.");
          }
        } else {
          interpretMethodInvocation(stream, frame, identifier);
        }
      }

      if (!callStack.empty()) {
        auto value = callStack.top()->returnValue;
        frame->clearFlag(FrameFlags::ReturnFlag);
        return value;
      }
    }

    throw UnknownIdentifierError(current(stream), identifier);
  }

  Value interpretValueInvocation(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 Value& v) {
    while (current(stream).getType() == TokenType::DOT ||
           current(stream).getType() == TokenType::OPEN_BRACKET) {
      if (current(stream).getType() == TokenType::DOT) {
        v = interpretDotNotation(stream, frame, v);
      } else if (current(stream).getType() == TokenType::OPEN_BRACKET) {
        if (!InterpHelper::isSliceAssignmentExpression(stream)) {
          throw SyntaxError(current(stream),
                            "Invalid slice-assignment expression.");
        }

        auto slice = interpretSliceIndex(stream, frame, v);
        auto list = std::get<std::shared_ptr<List>>(v);

        v = InterpHelper::interpretListSlice(stream, slice, list);
      }
    }
    return v;
  }

  Value interpretIdentifier(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame,
                            bool doAssignment = true) {
    auto tokenText = current(stream).getText();
    auto op = current(stream).getSubType();

    interpretQualifiedIdentifier(stream, tokenText);

    if (doAssignment && peek(stream).getType() == TokenType::OPERATOR &&
        (Operators.is_assignment_operator(peek(stream).getSubType()) ||
         peek(stream).getSubType() == SubTokenType::Ops_BitwiseLeftShift)) {
      interpretAssignment(stream, frame, tokenText);
      return 0;
    }

    if (hasVariable(frame, tokenText)) {
      if (peek(stream).getType() == TokenType::OPEN_BRACKET) {
        next(stream);
        interpretSliceAssignment(stream, frame, tokenText);
        return 0;
      }

      auto v = getVariable(stream, frame, tokenText);
      next(stream);

      if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
        auto lambdaRef = std::get<std::shared_ptr<LambdaRef>>(v)->identifier;
        if (frame->hasAssignedLambda(lambdaRef)) {
          return interpretMethodInvocation(stream, frame, lambdaRef);
        } else {
          throw InvalidOperationError(current(stream),
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

    throw UnknownIdentifierError(current(stream), tokenText);
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
      if (current(stream).getSubType() == SubTokenType::KW_End) {
        --count;

        // Stop here.
        if (count == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getSubType())) {
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

  void interpretToken(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    if (frame->isErrorStateSet()) {
      if (current(stream).getType() == TokenType::KEYWORD &&
          current(stream).getSubType() == SubTokenType::KW_Catch) {
        interpretCatch(stream, frame);
      }
      return;
    }

    switch (current(stream).getType()) {
      case TokenType::COMMENT:
      case TokenType::COMMA:
        next(stream);  // Skip these.
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
            current(stream),
            "Unrecognized token `" + current(stream).getText() + "`.");
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
                   const std::string& name) {
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

    throw MethodUndefinedError(current(stream), name);
  }

  Value getVariable(std::shared_ptr<TokenStream> stream,
                    std::shared_ptr<CallStackFrame> frame,
                    const std::string& name) {
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
    auto tempStack(callStack);
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

      Value argValue;

      if (current(stream).getType() == TokenType::TYPENAME) {
        argValue = current(stream).getText();
      } else if (hasClass(current(stream).getText())) {
        argValue = current(stream).getText();
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
                              const SubTokenType& builtin,
                              std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Module_Home) {
      if (args.size() != 1) {
        throw BuiltinUnexpectedArgumentError(current(stream),
                                             ModuleBuiltins.Home);
      }

      if (!std::holds_alternative<std::string>(args.at(0))) {
        throw SyntaxError(current(stream), "Expected string value for `" +
                                               ModuleBuiltins.Home +
                                               "` builtin parameter.");
      }
      std::string value = std::get<std::string>(args.at(0));

      modules[moduleName].setHome(value);
      modules[moduleName].setName(moduleName);
    }
  }

  std::vector<std::string> collectMethodParameters(
      std::shared_ptr<TokenStream> stream,
      std::shared_ptr<CallStackFrame> frame, Method& method) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          current(stream),
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
          throw ParameterMissingError(current(stream), parameterName);
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
    next(stream);  // Skip class name
    next(stream);  // Skip the "."

    auto methodName = current(stream).getText();
    next(stream);  // Skip the method name.

    auto clazz = classes[className];
    auto context = std::make_shared<Object>();
    bool isInstantiation = false;

    if (methodName == Keywords.New) {
      if (clazz.isAbstract()) {
        throw InvalidOperationError(current(stream),
                                    "Cannot instantiate an abstract class.");
      }

      if (!clazz.hasMethod(Keywords.Ctor)) {
        throw UnimplementedMethodError(current(stream), className,
                                       Keywords.Ctor);
      }

      methodName = Keywords.Ctor;
      isInstantiation = true;
    } else if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(current(stream), className, methodName);
    }

    auto method = clazz.getMethod(methodName);
    if (!method.isFlagSet(MethodFlags::Static) &&
        !method.isFlagSet(MethodFlags::Ctor)) {
      throw InvalidContextError(
          current(stream),
          "The method `" + methodName +
              "` can only be invoked on an instance of class `" + className +
              "`.");
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
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
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

  Value interpretInstanceMethodInvocation(std::shared_ptr<TokenStream> stream,
                                          std::shared_ptr<CallStackFrame> frame,
                                          std::shared_ptr<Object>& object,
                                          const std::string& methodName,
                                          const SubTokenType& op,
                                          std::vector<Value>& parameters) {
    if (!hasClass(object->className)) {
      throw ClassUndefinedError(current(stream), object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      if (KiwiBuiltins.is_builtin(op)) {
        return BuiltinInterpreter::execute(current(stream), op, object,
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

  void interpretInstanceMethodInvocation(std::shared_ptr<TokenStream> stream,
                                         std::shared_ptr<CallStackFrame> frame,
                                         const std::string& instanceName) {
    next(stream);  // Skip the "."
    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          current(stream),
          "Expected identifier in instance method invocation, instead got: `" +
              current(stream).getText() + "`");
    }

    auto methodName = current(stream).getText();
    next(stream);  // Skip the method name.

    auto instanceValue = getVariable(stream, frame, instanceName);
    auto object = std::get<std::shared_ptr<Object>>(instanceValue);

    if (!hasClass(object->className)) {
      throw ClassUndefinedError(current(stream), object->className);
    }

    auto clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
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
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
    codeFrame->setObjectContext(object);
    callStack.push(codeFrame);
    streamStack.push(codeStream);

    interpretStackFrame();
  }

  Value interpretMethodInvocation(std::shared_ptr<TokenStream> stream,
                                  std::shared_ptr<CallStackFrame> frame,
                                  const std::string& name) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      next(stream);  // Skip the name.
    }

    auto method = getMethod(stream, frame, name);
    auto codeStream = std::make_shared<TokenStream>(method.getCode());
    auto codeFrame = buildMethodInvocationStackFrame(stream, frame, method);
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
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          current(stream),
          "Expected open-parenthesis, `(`, in parameter set expression.");
    }
    next(stream);  // Skip "("

    std::unordered_set<std::string> paramNames;
    std::vector<Parameter> params;

    while (stream->canRead() &&
           current(stream).getType() != TokenType::CLOSE_PAREN) {
      auto paramName = current(stream).getText();
      if (current(stream).getType() == TokenType::IDENTIFIER) {
        if (paramNames.find(paramName) != paramNames.end()) {
          throw SyntaxError(current(stream),
                            "The parameter `" + paramName +
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
          current(stream),
          "Expected close-parenthesis, `)`, in parameter set expression.");
    }
    next(stream);  // Skip ")"

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
           current(stream).getSubType() != SubTokenType::KW_Method) {
      if (current(stream).getSubType() == SubTokenType::KW_Abstract) {
        method.setFlag(MethodFlags::Abstract);
      } else if (current(stream).getSubType() == SubTokenType::KW_Override) {
        method.setFlag(MethodFlags::Override);
      } else if (current(stream).getSubType() == SubTokenType::KW_Private) {
        method.setFlag(MethodFlags::Private);
      } else if (current(stream).getSubType() == SubTokenType::KW_Static) {
        method.setFlag(MethodFlags::Static);
      }
      next(stream);
    }
    next(stream);  // Skip "def"

    auto name = current(stream).getText();
    method.setName(name);
    next(stream);  // Skip the name.
    interpretMethodParameters(stream, frame, method);
    int counter = 1;

    if (method.isFlagSet(MethodFlags::Abstract)) {
      return method;
    }

    while (stream->canRead() && counter > 0) {
      if (current(stream).getSubType() == SubTokenType::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getSubType())) {
        ++counter;
      }

      method.addToken(current(stream));
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
      throw IllegalNameError(current(stream), name);
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
    next(stream);  // Skip "return"

    Value returnValue;

    if (hasValue) {
      returnValue = interpretExpression(stream, frame);
    }

    frame->returnValue = returnValue;
    frame->setFlag(FrameFlags::ReturnFlag);
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
      throw InvalidOperationError(
          current(stream), "Expected a list type for list access operation.");
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

    if (!std::holds_alternative<k_int>(output)) {
      throw SyntaxError(current(stream),
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
      throw HashKeyError(current(stream), key);
    }

    return hash->get(key);
  }

  Value interpretSlice(std::shared_ptr<TokenStream> stream,
                       std::shared_ptr<CallStackFrame> frame,
                       const std::string& name) {
    auto value = getVariable(stream, frame, name);
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
    Value variableValue;
    try {
      variableValue = getVariable(stream, frame, listVariableName);
    } catch (const VariableUndefinedError& e) {
      throw VariableUndefinedError(current(stream), listVariableName);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(variableValue)) {
      throw InvalidOperationError(current(stream),
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

    if (!std::holds_alternative<k_int>(startValue)) {
      throw RangeError(current(stream),
                       "A range start value must be an integer.");
    }

    if (!std::holds_alternative<k_int>(stopValue)) {
      throw RangeError(current(stream),
                       "A range stop value must be an integer.");
    }

    int start = std::get<k_int>(startValue), stop = std::get<k_int>(stopValue);
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
    if (current(stream).getSubType() != SubTokenType::KW_If) {
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
    auto building = SubTokenType::KW_If;

    // Eagerly evaluate the If conditions.
    auto value = interpretExpression(stream, frame);

    if (!std::holds_alternative<bool>(value)) {
      throw ConversionError(current(stream));
    }

    shortCircuitIf = std::get<bool>(value);
    conditional.getIfStatement().setEvaluation(shortCircuitIf);

    while (stream->canRead() && ifCount > 0) {
      if (Keywords.is_block_keyword(current(stream).getSubType())) {
        ++ifCount;
      } else if (current(stream).getSubType() == SubTokenType::KW_End &&
                 ifCount > 0) {
        --ifCount;

        // Stop here.
        if (ifCount == 0) {
          next(stream);
          continue;
        }
      } else if (current(stream).getSubType() == SubTokenType::KW_Else &&
                 ifCount == 1) {
        next(stream);
        building = SubTokenType::KW_Else;
      } else if (current(stream).getSubType() == SubTokenType::KW_ElseIf &&
                 ifCount == 1) {
        next(stream);
        building = SubTokenType::KW_ElseIf;

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
      if (shortCircuitIf && building == SubTokenType::KW_If) {
        conditional.getIfStatement().addToken(current(stream));
      } else if (!shortCircuitIf && building == SubTokenType::KW_ElseIf) {
        conditional.getElseIfStatement().addToken(current(stream));
      } else if (!shortCircuitIf && !shortCircuitElseIf &&
                 building == SubTokenType::KW_Else) {
        conditional.getElseStatement().addToken(current(stream));
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

    if (current(stream).getType() == TokenType::IDENTIFIER) {
      if (peek(stream).getType() == TokenType::OPERATOR) {
        value = interpretAssignment(stream, frame, "", false, true);
      } else if (peek(stream).getType() == TokenType::OPEN_PAREN) {
        value =
            interpretMethodInvocation(stream, frame, current(stream).getText());
      }
    }

    return value;
  }

  void interpretClassDefinition(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    bool isAbstract = current(stream).getSubType() == SubTokenType::KW_Abstract;
    std::string moduleName;
    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    while (stream->canRead() &&
           current(stream).getSubType() != SubTokenType::KW_Class) {
      next(stream);
    }
    next(stream);  // Skip "class"

    auto className = current(stream).getText();
    next(stream);  // Skip class name.

    if (classes.find(className) != classes.end()) {
      throw ClassRedefinitionError(current(stream), className);
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
        throw ClassUndefinedError(current(stream), baseClassName);
      }

      // inherit methods from base class.
      auto baseClass = classes[baseClassName];
      for (auto& pair : baseClass.getMethods()) {
        clazz.addMethod(pair.second);
      }
    }

    int counter = 1;
    while (stream->canRead() && counter > 0) {
      auto st = current(stream).getSubType();
      if (st == SubTokenType::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (st == SubTokenType::KW_Abstract ||
                 st == SubTokenType::KW_Override ||
                 st == SubTokenType::KW_Method ||
                 st == SubTokenType::KW_Private ||
                 st == SubTokenType::KW_Static) {
        if (st == SubTokenType::KW_Private) {
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
      if (current(stream).getSubType() == SubTokenType::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);  // Skip "end"
          break;
        }
      } else if (Keywords.is_block_keyword(current(stream).getSubType())) {
        ++counter;
      }

      module.addToken(current(stream));
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

    if (current(stream).getSubType() == SubTokenType::KW_As) {
      interpretModuleAlias(stream, moduleName);
    }
  }

  Value interpretVariableValue(std::shared_ptr<TokenStream> stream,
                               std::string name,
                               std::shared_ptr<CallStackFrame> frame) {
    if (!hasVariable(frame, name)) {
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

    if (current(stream).getType() == TokenType::IDENTIFIER) {
      auto name = current(stream).getText();
      next(stream);

      if (!hasVariable(frame, name)) {
        throw VariableUndefinedError(current(stream), name);
      }

      auto value = getVariable(stream, frame, name);

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
      std::cout << interpolateObject(stream, frame, value);
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
                         std::shared_ptr<CallStackFrame> frame,
                         const SubTokenType& builtin) {
    next(stream);  // Skip the name.

    auto args = interpretArguments(stream, frame);

    if (ModuleBuiltins.is_builtin(builtin)) {
      if (moduleStack.empty()) {
        throw InvalidContextError(current(stream),
                                  "Expected a module context.");
      }
      auto moduleName = moduleStack.top();
      interpretModuleBuiltin(stream, moduleName, builtin, args);
      return 0;
    }

    frame->returnValue =
        BuiltinInterpreter::execute(current(stream), builtin, args, kiwiArgs);
    return frame->returnValue;
  }

  Value interpretLambdaNone(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame,
                            const std::shared_ptr<List>& list) {
    next(stream);  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
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
        throw BuiltinUnexpectedArgumentError(current(stream),
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
    next(stream);  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
          "Expected a lambda in `" + ListBuiltins.Map + "` builtin.");
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
        throw BuiltinUnexpectedArgumentError(current(stream), ListBuiltins.Map);
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
          "Expected a lambda in `" + ListBuiltins.Reduce + "` builtin.");
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
                                             ListBuiltins.Reduce);
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
                              std::shared_ptr<CallStackFrame> frame,
                              const std::shared_ptr<List>& list) {
    next(stream);  // Skip "("

    auto lambda = getLambda(stream, frame);

    if (current(stream).getType() == CLOSE_PAREN) {
      next(stream);  // Skip ")"
    }

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(stream),
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
        throw BuiltinUnexpectedArgumentError(current(stream),
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
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }
    next(stream);  // Skip "("

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }
    next(stream);

    return interpolateString(stream, frame, input);
  }

  Value interpretObjectToHash(std::shared_ptr<TokenStream> stream,
                              const std::shared_ptr<Object>& object) {
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            ListBuiltins.ToH + "`.");
    }
    next(stream);  // Skip "("

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(stream),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            ListBuiltins.ToH + "`.");
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
                                    std::shared_ptr<CallStackFrame> frame,
                                    const SubTokenType& builtin,
                                    const Value& value) {
    if (!std::holds_alternative<std::string>(value)) {
      throw InvalidOperationError(current(stream), "Expected type String.");
    }

    auto input = std::get<std::string>(value);

    if (builtin == SubTokenType::Builtin_List_ToH) {
      return interpretStringToHash(stream, frame, input);
    }

    throw UnknownBuiltinError(current(stream), "");
  }

  Value interpretSpecializedObjectBuiltin(std::shared_ptr<TokenStream> stream,
                                          const SubTokenType& builtin,
                                          const Value& value) {
    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      throw InvalidOperationError(current(stream), "Expected type Object.");
    }

    auto object = std::get<std::shared_ptr<Object>>(value);

    if (builtin == SubTokenType::Builtin_List_ToH) {
      return interpretObjectToHash(stream, object);
    }

    throw UnknownBuiltinError(current(stream), "");
  }

  Value interpretSpecializedListBuiltin(std::shared_ptr<TokenStream> stream,
                                        std::shared_ptr<CallStackFrame> frame,
                                        const SubTokenType& builtin,
                                        const Value& value) {
    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(current(stream), "Expected type List.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);

    if (builtin == SubTokenType::Builtin_List_Select) {
      return interpretLambdaSelect(stream, frame, list);
    } else if (builtin == SubTokenType::Builtin_List_Map) {
      return interpretLambdaMap(stream, frame, list);
    } else if (builtin == SubTokenType::Builtin_List_Reduce) {
      return interpretLambdaReduce(stream, frame, list);
    } else if (builtin == SubTokenType::Builtin_List_None) {
      return interpretLambdaNone(stream, frame, list);
    }

    throw UnknownBuiltinError(current(stream), "");
  }

  Value interpretDotNotation(std::shared_ptr<TokenStream> stream,
                             std::shared_ptr<CallStackFrame> frame,
                             Value& value) {
    if (peek(stream).getType() == TokenType::DOT) {
      next(stream);
    }
    if (current(stream).getType() == TokenType::DOT) {
      next(stream);
    }

    auto opText = current(stream).getText();
    auto op = current(stream).getSubType();
    next(stream);

    bool isObject = false;

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      auto object = std::get<std::shared_ptr<Object>>(value);
      auto clazz = classes[object->className];
      isObject = true;

      if (object->instanceVariables.find(opText) !=
          object->instanceVariables.end()) {
        if (clazz.hasPrivateVariable(opText)) {
          throw InvalidContextError(current(stream),
                                    "Cannot access private instance variable "
                                    "outside of object context.");
        }

        return object->instanceVariables[opText];
      }
    }

    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(stream),
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
      auto object = std::get<std::shared_ptr<Object>>(value);
      return interpretInstanceMethodInvocation(stream, frame, object, opText,
                                               op, args);
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

    while (current(stream).getSubType() == SubTokenType::Ops_Or) {
      next(stream);  // Skip "||"
      auto right = parseLogicalAnd(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs || rhs;
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

    while (current(stream).getSubType() == SubTokenType::Ops_And) {
      next(stream);  // Skip "&&"
      auto right = parseBitwiseOr(stream, frame);

      if (std::holds_alternative<bool>(left) &&
          std::holds_alternative<bool>(right)) {
        bool lhs = std::get<bool>(left), rhs = std::get<bool>(right);
        left = lhs && rhs;
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

    while (current(stream).getSubType() == SubTokenType::Ops_BitwiseOr) {
      next(stream);
      auto right = parseBitwiseXor(stream, frame);
      left = std::visit(BitwiseOrVisitor(current(stream)), left, right);
    }

    return left;
  }

  Value parseBitwiseXor(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitwiseAnd(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_BitwiseXor) {
      next(stream);
      auto right = parseBitwiseAnd(stream, frame);
      left = std::visit(BitwiseXorVisitor(current(stream)), left, right);
    }

    return left;
  }

  Value parseBitwiseAnd(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseEquality(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_BitwiseAnd) {
      next(stream);  // Skip "&"
      auto right = parseEquality(stream, frame);
      left = std::visit(BitwiseAndVisitor(current(stream)), left, right);
    }

    return left;
  }

  Value parseEquality(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseComparison(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_Equal ||
           current(stream).getSubType() == SubTokenType::Ops_NotEqual) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseComparison(stream, frame);

      if (op == SubTokenType::Ops_Equal) {
        left = std::visit(EqualityVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_NotEqual) {
        left = std::visit(InequalityVisitor(current(stream)), left, right);
      }
    }
    return left;
  }

  Value parseComparison(std::shared_ptr<TokenStream> stream,
                        std::shared_ptr<CallStackFrame> frame) {
    auto left = parseBitshift(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_GreaterThan ||
           current(stream).getSubType() ==
               SubTokenType::Ops_GreaterThanOrEqual ||
           current(stream).getSubType() == SubTokenType::Ops_LessThan ||
           current(stream).getSubType() == SubTokenType::Ops_LessThanOrEqual) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseBitshift(stream, frame);

      if (op == SubTokenType::Ops_LessThan) {
        left = std::visit(LessThanVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_LessThanOrEqual) {
        left = std::visit(LessThanOrEqualVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_GreaterThan) {
        left = std::visit(GreaterThanVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_GreaterThanOrEqual) {
        left =
            std::visit(GreaterThanOrEqualVisitor(current(stream)), left, right);
      }
    }

    return left;
  }

  Value parseBitshift(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseAddition(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_BitwiseLeftShift ||
           current(stream).getSubType() ==
               SubTokenType::Ops_BitwiseRightShift) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseAddition(stream, frame);

      if (op == SubTokenType::Ops_BitwiseLeftShift) {
        left =
            std::visit(BitwiseLeftShiftVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_BitwiseRightShift) {
        left =
            std::visit(BitwiseRightShiftVisitor(current(stream)), left, right);
      }
    }

    return left;
  }

  Value parseAddition(std::shared_ptr<TokenStream> stream,
                      std::shared_ptr<CallStackFrame> frame) {
    auto left = parseMultiplication(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_Add ||
           current(stream).getSubType() == SubTokenType::Ops_Subtract) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseMultiplication(stream, frame);

      if (op == SubTokenType::Ops_Add) {
        left = std::visit(AddVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_Subtract) {
        left = std::visit(SubtractVisitor(current(stream)), left, right);
      }
    }

    return left;
  }

  Value parseMultiplication(std::shared_ptr<TokenStream> stream,
                            std::shared_ptr<CallStackFrame> frame) {
    auto left = parseUnary(stream, frame);

    while (current(stream).getSubType() == SubTokenType::Ops_Multiply ||
           current(stream).getSubType() == SubTokenType::Ops_Divide ||
           current(stream).getSubType() == SubTokenType::Ops_Modulus ||
           current(stream).getSubType() == SubTokenType::Ops_Exponent) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseUnary(stream, frame);

      if (op == SubTokenType::Ops_Multiply) {
        left = std::visit(MultiplyVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_Divide) {
        left = std::visit(DivideVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_Modulus) {
        left = std::visit(ModuloVisitor(current(stream)), left, right);
      } else if (op == SubTokenType::Ops_Exponent) {
        left = std::visit(PowerVisitor(current(stream)), left, right);
      }
    }

    return left;
  }

  Value parseUnary(std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CallStackFrame> frame) {
    while (current(stream).getSubType() == SubTokenType::Ops_Not ||
           current(stream).getSubType() == SubTokenType::Ops_Subtract ||
           current(stream).getSubType() == SubTokenType::Ops_BitwiseNot) {
      auto op = current(stream).getSubType();
      next(stream);  // Skip operator
      auto right = parseUnary(stream, frame);

      if (op == SubTokenType::Ops_Not) {
        if (std::holds_alternative<bool>(right)) {
          bool rhs = std::get<bool>(right);
          return !rhs;
        } else if (std::holds_alternative<k_int>(right)) {
          int rhs = std::get<k_int>(right);
          if (rhs == 0) {
            return 1;
          } else if (rhs == 1) {
            return 0;
          }
        }

        throw ConversionError(current(stream),
                              "Expected a `Boolean` expression.");
      } else if (op == SubTokenType::Ops_Subtract) {
        if (std::holds_alternative<k_int>(right)) {
          return -std::get<k_int>(right);
        } else if (std::holds_alternative<double>(right)) {
          return -std::get<double>(right);
        } else {
          throw ConversionError(current(stream),
                                "Unary minus applied to a non-numeric value.");
        }
      } else if (op == SubTokenType::Ops_BitwiseNot) {
        return std::visit(BitwiseNotVisitor(current(stream)), right);
      }
    }

    return parsePrimary(stream, frame);
  }

  Value parsePrimary(std::shared_ptr<TokenStream> stream,
                     std::shared_ptr<CallStackFrame> frame) {
    if (current(stream).getValueType() == ValueType::Integer) {
      auto value = current(stream).toInteger();
      next(stream);
      return value;
    } else if (current(stream).getValueType() == ValueType::Double) {
      auto value = current(stream).toDouble();
      next(stream);
      return value;
    } else if (current(stream).getValueType() == ValueType::Boolean) {
      auto value = current(stream).toBoolean();
      next(stream);
      return value;
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
      return interpretIdentifier(stream, frame, false);
    } else if (current(stream).getSubType() == SubTokenType::KW_This) {
      return interpretSelfInvocationTerm(stream, frame);
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
    return interpretValueInvocation(stream, frame, result);
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

    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw InvalidOperationError(
          current(stream), "Syntax error near `this`. Missing identifier.");
    }
    auto identifier = current(stream).getText();
    next(stream);  // Skip the identifier

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

    throw UnimplementedMethodError(current(stream), clazz.getClassName(),
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

    auto toString = clazz.getMethod(KiwiBuiltins.ToS);
    std::vector<Value> parameters;
    Value returnValue = interpretInstanceMethodInvocation(
        stream, frame, object, KiwiBuiltins.ToS, SubTokenType::Builtin_Kiwi_ToS,
        parameters);

    // Should probably check that an overridden to_s() actually returns a string.
    return Serializer::serialize(returnValue);
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
    auto tempAssignment =
        InterpHelper::getTemporaryAssignment(current(stream), tempId);
    Lexer lexer("", input);
    for (const auto& t : lexer.getAllTokens()) {
      tempAssignment.push_back(t);
    }

    auto tempStream = std::make_shared<TokenStream>(tempAssignment);
    auto tempFrame = buildSubFrame(frame);
    interpretAssignment(tempStream, tempFrame, "", true);

    if (tempFrame->variables.find(tempId) == tempFrame->variables.end()) {
      throw SyntaxError(current(stream),
                        "Invalid string interpolation: `" + input + "`");
    }

    Value value = getVariable(stream, tempFrame, tempId);

    tempAssignment.clear();

    return value;
  }

  std::string interpolateString(std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CallStackFrame> frame) {
    auto input = current(stream).getText();
    int i = 0, size = input.length();
    int interpCount = 0;
    char c = '\0';

    std::ostringstream sv;
    std::ostringstream builder;

    bool hasSyntaxError = false;

    while (!hasSyntaxError && i < size) {
      c = input[i];

      switch (c) {
        case '$':
          if (i + 1 < size && input[i + 1] == '{') {
            ++interpCount;
            i += 2;  // Skip "${"
            continue;
          }
          break;

        case '}':
          if (interpCount > 0) {
            --interpCount;

            if (interpCount > 0) {
              hasSyntaxError = true;
              continue;
            }

            if (builder.tellp() > 0) {
              auto value = interpolateString(stream, frame, builder.str());
              if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
                sv << Serializer::serialize(value);
              } else {
                sv << interpolateObject(stream, frame, value);
              }
              builder.str("");
              ++i;
            }

            continue;
          }
          break;

        case '\\':
          if (i + 1 < size) {
            if (input[i + 1] == 't') {
              sv << "\t";
              i += 2;
              continue;
            } else if (input[i + 1] == 'n') {
              sv << "\n";
              i += 2;
              continue;
            } else if (input[i + 1] == 'r') {
              sv << "\r";
              i += 2;
              continue;
            }
          }
          break;
      }

      if (interpCount == 0) {
        sv << c;
      } else {
        builder << c;
      }

      ++i;
    }

    if (hasSyntaxError) {
      throw SyntaxError(
          current(stream),
          "Invalid syntax in string interpolation: `" + input + "`");
    }

    return sv.str();
  }

  void interpretLambdaAssignment(std::shared_ptr<TokenStream> stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 const std::string& name,
                                 const SubTokenType& op) {
    if (op != SubTokenType::Ops_Assign) {
      throw InvalidOperationError(
          current(stream),
          "Expected assignment operator in lambda assignment.");
    }

    Method lambda = interpretLambda(stream, frame);
    lambda.setName(name);
    frame->assignLambda(name, lambda);
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

    if (current(stream).getSubType() != SubTokenType::Ops_Assign) {
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
    auto value = getVariable(stream, frame, name);

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
    bool insertOp = current(stream).getSubType() ==
                    SubTokenType::Ops_BitwiseLeftShiftAssign;
    bool simpleAssignOp =
        current(stream).getSubType() == SubTokenType::Ops_Assign;
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

  std::string interpretAssignment(std::shared_ptr<TokenStream> stream,
                                  std::shared_ptr<CallStackFrame> frame,
                                  const std::string& identifier = "",
                                  bool isTemporary = false,
                                  bool isInstanceVariable = false) {
    std::string name;

    if (identifier.empty() &&
        current(stream).getType() == TokenType::IDENTIFIER) {
      name = current(stream).toString();
    } else if (!identifier.empty()) {
      name = identifier;
    }

    next(stream);  // Skip the identifier.

    if (current(stream).getType() == TokenType::OPERATOR) {
      auto op = current(stream).getSubType();
      next(stream);

      if (Operators.is_assignment_operator(op) ||
          op == SubTokenType::Ops_BitwiseLeftShift) {
        interpretAssignment(stream, name, op, frame, isTemporary,
                            isInstanceVariable);
      }
    } else if (current(stream).getType() == TokenType::OPEN_BRACKET) {
      if (!hasVariable(frame, name)) {
        throw VariableUndefinedError(current(stream), name);
      }

      if (!InterpHelper::isSliceAssignmentExpression(stream)) {
        throw SyntaxError(current(stream),
                          "Invalid slice-assignment expression.");
      }

      interpretSliceAssignment(stream, frame, name);
    } else if (current(stream).getType() == TokenType::DOT) {
      if (hasVariable(frame, name)) {
        auto value = getVariable(stream, frame, name);
        if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
          interpretInstanceMethodInvocation(stream, frame, name);
          return name;
        } else {
          throw InvalidOperationError(
              current(stream), "Unsupported operation on `" + name + "`");
        }
      }

      throw VariableUndefinedError(current(stream), name);
    }

    return name;
  }

  void interpretAssignment(std::shared_ptr<TokenStream> stream,
                           std::string& name, const SubTokenType& op,
                           std::shared_ptr<CallStackFrame> frame,
                           bool isTemporary = false,
                           bool isInstanceVariable = false) {
    if (current(stream).getType() == TokenType::LAMBDA) {
      interpretLambdaAssignment(stream, frame, name, op);
      return;
    }

    Value value = interpretExpression(stream, frame);

    if (op == SubTokenType::Ops_Assign) {
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

    if (!hasVariable(frame, name)) {
      throw VariableUndefinedError(current(stream), name);
    }

    if (op == SubTokenType::Ops_BitwiseLeftShift) {
      interpretAppendToList(stream, frame, value, name);
      return;
    }

    Value currentValue = getVariable(stream, frame, name);
    frame->variables[name] =
        InterpHelper::interpretAssignOp(stream, op, currentValue, value);
  }
};

#endif
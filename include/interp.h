#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <map>
#include <stack>
#include "errors/error.h"
#include "errors/handler.h"
#include "errors/state.h"
#include "logging/logger.h"
#include "math/boolexpr.h"
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
#include "globals.h"
#include "interp_builtin.h"
#include "interp_helper.h"
#include "stackframe.h"

class Interpreter {
 public:
  Interpreter(Logger& logger) : logger(logger) {}

  int interpret(Lexer& lexer, std::string parentPath = "") {
    files[lexer.getFile()] = lexer.getLines();
    return interpret(lexer.getAllTokens(), parentPath);
  }

  int interpret(std::vector<Token> tokens, std::string parentPath = "") {
    _tokens = tokens;
    _parentPath = parentPath;

    if (_tokens.empty()) {
      return 0;
    }

    CallStackFrame mainFrame(_tokens);
    callStack.push(mainFrame);

    interpretStackFrame();

    if (!preservingMainStackFrame && callStack.size() == 1) {
      callStack.pop();
    }

    return 0;
  }

  void preserveMainStackFrame() { preservingMainStackFrame = true; }

 private:
  Logger& logger;
  std::map<std::string, std::vector<std::string>> files;
  std::map<std::string, Method> methods;
  std::map<std::string, Module> modules;
  std::map<std::string, Class> classes;
  std::vector<Token> _tokens;
  std::string _parentPath;
  std::stack<CallStackFrame> callStack;
  std::stack<std::string> moduleStack;
  bool preservingMainStackFrame = false;

  Token current(CallStackFrame& frame) {
    if (frame.position >= frame.tokens.size()) {
      return Token::createEndOfFrame();
    }
    return frame.tokens[frame.position];
  }

  void next(CallStackFrame& frame) {
    if (frame.position < frame.tokens.size()) {
      frame.position++;
    } else {
      // To complete the frame, or not to complete the frame, that is the question;
    }
  }

  Token peek(CallStackFrame& frame) {
    size_t nextPosition = frame.position + 1;
    if (nextPosition < frame.tokens.size()) {
      return frame.tokens[nextPosition];
    } else {
      return Token::createEndOfFrame();
    }
  }

  void interpretStackFrame() {
    auto& frame = callStack.top();
    while (frame.position < frame.tokens.size()) {
      auto& token = frame.tokens[frame.position];

      try {
        interpretToken(token, frame);
      } catch (const KiwiError& e) {
        if (frame.isFlagSet(FrameFlags::InTry)) {
          frame.setErrorState(e);
        } else {
          ErrorHandler::handleError(e, files);
          exit(1);
        }
      } catch (const std::exception& e) {
        ErrorHandler::handleFatalError(e);
      }

      if (frame.isErrorStateSet()) {
        ++frame.position;
        continue;
      }

      bool tokenChange = false;

      if (frame.position < frame.tokens.size()) {
        token = frame.tokens[frame.position];
        tokenChange = true;
      }

      if (tokenChange && ((token.getType() != TokenType::KEYWORD &&
                           token.getType() != TokenType::CONDITIONAL &&
                           token.getType() != TokenType::DECLVAR) ||
                          (token.getText() == Keywords.End ||
                           token.getText() == Keywords.Try))) {
        if (methods.find(token.getText()) != methods.end()) {
          continue;
        }
        if (modules.find(token.getText()) != modules.end()) {
          continue;
        }
        if (peek(frame).getType() == TokenType::OPEN_PAREN ||
            peek(frame).getType() == TokenType::QUALIFIER) {
          continue;
        }
        if (peek(frame).getType() == TokenType::DOT) {
          continue;
        }

        if (!frame.isFlagSet(FrameFlags::ReturnFlag)) {
          ++frame.position;
        }
      }

      if (frame.isFlagSet(FrameFlags::LoopBreak) ||
          frame.isFlagSet(FrameFlags::LoopContinue)) {
        if (callStack.size() > 1) {
          // Pop and propagate to the top frame to be used in the next iteration.
          bool loopBreak = frame.isFlagSet(FrameFlags::LoopBreak);
          bool loopContinue = frame.isFlagSet(FrameFlags::LoopContinue);
          auto topVariables = std::move(frame.variables);
          callStack.pop();
          auto& callerFrame = callStack.top();
          InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
          if (loopBreak) {
            callerFrame.setFlag(FrameFlags::LoopBreak);
          }
          if (loopContinue) {
            callerFrame.setFlag(FrameFlags::LoopContinue);
          }
          return;
        }
      }

      if (frame.isFlagSet(FrameFlags::ReturnFlag)) {
        if (callStack.size() > 1) {
          // Handle the return value in the caller frame
          auto returnValue = std::move(frame.returnValue);
          auto topVariables = std::move(frame.variables);
          callStack.pop();  // Pop the current frame
          auto& callerFrame = callStack.top();
          callerFrame.returnValue = returnValue;
          if (callerFrame.isFlagSet(FrameFlags::SubFrame)) {
            callerFrame.setFlag(FrameFlags::ReturnFlag);
          }
          InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
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

    Value returnValue = std::move(frame.returnValue);
    bool inObjectContext = frame.inObjectContext();
    if (inObjectContext) {
      returnValue = std::move(frame.objectContext);
    }
    auto topVariables = std::move(frame.variables);
    callStack.pop();
    auto& callerFrame = callStack.top();

    if (inObjectContext) {
      callerFrame.returnValue = returnValue;
    }
    InterpHelper::updateVariablesInCallerFrame(topVariables, callerFrame);
  }

  void interpretForLoop(CallStackFrame& frame) {
    std::string itemVariableName, indexVariableName;
    bool hasIndexVariable = false;

    // Check if the loop includes an index variable.
    if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);
      itemVariableName = current(frame).getText();
      next(frame);  // Skip the item variable name

      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);  // Skip ','
        if (current(frame).getType() == TokenType::DECLVAR) {
          hasIndexVariable = true;
          next(frame);
          indexVariableName =
              current(frame)
                  .getText();  // Get the index variable name after '@'
          next(frame);         // Skip the index variable name
        }
      }
    }

    if (current(frame).getText() != Keywords.In) {
      throw SyntaxError(current(frame), "Expected 'in' after loop variables.");
    }
    next(frame);  // Skip 'in'

    Token tokenTerm = current(frame);
    BooleanExpressionBuilder booleanExpression;
    Value collectionValue = interpretExpression(booleanExpression, frame);

    if (!std::holds_alternative<std::shared_ptr<List>>(collectionValue)) {
      throw InvalidOperationError(tokenTerm, "Term is not a list.");
    }

    if (peek(frame).getText() != Keywords.Do &&
        current(frame).getText() != Keywords.Do) {
      throw SyntaxError(current(frame), "Expected `do` in `for` loop.");
    }

    if (peek(frame).getText() == Keywords.Do) {
      next(frame);
    }

    if (current(frame).getText() == Keywords.Do) {
      next(frame);
    }

    auto& collection = std::get<std::shared_ptr<List>>(collectionValue);

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, frame);

    // Execute the loop
    size_t index = 0;
    for (const auto& item : collection->elements) {
      if (frame.isFlagSet(FrameFlags::LoopBreak)) {
        break;
      }

      if (frame.isFlagSet(FrameFlags::LoopContinue)) {
        frame.clearFlag(FrameFlags::LoopContinue);
        continue;
      }

      frame.variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame.variables[indexVariableName] = static_cast<int>(index);
      }

      CallStackFrame loopFrame(loopTokens);
      for (const auto& pair : frame.variables) {
        loopFrame.variables[pair.first] = pair.second;
      }
      callStack.push(loopFrame);
      interpretStackFrame();

      index++;
    }
    frame.clearFlag(FrameFlags::LoopBreak);
    frame.clearFlag(FrameFlags::LoopContinue);
  }

  void interpretWhileLoop(CallStackFrame& frame) {
    std::vector<Token> conditionTokens;
    while (current(frame).getText() != Keywords.Do) {
      Token t = current(frame);
      conditionTokens.push_back(current(frame));
      next(frame);
    }

    next(frame);  // Skip "do"
    Token tokenTerm = current(frame);

    std::vector<Token> loopTokens;
    InterpHelper::collectBodyTokens(loopTokens, frame);

    CallStackFrame oldFrame = frame;
    std::string tempId = InterpHelper::getTemporaryId();
    std::vector<Token> tempAssignment =
        InterpHelper::getTemporaryAssignment(tokenTerm, tempId);
    // Interpret the condition.
    while (true) {
      if (frame.isFlagSet(FrameFlags::LoopBreak)) {
        break;
      }

      if (frame.isFlagSet(FrameFlags::LoopContinue)) {
        frame.clearFlag(FrameFlags::LoopContinue);
        continue;
      }

      std::vector<Token> condition = conditionTokens;
      auto it = condition.begin() + 0;
      condition.insert(it, tempAssignment.begin(), tempAssignment.end());

      CallStackFrame conditionFrame(condition);
      for (const auto& pair : frame.variables) {
        conditionFrame.variables[pair.first] = pair.second;
      }

      callStack.push(conditionFrame);
      interpretAssignment(conditionFrame, true);

      if (conditionFrame.variables.find(tempId) ==
          conditionFrame.variables.end()) {
        throw SyntaxError(current(frame), "Invalid condition in while-loop.");
      }

      Value value = getVariable(tempId, conditionFrame);
      callStack.pop();

      if (!std::holds_alternative<bool>(value)) {
        throw ConversionError(current(frame));
      }

      // Stop here.
      if (!std::get<bool>(value)) {
        break;
      }

      CallStackFrame codeFrame(loopTokens);
      for (const auto& pair : frame.variables) {
        codeFrame.variables[pair.first] = pair.second;
      }

      callStack.push(codeFrame);

      // Interpret the loop code.
      interpretStackFrame();
      frame = callStack.top();
    }
    frame.clearFlag(FrameFlags::LoopBreak);
    frame.clearFlag(FrameFlags::LoopContinue);

    for (const auto& pair : frame.variables) {
      if (InterpHelper::shouldUpdateFrameVariables(pair.first, oldFrame)) {
        oldFrame.variables[pair.first] = pair.second;
      }
    }

    frame = oldFrame;
  }

  void interpretLoop(CallStackFrame& frame) {
    std::string loop = current(frame).getText();
    next(frame);  // Skip "while"|"for"

    if (loop == Keywords.While) {
      interpretWhileLoop(frame);
    } else if (loop == Keywords.For) {
      interpretForLoop(frame);
    }
  }

  void interpretKeyword(const Token& token, CallStackFrame& frame) {
    const std::string& keyword = token.getText();

    if (keyword == Keywords.If) {
      interpretConditional(frame);
    } else if (keyword == Keywords.DeclVar) {
      interpretAssignment(frame);
    } else if (Keywords.is_loop_keyword(keyword)) {
      interpretLoop(frame);
    } else if (keyword == Keywords.Method) {
      interpretMethodDefinition(frame);
    } else if (keyword == Keywords.Return) {
      interpretReturn(frame);
    } else if (keyword == Keywords.PrintLn || keyword == Keywords.Print) {
      interpretPrint(frame, keyword == Keywords.PrintLn);
    } else if (keyword == Keywords.Import) {
      interpretImport(frame);
    } else if (keyword == Keywords.Module) {
      interpretModuleDefinition(frame);
    } else if (keyword == Keywords.Delete) {
      interpretDelete(frame);
    } else if (keyword == Keywords.Abstract || keyword == Keywords.Class) {
      interpretClassDefinition(frame);
    } else if (keyword == Keywords.This) {
      interpretSelfInvocation(frame);
    } else if (keyword == Keywords.Break) {
      frame.setFlag(FrameFlags::LoopBreak);
    } else if (keyword == Keywords.Next) {
      frame.setFlag(FrameFlags::LoopContinue);
    } else if (keyword == Keywords.Try) {
      frame.setFlag(FrameFlags::InTry);
    } else if (keyword == Keywords.Pass) {
      // skip
    } else if (keyword == Keywords.Catch) {
      interpretCatch(frame);
    } else {
      throw UnrecognizedTokenError(token);
    }
  }

  /// @brief
  /// @param token
  /// @param identifier
  /// @param frame
  void interpretQualifiedIdentifier(Token& token, std::string& identifier,
                                    CallStackFrame& frame) {
    while (peek(frame).getType() == TokenType::QUALIFIER) {
      next(frame);  // Skip the identifier.
      next(frame);  // Skip the qualifier.
      token = current(frame);
      identifier += Symbols.Qualifier + token.getText();
    }
  }

  void interpretIdentifierOperation(const std::string& identifier,
                                    CallStackFrame& frame) {
    next(frame);  // Skip identifier.
    std::string op = current(frame).getText();
    next(frame);

    if (current(frame).getType() == TokenType::LAMBDA) {
      interpretLambdaAssignment(identifier, op, frame);
    } else {
      throw InvalidOperationError(
          current(frame), "Invalid operation near `" + identifier + "`.");
    }
  }

  void interpretIdentifier(CallStackFrame& frame) {
    Token token = current(frame);
    std::string tokenText = token.getText();

    interpretQualifiedIdentifier(token, tokenText, frame);

    if (hasMethod(tokenText, frame)) {
      interpretMethodInvocation(tokenText, frame);
    } else if (hasVariable(tokenText, frame)) {
      Value v = getVariable(tokenText, frame);
      if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
        std::string lambdaRef =
            std::get<std::shared_ptr<LambdaRef>>(v)->identifier;
        if (frame.hasAssignedLambda(lambdaRef)) {
          interpretMethodInvocation(lambdaRef, frame);
        } else {
          throw InvalidOperationError(token,
                                      "Unknown Lambda `" + tokenText + "`.");
        }
      }
      // Skip it (for now).
    } else if (KiwiBuiltins.is_builtin_method(tokenText)) {
      interpretBuiltin(tokenText, frame);
    } else if (hasClass(tokenText)) {
      interpretClassMethodInvocation(tokenText, frame);
    } else if (peek(frame).getType() == TokenType::OPERATOR) {
      interpretIdentifierOperation(tokenText, frame);
    } else {
      throw UnknownIdentifierError(token, tokenText);
    }
  }

  void interpretCatch(CallStackFrame& frame) {
    next(frame);  // SKip "catch"

    std::string errorVariableName;
    Value errorValue;

    if (current(frame).getType() == TokenType::OPEN_PAREN) {
      InterpHelper::interpretParameterizedCatch(frame, errorVariableName,
                                                errorValue);
    }

    std::vector<Token> catchTokens;
    int count = 1;
    while (count != 0) {
      if (current(frame).getText() == Keywords.End) {
        --count;

        // Stop here.
        if (count == 0) {
          break;
        }
      } else if (Keywords.is_required_end_keyword(current(frame).getText())) {
        ++count;
      }

      catchTokens.push_back(current(frame));
      next(frame);
    }

    if (frame.isErrorStateSet()) {
      CallStackFrame catchFrame(catchTokens);

      for (const auto& pair : frame.variables) {
        catchFrame.variables[pair.first] = pair.second;
      }

      if (!errorVariableName.empty() &&
          std::holds_alternative<std::string>(errorValue)) {
        catchFrame.variables[errorVariableName] = errorValue;
      }

      callStack.push(catchFrame);
      interpretStackFrame();
      frame.clearFlag(FrameFlags::InTry);
      frame.clearErrorState();
    }
  }

  void interpretToken(const Token& token, CallStackFrame& frame) {
    if (frame.isErrorStateSet()) {
      if (token.getType() == TokenType::KEYWORD &&
          token.getText() == Keywords.Catch) {
        interpretCatch(frame);
      }
      return;
    }

    switch (token.getType()) {
      case TokenType::COMMENT:
      case TokenType::COMMA:
        break;

      case TokenType::KEYWORD:
      case TokenType::DECLVAR:
        interpretKeyword(token, frame);
        break;

      case TokenType::IDENTIFIER:
        interpretIdentifier(frame);
        break;

      case TokenType::CONDITIONAL:
        interpretConditional(frame);
        break;

      default:
        throw UnrecognizedTokenError(token);
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

  bool hasMethod(const std::string& name, CallStackFrame& frame) {
    if (frame.inObjectContext()) {
      Class clazz = classes[frame.getObjectContext()->className];
      if (clazz.hasMethod(name)) {
        return true;
      }
    }

    if (frame.hasAssignedLambda(name)) {
      return true;
    }

    return methods.find(name) != methods.end();
  }

  bool hasVariable(const std::string& name, CallStackFrame& frame) {
    if (frame.variables.find(name) != frame.variables.end()) {
      return true;  // Found in the current frame
    }

    if (frame.inObjectContext()) {
      if (frame.getObjectContext()->instanceVariables.find(name) !=
          frame.getObjectContext()->instanceVariables.end()) {
        return true;
      }
    }

    // Check in outer frames
    std::stack<CallStackFrame> tempStack(callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      CallStackFrame& outerFrame = tempStack.top();
      if (outerFrame.variables.find(name) != outerFrame.variables.end()) {
        return true;  // Found in an outer frame
      }
      tempStack.pop();
    }

    return false;  // Not found in any scope
  }

  Module getHomedModule(const std::string& homeName,
                        const std::string& moduleName, CallStackFrame& frame) {
    for (auto pair : modules) {
      if (pair.first == moduleName && pair.second.hasHome() &&
          pair.second.getHome() == homeName) {
        return pair.second;
      }
    }

    throw ModuleUndefinedError(current(frame), moduleName);
  }

  Module getModule(const std::string& name, CallStackFrame& frame) {
    if (hasModule(name)) {
      return modules[name];
    }

    throw ModuleUndefinedError(current(frame), name);
  }

  Method getMethod(const std::string& name, CallStackFrame& frame) {
    if (hasMethod(name, frame)) {
      if (frame.inObjectContext()) {
        Class clazz = classes[frame.getObjectContext()->className];
        if (clazz.hasMethod(name)) {
          return clazz.getMethod(name);
        }

        if (frame.hasAssignedLambda(name)) {
          return frame.getAssignedLambda(name);
        }
      }

      if (frame.hasAssignedLambda(name)) {
        return frame.getAssignedLambda(name);
      }

      return methods[name];
    }

    throw MethodUndefinedError(current(frame), name);
  }

  Value getVariable(const std::string& name, CallStackFrame& frame) {
    // Check in the current frame
    if (frame.variables.find(name) != frame.variables.end()) {
      Value value = frame.variables[name];
      return value;
    }

    if (frame.inObjectContext()) {
      if (frame.getObjectContext()->instanceVariables.find(name) !=
          frame.getObjectContext()->instanceVariables.end()) {
        Value value = frame.getObjectContext()->instanceVariables[name];
        return value;
      }
    }

    // Check in outer frames
    std::stack<CallStackFrame> tempStack(callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      CallStackFrame& outerFrame = tempStack.top();
      if (outerFrame.variables.find(name) != outerFrame.variables.end()) {
        Value value = outerFrame.variables[name];
        return value;  // Found in an outer frame
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(current(frame), name);
  }

  void interpretModuleImport(const std::string& home, const std::string& name,
                             CallStackFrame& frame) {
    next(frame);  // Skip the name.

    moduleStack.push(name);
    Module module = hasHomedModule(home, name)
                        ? getHomedModule(home, name, frame)
                        : getModule(name, frame);

    CallStackFrame codeFrame(module.getCode());
    callStack.push(codeFrame);
    interpretStackFrame();
    moduleStack.pop();
  }

  std::vector<Value> interpretArguments(CallStackFrame& frame) {
    std::vector<Value> args;

    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(frame),
                        "Expected open-parenthesis, `(`, near `" +
                            current(frame).getText() + "`.");
    }
    next(frame);  // Skip "("

    bool closeParenthesisFound = false;
    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
        continue;
      }

      Token subTokenTerm = current(frame);
      Value argValue;

      if (subTokenTerm.getType() == TokenType::TYPENAME) {
        argValue = subTokenTerm.getText();
      } else if (hasClass(subTokenTerm.getText())) {
        argValue = subTokenTerm.getText();
      } else {
        BooleanExpressionBuilder booleanExpression;
        argValue = interpretTerm(subTokenTerm, booleanExpression, frame);
      }

      if (peek(frame).getType() == TokenType::CLOSE_PAREN) {
        next(frame);
        closeParenthesisFound = true;
      }

      args.push_back(argValue);

      if (!closeParenthesisFound) {
        next(frame);
      }
    }
    next(frame);  // Skip ")"
    return args;
  }

  void interpretModuleBuiltin(const std::string moduleName,
                              const std::string& builtin,
                              std::vector<Value>& args, CallStackFrame& frame) {
    if (builtin == ModuleBuiltins.Home) {
      if (args.size() != 1) {
        throw BuiltinUnexpectedArgumentError(current(frame), builtin);
      }

      if (!std::holds_alternative<std::string>(args.at(0))) {
        throw SyntaxError(current(frame), "Expected string value for `" +
                                              builtin + "` builtin parameter.");
      }
      std::string value = std::get<std::string>(args.at(0));

      modules[moduleName].setHome(value);
      modules[moduleName].setName(moduleName);
    }
  }

  std::vector<std::string> collectMethodParameters(Token& tokenTerm,
                                                   Method& method,
                                                   CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected open-parenthesis, `(`, in method parameter set.");
    }
    next(frame);  // Skip "("

    // Interpret parameters.
    std::vector<std::string> parameters = method.getParameters();
    int paramIndex = 0;

    bool closeParenthesisFound = false;
    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
        continue;
      }

      std::string paramName = parameters.at(paramIndex++);
      BooleanExpressionBuilder booleanExpression;
      tokenTerm = current(frame);
      Value paramValue = interpretExpression(booleanExpression, frame);
      if (peek(frame).getType() == TokenType::CLOSE_PAREN) {
        next(frame);
        closeParenthesisFound = true;
      } else if (current(frame).getType() == TokenType::CLOSE_PAREN) {
        closeParenthesisFound = true;
      }
      method.addParameterValue(paramName, paramValue);
      if (!closeParenthesisFound) {
        next(frame);
      }
    }
    next(frame);  // Skip ")"

    return parameters;
  }

  CallStackFrame buildMethodInvocationStackFrame(Token& tokenTerm,
                                                 Method& method,
                                                 CallStackFrame& frame) {
    std::vector<std::string> parameters =
        collectMethodParameters(tokenTerm, method, frame);

    CallStackFrame codeFrame(method.getCode());
    for (const auto& pair : frame.variables) {
      codeFrame.variables[pair.first] = std::move(pair.second);
    }
    for (const auto& pair : frame.lambdas) {
      // Check this.
      codeFrame.assignLambda(pair.first, pair.second);
    }

    // Check all parameters are passed.
    for (const std::string& parameterName : parameters) {
      if (!method.hasParameter(parameterName)) {
        throw ParameterMissingError(tokenTerm, parameterName);
      } else {
        codeFrame.variables[parameterName] =
            std::move(method.getParameterValue(parameterName));
      }
    }
    codeFrame.setFlag(FrameFlags::SubFrame);
    return codeFrame;
  }

  void interpretClassMethodInvocation(const std::string& className,
                                      CallStackFrame& frame) {
    next(frame);  // Skip class name
    next(frame);  // Skip the "."

    std::string methodName = current(frame).getText();
    next(frame);  // Skip the method name.
    Class clazz = classes[className];
    Token tokenTerm = current(frame);
    std::shared_ptr<Object> context = std::make_shared<Object>();
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

    Method method = clazz.getMethod(methodName);
    if (!method.isFlagSet(MethodFlags::Static) &&
        !method.isFlagSet(MethodFlags::Ctor)) {
      throw InvalidContextError(
          tokenTerm, "The method `" + methodName +
                         "` can only be invoked on an instance of class `" +
                         className + "`.");
    }

    if (method.isFlagSet(MethodFlags::Private)) {
      if (!frame.inObjectContext() ||
          frame.getObjectContext()->className != className) {
        throw InvalidContextError(
            current(frame),
            "Cannot invoke private method outside of object context.");
      }
    }

    CallStackFrame codeFrame =
        buildMethodInvocationStackFrame(tokenTerm, method, frame);
    if (isInstantiation) {
      context->className = className;
      codeFrame.setObjectContext(context);
    }
    callStack.push(codeFrame);

    // Now interpret the method in its own context
    interpretStackFrame();
  }

  Value interpretInstanceMethodInvocation(std::shared_ptr<Object>& object,
                                          const std::string& methodName,
                                          std::vector<Value>& parameters,
                                          CallStackFrame& frame) {
    if (!hasClass(object->className)) {
      throw ClassUndefinedError(current(frame), object->className);
    }

    Class clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      if (KiwiBuiltins.is_builtin(methodName)) {
        return BuiltinInterpreter::execute(current(frame), methodName, object,
                                           parameters);
      }
      throw UnimplementedMethodError(current(frame), object->className,
                                     methodName);
    }

    Method method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame.inObjectContext()) {
      throw InvalidContextError(
          current(frame),
          "Cannot invoke private method outside of object context.");
    }

    CallStackFrame codeFrame(method.getCode());
    for (const auto& pair : frame.variables) {
      codeFrame.variables[pair.first] = pair.second;
    }

    if (static_cast<int>(parameters.size()) != method.getParameterCount()) {
      throw ParameterCountMismatchError(current(frame), methodName);
    }

    // Check all parameters are passed.
    int parameterIndex = 0;
    for (const std::string& parameterName : method.getParameters()) {
      codeFrame.variables[parameterName] = parameters.at(parameterIndex++);
    }
    codeFrame.setFlag(FrameFlags::SubFrame);
    codeFrame.setObjectContext(object);
    callStack.push(codeFrame);

    interpretStackFrame();

    Value value;
    if (!callStack.empty()) {
      value = callStack.top().returnValue;
      frame.clearFlag(FrameFlags::ReturnFlag);
    }

    return value;
  }

  void interpretInstanceMethodInvocation(const std::string& instanceName,
                                         CallStackFrame& frame) {
    next(frame);  // Skip the "."
    if (current(frame).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          current(frame),
          "Expected identifier in instance method invocation, instead got: `" +
              current(frame).getText() + "`");
    }
    Token tokenTerm = current(frame);
    std::string methodName = tokenTerm.getText();
    next(frame);  // Skip the method name.

    Value instanceValue = getVariable(instanceName, frame);
    std::shared_ptr<Object> object =
        std::get<std::shared_ptr<Object>>(instanceValue);

    if (!hasClass(object->className)) {
      throw ClassUndefinedError(tokenTerm, object->className);
    }

    Class clazz = classes[object->className];
    if (!clazz.hasMethod(methodName)) {
      throw UnimplementedMethodError(tokenTerm, object->className, methodName);
    }

    Method method = clazz.getMethod(methodName);

    if (method.isFlagSet(MethodFlags::Private) && !frame.inObjectContext()) {
      throw InvalidContextError(
          tokenTerm, "Cannot invoke private method outside of object context.");
    }

    CallStackFrame codeFrame =
        buildMethodInvocationStackFrame(tokenTerm, method, frame);
    codeFrame.setObjectContext(object);
    callStack.push(codeFrame);

    interpretStackFrame();
  }

  void interpretMethodInvocation(const std::string& name,
                                 CallStackFrame& frame) {
    next(frame);  // Skip the name.
    Method method = getMethod(name, frame);

    Token tokenTerm = current(frame);

    CallStackFrame codeFrame =
        buildMethodInvocationStackFrame(tokenTerm, method, frame);
    if (frame.inObjectContext()) {
      codeFrame.setObjectContext(frame.getObjectContext());
    }
    callStack.push(codeFrame);

    interpretStackFrame();
  }

  void interpretMethodDefinition(CallStackFrame& frame) {
    Method method = InterpHelper::interpretMethodDeclaration(frame);
    std::string name = method.getName();
    std::string moduleName;

    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    if (!moduleName.empty()) {
      name = moduleName + Symbols.Qualifier + name;
    }

    if (Keywords.is_keyword(name)) {
      Token tokenTerm = current(frame);
      throw IllegalNameError(tokenTerm, name);
    }

    method.setName(name);
    methods[name] = method;
  }

  void interpretReturn(CallStackFrame& frame) {
    bool hasValue = InterpHelper::hasReturnValue(frame);
    next(frame);  // Skip "return"

    Value returnValue;
    BooleanExpressionBuilder ifExpression;
    if (hasValue) {
      returnValue = interpretExpression(ifExpression, frame);
      if (ifExpression.isSet()) {
        if (std::holds_alternative<bool>(returnValue)) {
          returnValue = ifExpression.evaluate();
        }
      }
    }

    frame.returnValue = returnValue;
    frame.setFlag(FrameFlags::ReturnFlag);
  }

  std::string interpretAssignment(CallStackFrame& frame,
                                  bool isTemporary = false) {
    std::string name;
    if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);  // Skip the "@"
    }

    if (current(frame).getType() == TokenType::IDENTIFIER) {
      name = current(frame).toString();
      next(frame);

      if (current(frame).getType() == TokenType::OPERATOR) {
        std::string op = current(frame).toString();
        next(frame);

        if (Operators.is_assignment_operator(op) ||
            op == Operators.ListAppend) {
          interpretAssignment(name, op, frame, isTemporary);
        }
      } else if (current(frame).getType() == TokenType::OPEN_BRACKET) {
        if (!hasVariable(name, frame)) {
          throw VariableUndefinedError(current(frame), name);
        }

        if (!InterpHelper::isSliceAssignmentExpression(frame)) {
          throw SyntaxError(current(frame),
                            "Invalid slice-assignment expression.");
        }

        interpretSliceAssignment(frame, name);
      } else if (current(frame).getType() == TokenType::DOT) {
        if (hasVariable(name, frame)) {
          Value value = getVariable(name, frame);
          if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
            interpretInstanceMethodInvocation(name, frame);
            return name;
          } else {
            throw InvalidOperationError(
                current(frame), "Unsupported operation on `" + name + "`");
          }
        }

        throw VariableUndefinedError(current(frame), name);
      }
    }

    return name;
  }

  void interpretHashElementAssignment(CallStackFrame& frame,
                                      const std::string& name, Value& value) {
    std::string key;

    if (current(frame).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(
          current(frame),
          "Expected open-bracket, `[`, in hash element assignment.");
    }
    next(frame);  // Skip "["

    BooleanExpressionBuilder booleanExpression;
    Value keyValue = interpretExpression(booleanExpression, frame);

    if (!std::holds_alternative<std::string>(keyValue)) {
      throw SyntaxError(current(frame), "Hash key must be a string value.");
    }
    key = std::get<std::string>(keyValue);

    if (peek(frame).getType() == TokenType::CLOSE_BRACKET) {
      next(frame);
    }

    if (current(frame).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(
          current(frame),
          "Expected close-bracket, `]`, in hash element assignment.");
    }
    next(frame);

    std::string op = "";
    if (current(frame).getType() == TokenType::OPERATOR) {
      op = current(frame).getText();
      next(frame);
    }

    if (op != Operators.Assign) {
      throw InvalidOperationError(current(frame),
                                  "Expected assignment operator.");
    }

    Value elementValue = interpretExpression(booleanExpression, frame);

    /*if (std::holds_alternative<std::shared_ptr<Hash>>(elementValue)) {
      std::shared_ptr<Hash> newHashElementValue = std::make_shared<Hash>();
      std::shared_ptr<Hash> hashElementValue = std::get<std::shared_ptr<Hash>>(elementValue);
      for (const auto& kvp : hashElementValue->kvp) {
        newHashElementValue->kvp[kvp.first] = kvp.second;
      }
      elementValue = hashElementValue;
    }*/

    std::shared_ptr<Hash> hashValue = std::get<std::shared_ptr<Hash>>(value);
    hashValue->kvp[key] = elementValue;

    frame.variables[name] = hashValue;
  }

  void interpretSliceAssignment(CallStackFrame& frame,
                                const std::string& name) {
    Value value = getVariable(name, frame);

    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      interpretHashElementAssignment(frame, name, value);
      return;
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(current(frame),
                                  "`" + name + "` is not a list.");
    }

    // Parse slice parameters (start:stop:step)
    SliceIndex slice = interpretSliceIndex(frame, value);

    if (peek(frame).getType() == TokenType::OPERATOR) {
      next(frame);
    }

    // Expect assignment operator next
    bool insertOp = current(frame).getText() == Operators.ListInsert;
    bool simpleAssignOp = current(frame).getText() == Operators.Assign;
    if (!insertOp && !simpleAssignOp) {
      throw SyntaxError(current(frame),
                        "Expected assignment operator in slice assignment.");
    }
    next(frame);  // Move past the assignment operator

    BooleanExpressionBuilder booleanExpression;
    Value rhsValues = interpretExpression(booleanExpression, frame);

    // Get the target list to update
    auto targetListPtr = std::get<std::shared_ptr<List>>(value);

    auto rhsList = Serializer::convert_value_to_list(rhsValues);
    InterpHelper::updateListSlice(frame, insertOp, targetListPtr, slice,
                                  rhsList);
  }

  SliceIndex interpretSliceIndex(CallStackFrame& frame, Value& listValue) {
    if (current(frame).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(current(frame),
                        "Expected open-bracket, `[`, for list access.");
    }
    next(frame);  // Skip "["

    auto listPtr = std::get<std::shared_ptr<List>>(listValue);

    BooleanExpressionBuilder booleanExpression;
    SliceIndex slice;
    slice.indexOrStart = 0;
    slice.stopIndex = static_cast<int>(listPtr->elements.size());
    slice.stepValue = 1;
    slice.isSlice = false;

    // Detect if slicing or single index
    if (peek(frame).getType() == TokenType::COLON ||
        current(frame).getType() == TokenType::COLON) {
      slice.isSlice = true;
      if (current(frame).getType() != TokenType::COLON) {
        slice.indexOrStart = interpretExpression(booleanExpression, frame);
      }

      if (peek(frame).getType() == TokenType::COLON) {
        next(frame);  // Consume colon for start:stop:
      }

      if (current(frame).getType() == TokenType::COLON) {
        next(frame);  // Consume colon for start::step or ::step
        if (current(frame).getType() != TokenType::CLOSE_BRACKET) {
          slice.stopIndex = interpretExpression(booleanExpression, frame);
        }
      }

      if (peek(frame).getType() == TokenType::COLON) {
        next(frame);  // Consume colon for ::step
        if (current(frame).getType() == TokenType::COLON) {
          next(frame);
        }
        if (current(frame).getType() != TokenType::CLOSE_BRACKET) {
          slice.stepValue = interpretExpression(booleanExpression, frame);
        }
      }
    } else if (current(frame).getType() == TokenType::QUALIFIER) {
      next(frame);
      slice.isSlice = true;
      if (current(frame).getType() != TokenType::CLOSE_BRACKET) {
        slice.stepValue = interpretExpression(booleanExpression, frame);
      }
    } else {
      slice.indexOrStart =
          interpretExpression(booleanExpression, frame);  // Single index
    }

    if (peek(frame).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(current(frame),
                        "Expected close-bracket, `]`, for list access.");
    }
    next(frame);

    return slice;
  }

  Value interpretKeyOrIndex(CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_BRACKET) {
      throw SyntaxError(current(frame),
                        "Expected open-bracket, `[`, in key or index access.");
    }
    next(frame);  // Skip "["

    BooleanExpressionBuilder booleanExpression;
    Value output = interpretExpression(booleanExpression, frame);

    if (peek(frame).getType() != TokenType::CLOSE_BRACKET) {
      throw SyntaxError(current(frame),
                        "Expected close-bracket, `]`, in key or index access.");
    }
    next(frame);
    return output;
  }

  std::string interpretKey(CallStackFrame& frame) {
    Value output = interpretKeyOrIndex(frame);

    if (!std::holds_alternative<std::string>(output)) {
      throw SyntaxError(current(frame), "Hash key must be a string value.");
    }

    return std::get<std::string>(output);
  }

  int interpretIndex(CallStackFrame& frame) {
    Value output = interpretKeyOrIndex(frame);

    if (!std::holds_alternative<int>(output)) {
      throw SyntaxError(current(frame), "List index must be an integer value.");
    }

    return std::get<int>(output);
  }

  Value interpretHashElementAccess(CallStackFrame& frame, Value& value) {
    Token tokenTerm = current(frame);

    std::string key = interpretKey(frame);
    std::shared_ptr<Hash> hash = std::get<std::shared_ptr<Hash>>(value);
    if (hash->kvp.find(key) == hash->kvp.end()) {
      throw HashKeyError(current(frame), key);
    }

    return hash->kvp[key];
  }

  Value interpretSlice(CallStackFrame& frame, const std::string& name) {
    Value value = getVariable(name, frame);
    if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      return interpretHashElementAccess(frame, value);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(current(frame),
                                  "`" + name + "` is not a list.");
    }

    SliceIndex slice = interpretSliceIndex(frame, value);
    auto list = std::get<std::shared_ptr<List>>(value);

    return InterpHelper::interpretListSlice(slice, list, frame);
  }

  void interpretAppendToList(CallStackFrame& frame, Value& listValue,
                             const std::string& listVariableName) {
    Token tokenTerm = current(frame);

    Value variableValue;
    try {
      variableValue = getVariable(listVariableName, frame);
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

  std::shared_ptr<List> interpretRange(
      BooleanExpressionBuilder& booleanExpression, CallStackFrame& frame) {
    std::shared_ptr<List> list = std::make_shared<List>();

    next(frame);  // Skip the "["

    auto startValue = interpretExpression(booleanExpression, frame);

    if (peek(frame).getType() != TokenType::RANGE) {
      throw RangeError(current(frame),
                       "Expected range separator, `..`, in range expression.");
    } else {
      next(frame);
    }

    next(frame);

    auto stopValue = interpretExpression(booleanExpression, frame);

    if (peek(frame).getType() != TokenType::CLOSE_BRACKET) {
      throw RangeError(current(frame),
                       "Expected close-bracket, `]`, in range expression.");
    } else {
      next(frame);
    }

    next(frame);  // Skip the "]"

    if (!std::holds_alternative<int>(startValue)) {
      throw RangeError(current(frame),
                       "A range start value must be an integer.");
    }

    if (!std::holds_alternative<int>(stopValue)) {
      throw RangeError(current(frame),
                       "A range stop value must be an integer.");
    }

    int start = std::get<int>(startValue), stop = std::get<int>(stopValue);
    int step = stop < start ? -1 : 1;
    int i = start;

    for (; i != stop; i += step) {
      list->elements.push_back(i);
    }
    list->elements.push_back(i);

    return list;
  }

  std::shared_ptr<Hash> interpretHash(
      BooleanExpressionBuilder& booleanExpression, CallStackFrame& frame) {
    std::shared_ptr<Hash> hash = std::make_shared<Hash>();
    next(frame);  // Skip the "{"

    while (current(frame).getType() != TokenType::CLOSE_BRACE) {
      auto keyValue = interpretExpression(booleanExpression, frame);
      if (!std::holds_alternative<std::string>(keyValue)) {
        throw SyntaxError(current(frame), "Hash key must be a string value.");
      }
      std::string key = std::get<std::string>(keyValue);

      if (peek(frame).getType() == TokenType::COLON) {
        next(frame);
      }

      if (current(frame).getType() == TokenType::COLON) {
        next(frame);  // Skip the ":"
        auto value = interpretExpression(booleanExpression, frame);
        hash->kvp[key] = value;
      }

      if (peek(frame).getType() == TokenType::COMMA) {
        next(frame);  // Skip current value
      }

      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
      }

      if (peek(frame).getType() == TokenType::CLOSE_BRACE) {
        next(frame);
      }
    }

    next(frame);  // Skip the "}"

    return hash;
  }

  std::shared_ptr<List> interpretList(
      BooleanExpressionBuilder& booleanExpression, CallStackFrame& frame) {
    std::shared_ptr<List> list = std::make_shared<List>();
    next(frame);  // Skip "["
    int bracketCount = 1;

    while (bracketCount > 0) {
      if (current(frame).getType() == TokenType::OPEN_BRACKET) {
        ++bracketCount;
        // It's another list.
        auto value = interpretExpression(booleanExpression, frame);
        list->elements.push_back(value);
        next(frame);
        continue;
      } else if (current(frame).getType() == TokenType::CLOSE_BRACKET) {
        --bracketCount;

        // Stop here.
        if (bracketCount == 0) {
          break;
        }
      } else if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
        continue;
      } else {
        auto value = interpretExpression(booleanExpression, frame);
        list->elements.push_back(value);

        if (peek(frame).getType() == TokenType::COMMA ||
            peek(frame).getType() == TokenType::CLOSE_BRACKET) {
          next(frame);
          continue;
        }
      }

      if (current(frame).getType() == TokenType::COMMA ||
          current(frame).getType() == TokenType::CLOSE_BRACKET) {
        continue;
      }
      next(frame);
    }

    return list;
  }

  void interpretConditional(CallStackFrame& frame) {
    if (current(frame).getText() != Keywords.If) {
      throw SyntaxError(current(frame), "Invalid conditional. Expected `" +
                                            Keywords.If +
                                            "` keyword, instead got: `" +
                                            current(frame).getText() + "`");
    }

    next(frame);  // Skip "if"

    Conditional conditional;
    int ifCount = 1;
    bool ifValue = false;
    bool hasTrueElseIfEvaluation =
        false;  // An optimization to prevent unnecessary evaluation.
    std::string building = Keywords.If;

    // Eagerly evaluate the If conditions.
    BooleanExpressionBuilder ifExpression;
    Value value = interpretExpression(ifExpression, frame);
    if (ifExpression.isSet()) {
      value = ifExpression.evaluate();
    }

    if (std::holds_alternative<bool>(value)) {
      ifValue = std::get<bool>(value);
      conditional.getIfStatement().setEvaluation(ifValue);
      if (current(frame).getType() != TokenType::KEYWORD ||
          current(frame).getType() != TokenType::DECLVAR) {
        if (current(frame).getType() == TokenType::LITERAL) {
          next(frame);
        } else if (current(frame).getType() == TokenType::IDENTIFIER &&
                   !hasModule(current(frame).getText())) {
          next(frame);
        }
      }
    } else {
      throw ConversionError(current(frame));
    }

    while (current(frame).getType() == TokenType::CLOSE_PAREN ||
           current(frame).getType() == TokenType::CLOSE_BRACKET) {
      next(frame);
    }

    while (ifCount > 0) {
      if (Keywords.is_required_end_keyword(current(frame).getText())) {
        ++ifCount;
      } else if (current(frame).getText() == Keywords.End && ifCount > 0) {
        --ifCount;

        // Stop here.
        if (ifCount == 0) {
          next(frame);
          continue;
        }
      } else if (current(frame).getText() == Keywords.Else && ifCount == 1) {
        building = Keywords.Else;
        next(frame);
      } else if (current(frame).getText() == Keywords.ElseIf && ifCount == 1) {
        building = Keywords.ElseIf;
        next(frame);

        conditional.addElseIfStatement();

        // No need to evaluate if the previous condition is true.
        if (!ifValue && !hasTrueElseIfEvaluation) {
          // Eagerly evaluate ElseIf conditions.
          BooleanExpressionBuilder elseIfExpression;
          value = interpretExpression(elseIfExpression, frame);

          if (elseIfExpression.isSet()) {
            value = elseIfExpression.evaluate();
          }

          if (std::holds_alternative<bool>(value)) {
            bool elseIfValue = std::get<bool>(value);
            if (elseIfValue) {
              hasTrueElseIfEvaluation = true;
            }
            conditional.getElseIfStatement().setEvaluation(elseIfValue);
            Token bodyToken = current(frame);
            if (bodyToken.getType() == TokenType::IDENTIFIER ||
                bodyToken.getType() == TokenType::LITERAL) {
              next(frame);
            } else {
              conditional.getElseIfStatement().addToken(bodyToken);
            }
          } else {
            throw ConversionError(current(frame));
          }
        }
        continue;
      }

      // Distribute tokens to be executed.
      Token bodyToken = current(frame);
      if (building == Keywords.If) {
        conditional.getIfStatement().addToken(bodyToken);
      } else if (building == Keywords.ElseIf) {
        conditional.getElseIfStatement().addToken(bodyToken);
      } else if (building == Keywords.Else) {
        conditional.getElseStatement().addToken(bodyToken);
      }

      next(frame);
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
      CallStackFrame executableFrame(executableTokens);
      for (const auto& pair : frame.variables) {
        executableFrame.variables[pair.first] = pair.second;
      }

      callStack.push(executableFrame);
      interpretStackFrame();
    }
  }

  /*// TODO: implement this.
  void interpretSuperInvocation(CallStackFrame& frame) {
    
  }
*/

  void interpretSelfInvocation(CallStackFrame& frame) {
    if (!frame.inObjectContext()) {
      throw InvalidContextError(current(frame),
                                "Invalid context for keyword `this`.");
    }

    next(frame);  // Skip "this"
    if (current(frame).getType() != TokenType::DOT) {
      throw SyntaxError(current(frame), "Invalid syntax near keyword `this`.");
    }
    next(frame);  // Skip "."

    if (current(frame).getType() == TokenType::DECLVAR) {
      interpretAssignment(frame);
    } else if (current(frame).getType() == TokenType::IDENTIFIER &&
               peek(frame).getType() == TokenType::OPEN_PAREN) {
      interpretMethodInvocation(current(frame).getText(), frame);
    }
  }

  void interpretClassDefinition(CallStackFrame& frame) {
    bool isAbstract = current(frame).getText() == Keywords.Abstract;
    std::string moduleName;
    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    while (current(frame).getText() != Keywords.Class) {
      next(frame);
    }
    next(frame);  // Skip "class"

    Token tokenTerm = current(frame);
    std::string className = current(frame).getText();
    next(frame);  // Skip class name.

    if (classes.find(className) != classes.end()) {
      throw ClassRedefinitionError(tokenTerm, className);
    }

    std::string baseClassName = InterpHelper::interpretBaseClass(frame);

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
      Class baseClass = classes[baseClassName];
      for (auto& pair : baseClass.getMethods()) {
        clazz.addMethod(pair.second);
      }
    }

    int counter = 1;
    while (counter > 0) {
      std::string tokenText = current(frame).getText();
      if (tokenText == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          break;
        }
      } else if (tokenText == Keywords.Abstract ||
                 tokenText == Keywords.Override ||
                 tokenText == Keywords.Method ||
                 tokenText == Keywords.Private ||
                 tokenText == Keywords.Static) {
        if (tokenText == Keywords.Private) {
          if (peek(frame).getType() == TokenType::OPEN_PAREN) {
            next(frame);  // Skip "private"
            for (std::string privateVar :
                 InterpHelper::getParameterSet(frame)) {
              clazz.addPrivateVariable(privateVar);
            }
          }
        }

        Method method = InterpHelper::interpretMethodDeclaration(frame);
        if (!method.isFlagSet(MethodFlags::Abstract) &&
            current(frame).getText() == Keywords.End) {
          next(frame);
        }

        if (method.getName() == Keywords.Ctor) {
          method.setFlag(MethodFlags::Ctor);
        }

        if (clazz.hasMethod(method.getName())) {
          Method classMethod = clazz.getMethod(method.getName());
          if (!method.isFlagSet(MethodFlags::Override) &&
              classMethod.isFlagSet(MethodFlags::Abstract)) {
            throw SyntaxError(current(frame),
                              "The class, `" + className +
                                  "` has an abstract definition for `" +
                                  method.getName() +
                                  "` and the `override` keyword is missing.");
          }
        }

        clazz.addMethod(method);
        continue;
      }

      next(frame);
    }

    if (current(frame).getText() != Keywords.End) {
      throw SyntaxError(tokenTerm,
                        "Expected `end` keyword at end of class definition.");
    }

    // Check for unimplemented abstract methods.
    if (!clazz.isAbstract()) {
      for (const auto& pair : clazz.getMethods()) {
        if (pair.second.isFlagSet(MethodFlags::Abstract)) {
          throw UnimplementedMethodError(current(frame), className,
                                         pair.second.getName());
        }
      }
    }

    classes[className] = std::move(clazz);
  }

  void interpretModuleDefinition(CallStackFrame& frame) {
    next(frame);  // Skip "module"

    std::string name = current(frame).getText();
    if (hasModule(name)) {
      // WIP: Mixins?
    }

    next(frame);  // Skip the module name.

    Module module;
    int counter = 1;
    while (counter > 0) {
      if (current(frame).getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          break;
        }
      } else if (Keywords.is_required_end_keyword(current(frame).getText())) {
        ++counter;
      }

      Token codeToken = current(frame);
      module.addToken(codeToken);
      next(frame);
    }

    modules[name] = std::move(module);
  }

  void interpretExternalImport(CallStackFrame& frame) {
    BooleanExpressionBuilder booleanExpression;
    Value scriptNameValue = interpretExpression(booleanExpression, frame);
    if (!std::holds_alternative<std::string>(scriptNameValue)) {
      throw ConversionError(current(frame),
                            "Expected a string for `import` statement.");
    }
    std::string scriptName = std::get<std::string>(scriptNameValue);
    if (!Strings::ends_with(scriptName, ".kiwi")) {
      scriptName += ".kiwi";
    }
    std::string scriptPath = FileIO::joinPath(_parentPath, scriptName);
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    next(frame);

    std::string content = FileIO::readFile(scriptPath);
    if (content.empty()) {
      return;
    }

    Lexer lexer(scriptPath, content);
    files[scriptPath] = lexer.getLines();
    std::vector<Token> tokens = lexer.getAllTokens();
    CallStackFrame scriptFrame(tokens);
    for (const auto& pair : frame.variables) {
      scriptFrame.variables[pair.first] = pair.second;
    }

    callStack.push(scriptFrame);
    interpretStackFrame();
  }

  void interpretImport(CallStackFrame& frame) {
    next(frame);  // skip the "import"

    std::string tokenText = current(frame).getText();
    std::string moduleHome =
        InterpHelper::interpretModuleHome(tokenText, frame);

    if (hasModule(tokenText)) {
      interpretModuleImport(moduleHome, tokenText, frame);
    } else {
      interpretExternalImport(frame);
    }
  }

  Value interpretVariableValue(std::string name, CallStackFrame& frame) {
    if (!hasVariable(name, frame)) {
      throw KiwiError(current(frame), "Unknown variable `" + name + "`");
    }

    BooleanExpressionBuilder booleanExpression;
    Value value = interpretExpression(booleanExpression, frame);
    if (booleanExpression.isSet()) {
      value = booleanExpression.evaluate();
    }

    return value;
  }

  void interpretDeleteHashKey(CallStackFrame& frame, const std::string& name,
                              Value& value) {
    std::string key = interpretKey(frame);
    std::shared_ptr<Hash> hash = std::get<std::shared_ptr<Hash>>(value);

    if (hash->kvp.find(key) == hash->kvp.end()) {
      throw HashKeyError(current(frame), key);
    }

    hash->kvp.erase(key);
    frame.variables[name] = hash;
  }

  void interpretDeleteListIndex(CallStackFrame& frame, const std::string& name,
                                Value& value) {
    int index = interpretIndex(frame);
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(value);

    if (index < 0 || index >= static_cast<int>(list->elements.size())) {
      throw RangeError(current(frame), "List index out of range.");
    }

    list->elements.erase(list->elements.begin() + index);
    frame.variables[name] = list;
  }

  void interpretDelete(CallStackFrame& frame) {
    next(frame);  // Skip "delete"

    if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);
      std::string name = current(frame).getText();

      if (!hasVariable(name, frame)) {
        throw VariableUndefinedError(current(frame), name);
      }
      Value value = getVariable(name, frame);
      next(frame);

      if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
        interpretDeleteHashKey(frame, name, value);
      } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
        interpretDeleteListIndex(frame, name, value);
      }

      return;
    }

    throw SyntaxError(current(frame),
                      "Cannot delete from a non-variable value.");
  }

  void interpretPrint(CallStackFrame& frame, bool printNewLine = false) {
    next(frame);  // skip the "print"
    BooleanExpressionBuilder booleanExpression;

    Value value;

    if (current(frame).getType() == TokenType::STRING) {
      value = interpretExpression(booleanExpression, frame);
    } else if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);
      std::string name = current(frame).getText();

      value = interpretVariableValue(name, frame);

    } else if (current(frame).getType() == TokenType::OPEN_BRACKET) {
      value = interpretBracketExpression(booleanExpression, frame);
    } else if (current(frame).getType() == TokenType::IDENTIFIER) {
      value = interpretExpression(booleanExpression, frame);
    } else if (current(frame).getType() == TokenType::KEYWORD &&
               current(frame).getText() == Keywords.This) {
      value = interpretExpression(booleanExpression, frame);
    } else {
      throw ConversionError(current(frame));
    }

    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      std::cout << Serializer::serialize(value);
    } else {
      std::cout << interpolateObject(value, frame);
    }

    if (printNewLine) {
      std::cout << std::endl;
    }
  }

  Value interpretBracketExpression(BooleanExpressionBuilder& booleanExpression,
                                   CallStackFrame& frame) {
    Value value;
    if (InterpHelper::isListExpression(frame)) {
      value = interpretList(booleanExpression, frame);
    } else if (InterpHelper::isRangeExpression(frame)) {
      value = interpretRange(booleanExpression, frame);
    }
    if (current(frame).getType() == TokenType::DOT) {
      value = interpretDotNotation(value, frame);
    }
    return value;
  }

  void interpretBooleanExpression(Token& tokenTerm,
                                  BooleanExpressionBuilder& booleanExpression,
                                  std::string& op, CallStackFrame& frame) {
    Value nextTerm = interpretExpression(booleanExpression, frame);

    // Check if the last term is valueless.
    if (nextTerm.valueless_by_exception()) {
      throw SyntaxError(tokenTerm,
                        "The boolean expression is valueless, near `" +
                            tokenTerm.getText() + "`.");
    }

    // We can't use non-boolean values in our expression.
    if (!std::holds_alternative<bool>(nextTerm)) {
      throw ConversionError(tokenTerm);
    }

    bool nextTermValue = std::get<bool>(nextTerm);

    if (op == Operators.And) {
      booleanExpression.andOperation(
          std::make_shared<ValueNode>(nextTermValue));
    } else if (op == Operators.Or) {
      booleanExpression.orOperation(std::make_shared<ValueNode>(nextTermValue));
    } else if (op == Operators.Not) {
      InterpHelper::ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression,
                                                   nextTerm);
      booleanExpression.notOperation();
    }
  }

  void interpretBuiltin(const std::string& builtin, CallStackFrame& frame) {
    Token tokenTerm = current(frame);
    next(frame);  // Skip the name.

    auto args = interpretArguments(frame);

    if (ModuleBuiltins.is_builtin(builtin)) {
      if (moduleStack.empty()) {
        throw InvalidContextError(
            tokenTerm, "Builtin `" + builtin +
                           "` is illegal outside of a module context.");
      }
      std::string moduleName = moduleStack.top();
      interpretModuleBuiltin(moduleName, builtin, args, frame);
    } else {
      frame.returnValue = BuiltinInterpreter::execute(tokenTerm, builtin, args);
    }
  }

  Value interpretLambdaMap(const std::shared_ptr<List>& list,
                           CallStackFrame& frame) {
    next(frame);  // Skip "("

    Method lambda = InterpHelper::getLambda(frame);

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(frame),
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
        throw BuiltinUnexpectedArgumentError(current(frame),
                                             SpecializedBuiltins.Map);
      }
    }

    std::shared_ptr<List> mappedList = std::make_shared<List>();

    size_t index = 0;
    for (const auto& item : list->elements) {
      frame.variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame.variables[indexVariableName] = static_cast<int>(index);
      }

      CallStackFrame loopFrame(lambda.getCode());
      for (const auto& pair : frame.variables) {
        loopFrame.variables[pair.first] = pair.second;
      }
      callStack.push(loopFrame);
      interpretStackFrame();

      if (!callStack.empty()) {
        Value value = callStack.top().returnValue;
        frame.clearFlag(FrameFlags::ReturnFlag);
        mappedList->elements.push_back(value);
      }
      index++;
    }

    return mappedList;
  }

  Value interpretLambdaReduce(const std::shared_ptr<List>& list,
                              CallStackFrame& frame) {
    next(frame);  // Skip "("

    BooleanExpressionBuilder booleanExpression;
    Token termToken = current(frame);
    Value accumulator = interpretTerm(termToken, booleanExpression, frame);
    if (current(frame).getType() != TokenType::LAMBDA) {
      next(frame);
    }
    if (current(frame).getType() == TokenType::COMMA) {
      next(frame);
    }

    Method lambda = InterpHelper::getLambda(frame);

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(frame),
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
        throw BuiltinUnexpectedArgumentError(current(frame),
                                             SpecializedBuiltins.Reduce);
      }
    }

    std::shared_ptr<List> mappedList = std::make_shared<List>();
    size_t index = 0;

    for (const auto& item : list->elements) {
      frame.variables[accumulatorName] = accumulator;
      if (hasIndexVariable) {
        frame.variables[indexVariableName] = item;
      }

      CallStackFrame loopFrame(lambda.getCode());
      for (const auto& pair : frame.variables) {
        loopFrame.variables[pair.first] = pair.second;
      }
      callStack.push(loopFrame);
      interpretStackFrame();

      if (!callStack.empty()) {
        Value value = callStack.top().returnValue;
        frame.clearFlag(FrameFlags::ReturnFlag);
        accumulator = value;
      }
      index++;
    }

    return accumulator;
  }

  Value interpretLambdaSelect(const std::shared_ptr<List>& list,
                              CallStackFrame& frame) {
    next(frame);  // Skip "("

    Method lambda = InterpHelper::getLambda(frame);

    if (!lambda.isFlagSet(MethodFlags::Lambda)) {
      throw InvalidOperationError(
          current(frame),
          "Expected a lambda in `" + SpecializedBuiltins.Select + "` builtin.");
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
        throw BuiltinUnexpectedArgumentError(current(frame),
                                             SpecializedBuiltins.Select);
      }
    }

    std::shared_ptr<List> filteredList = std::make_shared<List>();

    size_t index = 0;
    for (const auto& item : list->elements) {
      frame.variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame.variables[indexVariableName] = static_cast<int>(index);
      }

      CallStackFrame loopFrame(lambda.getCode());
      for (const auto& pair : frame.variables) {
        loopFrame.variables[pair.first] = pair.second;
      }
      callStack.push(loopFrame);
      interpretStackFrame();

      if (!callStack.empty()) {
        Value value = callStack.top().returnValue;
        frame.clearFlag(FrameFlags::ReturnFlag);

        if (std::holds_alternative<bool>(value) && std::get<bool>(value)) {
          filteredList->elements.push_back(item);
        }
      }
      index++;
    }

    return filteredList;
  }

  Value interpretObjectToHash(const std::shared_ptr<Object>& object,
                              CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(frame),
                        "Expected open-parenthesis, `(`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(frame);  // Skip "("

    if (current(frame).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(frame),
                        "Expected close-parenthesis, `)`, in builtin `" +
                            SpecializedBuiltins.ToH + "`.");
    }
    next(frame);

    std::shared_ptr<Hash> hash = std::make_shared<Hash>();
    Class clazz = classes[object->className];
    for (const auto& pair : object->instanceVariables) {
      if (clazz.hasPrivateVariable(pair.first)) {
        continue;
      }

      hash->kvp[pair.first] = pair.second;
    }

    return hash;
  }

  Value interpretSpecializedObjectBuiltin(const std::string& builtin,
                                          const Value& value,
                                          CallStackFrame& frame) {
    if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
      throw InvalidOperationError(
          current(frame), "Specialized object builtin `" + builtin +
                              "` is illegal for type `" +
                              Serializer::get_value_type_string(value) + "`.");
    }

    std::shared_ptr<Object> object = std::get<std::shared_ptr<Object>>(value);

    if (builtin == SpecializedBuiltins.ToH) {
      return interpretObjectToHash(object, frame);
    }

    throw UnknownBuiltinError(current(frame), builtin);
  }

  Value interpretSpecializedListBuiltin(const std::string& builtin,
                                        const Value& value,
                                        CallStackFrame& frame) {
    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw InvalidOperationError(
          current(frame), "Specialized list builtin `" + builtin +
                              "` is illegal for type `" +
                              Serializer::get_value_type_string(value) + "`.");
    }

    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(value);

    if (builtin == SpecializedBuiltins.Select) {
      return interpretLambdaSelect(list, frame);
    } else if (builtin == SpecializedBuiltins.Map) {
      return interpretLambdaMap(list, frame);
    } else if (builtin == SpecializedBuiltins.Reduce) {
      return interpretLambdaReduce(list, frame);
    }

    throw UnknownBuiltinError(current(frame), builtin);
  }

  Value interpretDotNotation(Value& value, CallStackFrame& frame) {
    if (peek(frame).getType() == TokenType::DOT) {
      next(frame);
    }
    if (current(frame).getType() == TokenType::DOT) {
      next(frame);
    }
    Token tokenTerm = current(frame);
    std::string op = tokenTerm.getText();
    next(frame);

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      auto object = std::get<std::shared_ptr<Object>>(value);
      Class clazz = classes[object->className];

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

    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(frame),
                        "Expected open-parenthesis, `(`, to invoke builtin or "
                        "method using dot-notation.");
    }

    if (SpecializedBuiltins.is_builtin(op)) {
      if (op == SpecializedBuiltins.ToH) {
        return interpretSpecializedObjectBuiltin(op, value, frame);
      } else {
        return interpretSpecializedListBuiltin(op, value, frame);
      }
    }

    auto args = interpretArguments(frame);

    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
      std::shared_ptr<Object> object = std::get<std::shared_ptr<Object>>(value);
      return interpretInstanceMethodInvocation(object, op, args, frame);
    }

    return BuiltinInterpreter::execute(current(frame), op, value, args);
  }

  Value interpretExpression(BooleanExpressionBuilder& booleanExpression,
                            CallStackFrame& frame) {
    Token tokenTerm = current(frame);
    std::string tokenText = tokenTerm.getText();

    Value value;
    bool valueSet = false;

    if (tokenText == Keywords.This) {
      if (!frame.inObjectContext()) {
        throw InvalidContextError(current(frame),
                                  "Invalid context for keyword `this`.");
      }

      if (peek(frame).getType() != TokenType::DOT) {
        value = frame.getObjectContext();
        valueSet = true;
      } else {
        next(frame);  // Skip to "."
        next(frame);  // Skip the "."

        tokenTerm = current(frame);
        tokenText = tokenTerm.getText();

        if (tokenTerm.getType() != TokenType::IDENTIFIER &&
            tokenTerm.getType() != TokenType::DECLVAR) {
          throw InvalidOperationError(
              current(frame), "Syntax error near `this`. Missing identifier.");
        }
      }
    }

    if (current(frame).getType() == TokenType::OPEN_BRACKET) {
      value = interpretBracketExpression(booleanExpression, frame);
      valueSet = true;
    } else if (current(frame).getType() == TokenType::OPEN_BRACE) {
      value = interpretHash(booleanExpression, frame);
      valueSet = true;
    }

    if (!valueSet) {
      interpretQualifiedIdentifier(tokenTerm, tokenText, frame);
    }

    bool methodFound = hasMethod(tokenText, frame),
         classFound = hasClass(tokenText);
    if (!valueSet && (methodFound || classFound)) {
      if (classFound) {
        if (peek(frame).getType() != TokenType::DOT) {
          throw SyntaxError(current(frame),
                            "Invalid syntax near `" + tokenText + "`");
        }
        interpretClassMethodInvocation(tokenText, frame);
      } else if (methodFound) {
        if (peek(frame).getType() == TokenType::COMMA ||
            peek(frame).getType() == TokenType::CLOSE_PAREN) {
          if (frame.hasAssignedLambda(tokenText)) {
            std::shared_ptr<LambdaRef> lambdaRef =
                std::make_shared<LambdaRef>(tokenText);
            value = lambdaRef;
            valueSet = true;
          } else {
            throw SyntaxError(current(frame), "Expected lambda reference.");
          }
        } else {
          interpretMethodInvocation(tokenText, frame);
        }
      }

      if (!valueSet && !callStack.empty()) {
        value = callStack.top().returnValue;
        frame.clearFlag(FrameFlags::ReturnFlag);
        valueSet = true;
      }
    } else if (!valueSet && KiwiBuiltins.is_builtin_method(tokenText)) {
      interpretBuiltin(tokenText, frame);
      value = frame.returnValue;
      valueSet = true;
    }

    Token lastTerm = tokenTerm;

    if (!valueSet) {
      value = interpretTerm(tokenTerm, booleanExpression, frame);
      valueSet = true;
    }

    while (peek(frame).getType() == TokenType::DOT) {
      value = interpretDotNotation(value, frame);
      valueSet = true;
    }

    if (peek(frame).getType() == TokenType::OPERATOR) {
      next(frame);
    }

    while (current(frame).getType() == TokenType::OPERATOR) {
      std::string op = current(frame).toString();
      next(frame);

      if (Operators.is_logical_operator(op)) {
        InterpHelper::ensureBooleanExpressionHasRoot(tokenTerm,
                                                     booleanExpression, value);
        interpretBooleanExpression(tokenTerm, booleanExpression, op, frame);
        value = booleanExpression.evaluate();
        break;
      }

      lastTerm = current(frame);
      Value nextTerm =
          interpretTerm(tokenTerm, booleanExpression, frame, false);

      if (Operators.is_arithmetic_operator(op)) {
        InterpHelper::interpretArithmeticExpression(tokenTerm, op, value,
                                                    nextTerm);
        std::string peekNext = peek(frame).getText();
        if ((current(frame).getType() == TokenType::LITERAL ||
             current(frame).getType() == TokenType::IDENTIFIER) &&
            (Operators.is_relational_operator(peekNext) ||
             Operators.is_logical_operator(peekNext))) {
          next(frame);
        }

        if (peekNext == Symbols.CloseParenthesis ||
            Operators.is_arithmetic_operator(peekNext)) {
          next(frame);
        }

        if (current(frame).getType() == TokenType::CLOSE_PAREN) {
          if (peek(frame).getType() == TokenType::OPERATOR) {
            next(frame);
          }
        }
      }

      if (Operators.is_relational_operator(op)) {
        InterpHelper::interpretRelationalExpression(
            tokenTerm, booleanExpression, op, value, nextTerm);
      }

      if (Operators.is_bitwise_operator(op)) {
        InterpHelper::interpretBitwiseExpression(tokenTerm, op, value,
                                                 nextTerm);
      }
    }

    return value;
  }

  Value interpretSimpleValueType(CallStackFrame& frame) {
    if (current(frame).getValueType() == ValueType::Boolean) {
      return current(frame).toBoolean();
    } else if (current(frame).getValueType() == ValueType::Double) {
      return current(frame).toDouble();
    } else if (current(frame).getValueType() == ValueType::Integer) {
      return current(frame).toInteger();
    } else if (current(frame).getValueType() == ValueType::String) {
      return interpolateString(frame);
    }

    throw ConversionError(current(frame));
  }

  Value interpretSelfInvocationTerm(CallStackFrame& frame) {
    if (!frame.inObjectContext()) {
      throw InvalidContextError(current(frame),
                                "Invalid context for keyword `this`.");
    }

    if (peek(frame).getType() == TokenType::DOT) {
      next(frame);
    }

    if (current(frame).getType() != TokenType::DOT) {
      return frame.getObjectContext();
    }
    next(frame);  // Skip the "."

    if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);  // Skip the "@"
    }

    if (current(frame).getType() != TokenType::IDENTIFIER) {
      throw InvalidOperationError(
          current(frame), "Syntax error near `this`. Missing identifier.");
    }
    std::string identifier = current(frame).getText();
    next(frame);  // Skip the identifier

    Class clazz = classes[frame.getObjectContext()->className];

    if (clazz.hasMethod(identifier)) {
      interpretInstanceMethodInvocation(frame.getObjectContext()->identifier,
                                        frame);

      if (!callStack.empty()) {
        return callStack.top().returnValue;
      }
    } else if (frame.getObjectContext()->instanceVariables.find(identifier) !=
               frame.getObjectContext()->instanceVariables.end()) {
      return frame.getObjectContext()->instanceVariables[identifier];
    }

    throw UnimplementedMethodError(current(frame), clazz.getClassName(),
                                   identifier);
  }

  Value interpretTerm(Token& termToken,
                      BooleanExpressionBuilder& booleanExpression,
                      CallStackFrame& frame, bool skipOnRetrieval = true) {
    if (current(frame).getType() == TokenType::DECLVAR) {
      next(frame);
    }

    termToken = current(frame);

    if (termToken.getType() == TokenType::OPERATOR &&
        termToken.getText() == Operators.Subtract) {
      next(frame);  // Move past the minus sign
      Value nextValue =
          interpretTerm(termToken, booleanExpression, frame, false);

      // Apply negation if the next value is a number
      if (std::holds_alternative<int>(nextValue)) {
        return -std::get<int>(nextValue);
      } else if (std::holds_alternative<double>(nextValue)) {
        return -std::get<double>(nextValue);
      } else {
        throw SyntaxError(termToken,
                          "Unary minus applied to a non-numeric value.");
      }
    }

    if (termToken.getType() == TokenType::OPEN_BRACE) {
      std::shared_ptr<Hash> hash = interpretHash(booleanExpression, frame);
      return hash;
    }

    if (termToken.getType() == TokenType::IDENTIFIER &&
        peek(frame).getType() == TokenType::OPEN_BRACKET) {
      std::string variableName = termToken.getText();
      next(frame);  // Skip the identifier.
      return interpretSlice(frame, variableName);
    }

    if (current(frame).getType() == TokenType::OPEN_PAREN) {
      next(frame);  // Skip the '('
      Value result = interpretExpression(booleanExpression, frame);
      if (current(frame).getType() == TokenType::CLOSE_PAREN) {
        if (peek(frame).getType() == TokenType::OPERATOR) {
          next(frame);
        }
      }
      return result;
    } else if (current(frame).getType() == TokenType::OPEN_BRACKET) {
      Value result = interpretExpression(booleanExpression, frame);
      if (current(frame).getType() == TokenType::CLOSE_BRACKET) {
        if (peek(frame).getType() == TokenType::OPERATOR) {
          next(frame);
        }
      }
      return result;
    } else if (current(frame).getType() == TokenType::IDENTIFIER) {
      std::string identifier = current(frame).toString();
      if (hasVariable(identifier, frame)) {
        if (skipOnRetrieval) {
          if (peek(frame).getType() == TokenType::OPERATOR) {
            next(frame);
          }
        }
        return getVariable(identifier, frame);
      } else {
        throw UnknownIdentifierError(current(frame), identifier);
      }
    } else if (current(frame).getType() == TokenType::OPERATOR) {
      std::string op = current(frame).toString();
      next(frame);
      if (op == Operators.BitwiseNot) {
        Value bitwiseResult = interpretExpression(booleanExpression, frame);
        bitwiseResult =
            std::visit(BitwiseNotVisitor(current(frame)), bitwiseResult);
        return bitwiseResult;
      } else {
        // We can assume it's a boolean expression.
        interpretBooleanExpression(termToken, booleanExpression, op, frame);
        return booleanExpression.evaluate();
      }
    } else if (current(frame).getType() == TokenType::KEYWORD &&
               current(frame).getText() == Keywords.This) {
      return interpretSelfInvocationTerm(frame);
    } else {
      return interpretSimpleValueType(frame);
    }

    return 0;  // Placeholder for unsupported types
  }

  std::string interpolateObject(Value& value, CallStackFrame& frame) {
    std::shared_ptr<Object> object = std::get<std::shared_ptr<Object>>(value);
    Class clazz = classes[object->className];

    if (!clazz.hasMethod(KiwiBuiltins.ToS)) {
      return Serializer::basic_serialize_object(object);
    }

    Method toString = clazz.getMethod(KiwiBuiltins.ToS);
    std::vector<Value> parameters;
    Value returnValue = interpretInstanceMethodInvocation(
        object, KiwiBuiltins.ToS, parameters, frame);

    // Should probably check that an overridden to_s() actually returns a string.
    return Serializer::serialize(returnValue);
  }

  Value interpolateString(CallStackFrame& frame, std::string& input) {
    if (input[0] == '@') {
      std::string name = input.substr(1);
      if (hasVariable(name, frame)) {
        return getVariable(name, frame);
      }
    }

    Token tempToken = Token::createEmpty();
    std::string tempId = InterpHelper::getTemporaryId();
    std::vector<Token> tempAssignment =
        InterpHelper::getTemporaryAssignment(tempToken, tempId);
    Lexer lexer("", input);
    for (Token t : lexer.getAllTokens()) {
      tempAssignment.push_back(t);
    }

    std::string line;
    for (Token t : tempAssignment) {
      line += t.getText();
    }

    CallStackFrame tempFrame(tempAssignment);
    if (frame.inObjectContext()) {
      tempFrame.setObjectContext(frame.getObjectContext());
    }
    interpretAssignment(tempFrame, true);

    if (tempFrame.variables.find(tempId) == tempFrame.variables.end()) {
      throw SyntaxError(current(frame),
                        "Invalid string interpolation: `" + input + "`");
    }

    Value value = getVariable(tempId, tempFrame);

    tempAssignment.clear();

    return value;
  }

  std::string interpolateString(CallStackFrame& frame) {
    std::ostringstream sv;
    std::string input = current(frame).getText();
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
                  current(frame),
                  "Invalid syntax in string interpolation: `" + input + "`");
            }

            if (!builder.empty()) {
              Value interpolatedValue = interpolateString(frame, builder);
              if (!std::holds_alternative<std::shared_ptr<Object>>(
                      interpolatedValue)) {
                sv << Serializer::serialize(interpolatedValue);
              } else {
                sv << interpolateObject(interpolatedValue, frame);
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

  void interpretLambdaAssignment(const std::string& name, std::string op,
                                 CallStackFrame& frame) {
    if (op != Operators.Assign) {
      throw InvalidOperationError(
          current(frame), "Expected assignment operator in lambda assignment.");
    }

    Method lambda = InterpHelper::interpretLambda(frame);
    lambda.setName(name);
    frame.assignLambda(name, lambda);
  }

  void interpretAssignment(std::string& name, std::string& op,
                           CallStackFrame& frame, bool isTemporary = false) {
    if (current(frame).getType() == TokenType::LAMBDA) {
      interpretLambdaAssignment(name, op, frame);
      return;
    }

    BooleanExpressionBuilder booleanExpression;
    Value value = interpretExpression(booleanExpression, frame);

    if (current(frame).getType() == TokenType::DOT) {
      value = interpretDotNotation(value, frame);
    }

    if (op == Operators.Assign) {
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }
      if (!isTemporary && frame.inObjectContext()) {
        frame.getObjectContext()->instanceVariables[name] = value;
      } else {
        if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
          std::shared_ptr<Object> object =
              std::get<std::shared_ptr<Object>>(value);
          object->identifier = name;
          value = object;
        }
        frame.variables[name] = value;
      }
      Token nextToken = peek(frame);
      if (nextToken.getType() == TokenType::CLOSE_PAREN ||
          nextToken.getType() == TokenType::CLOSE_BRACKET) {
        next(frame);
      }
      return;
    }

    if (!hasVariable(name, frame)) {
      throw VariableUndefinedError(current(frame), name);
    }

    if (op == Operators.ListAppend) {
      interpretAppendToList(frame, value, name);
      return;
    }

    Value currentValue = getVariable(name, frame);
    frame.variables[name] =
        InterpHelper::interpretAssignOp(op, currentValue, value, frame);
  }
};

#endif
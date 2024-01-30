#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <map>
#include <stack>
#include <variant>
#include "errors/error.h"
#include "errors/handler.h"
#include "logging/logger.h"
#include "math/boolexpr.h"
#include "math/visitor.h"
#include "math/rng.h"
#include "objects/conditional.h"
#include "objects/loops/while.h"
#include "objects/method.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "typing/valuetype.h"
#include "globals.h"
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

    return interpret();
  }

 private:
  Logger& logger;
  std::map<std::string, std::vector<std::string>> files;
  std::map<std::string, Method> methods;
  std::vector<Token> _tokens;
  bool _errorState = false;
  bool _caught = false;
  std::string _parentPath;
  std::stack<CallStackFrame> callStack;

  int interpret() {
    if (callStack.empty()) {
      throw std::runtime_error("Call stack is empty");
    }

    while (!callStack.empty()) {
      auto& frame = callStack.top();
      while (frame.position < frame.tokens.size()) {
        try {
          const Token& token = current(frame);
          interpretToken(token, frame);

          // Move to the next token
          if (current(frame).getType() != TokenType::KEYWORD) {
            frame.position++;
          }
        } catch (const KiwiError& e) {
          _errorState = true;
          std::vector<std::string> lines = files[e.getToken().getFile()];
          // callStack.pop(); // WIP: unwind callstack
          return ErrorHandler::handleError(e, lines);
        } catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
          _errorState = true;
          // callStack.pop(); // WIP: unwind callstack
          exit(1);
        }
      }

      if (!callStack.empty()) {
        if (callStack.size() < 2) {
          // There needs to be at least two frames to update the next frame
          return 0;
        }

        auto topVariables = frame.variables;
        callStack.pop();
        CallStackFrame& nextFrame = callStack.top();

        for (const auto& var : topVariables) {
          std::string varName = var.first;
          if (shouldUpdateFrameVariables(varName, nextFrame)) {
            std::variant<int, double, bool, std::string> varValue = var.second;
            nextFrame.variables[varName] = varValue;
          }
        }
      }
    }

    return 0;
  }

  bool shouldUpdateFrameVariables(const std::string& varName,
                                  CallStackFrame& nextFrame) {
    return nextFrame.variables.find(varName) != nextFrame.variables.end();
  }

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
      //
    }
  }

  Token current() {
    if (callStack.empty()) {
      throw std::runtime_error("Call stack is empty");
    }
    CallStackFrame& topFrame = callStack.top();
    if (topFrame.position >= topFrame.tokens.size()) {
      return Token::createEndOfFrame();
    }
    return topFrame.tokens[topFrame.position];
  }

  Token next() {
    if (callStack.empty()) {
      throw std::runtime_error("Call stack is empty");
    }
    CallStackFrame& topFrame = callStack.top();
    topFrame.position++;
    if (topFrame.position >= topFrame.tokens.size()) {
      return Token::createEndOfFrame();
    }
    return topFrame.tokens[topFrame.position];
  }

  Token peek(CallStackFrame& frame) {
    size_t nextPosition = frame.position + 1;
    if (nextPosition < frame.tokens.size()) {
      return frame.tokens[nextPosition];
    } else {
      return Token::createEndOfFrame();
    }
  }

  std::string getTemporaryId() {
    std::string tempId = "temporary_" + RNG::getInstance().random16();
    return tempId;
  }

  std::vector<Token> getTemporaryAssignment(Token& tokenTerm,
                                            const std::string& tempId) {
    std::vector<Token> tokens;
    std::string file = tokenTerm.getFile();
    tokens.push_back(
        Token::create(TokenType::KEYWORD, file, Symbols.DeclVar, 0, 0));
    tokens.push_back(Token::create(TokenType::IDENTIFIER, file, tempId, 0, 0));
    tokens.push_back(
        Token::create(TokenType::OPERATOR, file, Operators.Assign, 0, 0));

    return tokens;
  }

  void interpretWhileLoop(CallStackFrame& frame) {
    WhileLoop loop;
    while (current(frame).getText() != Keywords.Do) {
      Token t = current(frame);
      loop.addConditionToken(t);
      next(frame);
    }

    next(frame); // Skip "do"
    Token tokenTerm = current(frame);

    int loops = 1;
    while (loops != 0) {
      if (Keywords.is_required_end_keyword(current(frame).getText())) {
        ++loops;
      } else if (current(frame).getText() == Keywords.End) {
        --loops;

        // Stop here.
        if (loops == 0) {
          next(frame);
          continue;
        }
      }

      Token t = current(frame);
      loop.addToken(t);
      next(frame);
    }

    CallStackFrame oldFrame = frame;
    std::string tempId = getTemporaryId();
    std::vector<Token> tempAssignment =
        getTemporaryAssignment(tokenTerm, tempId);
    // Interpret the condition.
    while (true) {
      std::vector<Token> condition = loop.getCondition();
      auto it = condition.begin() + 0;
      condition.insert(it, tempAssignment.begin(), tempAssignment.end());
      condition.push_back(Token::createNoOp());

      CallStackFrame conditionFrame(condition);
      for (const auto& pair : frame.variables) {
        conditionFrame.variables[pair.first] = pair.second;
      }

      callStack.push(conditionFrame);
      interpretAssignment(conditionFrame);

      if (conditionFrame.variables.find(tempId) ==
          conditionFrame.variables.end()) {
        throw SyntaxError(current(frame));
      }

      std::variant<int, double, bool, std::string> value =
          getVariable(tempId, conditionFrame);
      ValueType vt = get_value_type(value);

      if (vt != ValueType::Boolean) {
        throw ConversionError(current(frame));
      }

      // Stop here.
      if (!std::get<bool>(value)) {
        break;
      }

      std::vector<Token> code = loop.getCode();
      CallStackFrame codeFrame(code);
      for (const auto& pair : frame.variables) {
        codeFrame.variables[pair.first] = pair.second;
      }

      callStack.push(codeFrame);

      // Interpret the loop code.
      interpretMethod();
      frame = callStack.top();
    }

    for (const auto& pair : frame.variables) {
      if (shouldUpdateFrameVariables(pair.first, oldFrame)) {
        oldFrame.variables[pair.first] = pair.second;
      }
    }

    frame = oldFrame;
  }

  void interpretLoop(CallStackFrame& frame) {
    std::string loop = current(frame).getText();
    next(frame); // Skip "while"|"for"

    if (loop == Keywords.While) {
      interpretWhileLoop(frame);
    }
  }

  void interpretKeyword(const Token& token, CallStackFrame& frame) {
    const std::string& keyword = token.getText();
    if (keyword == Keywords.If) {
      interpretConditional(frame);
    } else if (keyword == Symbols.DeclVar) {
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
    } else {
      throw UnrecognizedTokenError(token);
    }
  }

  void interpretIdentifier(CallStackFrame& frame) {
    Token token = current(frame);
    std::string tokenText = token.getText();

    if (hasMethod(tokenText)) {
      interpretMethodInvocation(tokenText, frame);
    } else if (hasVariable(tokenText, frame)) {
      // Skip it.
    } else {
      throw UnknownIdentifierError(token, tokenText);
    }
  }

  void interpretToken(const Token& token, CallStackFrame& frame) {
    switch (token.getType()) {
      case TokenType::COMMENT:
      case TokenType::COMMA:
      case TokenType::NOOP:
        break;

      case TokenType::KEYWORD:
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

  void interpretMethod() {
    auto& frame = callStack.top();
    while (frame.position < frame.tokens.size()) {
      auto& token = frame.tokens[frame.position];
      interpretToken(token, frame);

      // WIP: clean this up.
      if (frame.position < frame.tokens.size()) {
        token = frame.tokens[frame.position];
      }

      // WIP: clean this up.
      if ((token.getType() != TokenType::KEYWORD &&
           token.getType() != TokenType::CONDITIONAL) ||
          token.getText() == Keywords.End) {
        if (methods.find(token.getText()) != methods.end()) {
          continue;
        }
        ++frame.position;
      }

      if (frame.isReturnFlagSet()) {
        if (callStack.size() > 1) {
          // Handle the return value in the caller frame
          auto returnValue = frame.returnValue;
          auto topVariables = frame.variables;
          callStack.pop(); // Pop the current frame
          auto& callerFrame = callStack.top();
          callerFrame.returnValue = returnValue;
          if (callerFrame.isSubFrame()) {
            callerFrame.setReturnFlag();
          }
          updateVariablesInCallerFrame(topVariables, callerFrame);
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

    auto topVariables = frame.variables;
    callStack.pop();
    auto& callerFrame = callStack.top();
    updateVariablesInCallerFrame(topVariables, callerFrame);
  }

  void updateVariablesInCallerFrame(std::map<std::string, std::variant<int, double, bool, std::string>> variables, CallStackFrame& callerFrame) {
    for (const auto& var : variables) {
      std::string varName = var.first;
      if (shouldUpdateFrameVariables(varName, callerFrame)) {
        std::variant<int, double, bool, std::string> varValue = var.second;
        callerFrame.variables[varName] = varValue;
      }
    }
  }

  bool hasMethod(const std::string& name) {
    return methods.find(name) != methods.end();
  }

  bool hasVariable(const std::string& name, CallStackFrame& frame) {
    if (frame.variables.find(name) != frame.variables.end()) {
      return true; // Found in the current frame
    }

    // Check in outer frames
    std::stack<CallStackFrame> tempStack(callStack); // Copy the call stack
    while (!tempStack.empty()) {
      CallStackFrame& outerFrame = tempStack.top();
      if (outerFrame.variables.find(name) != outerFrame.variables.end()) {
        return true; // Found in an outer frame
      }
      tempStack.pop();
    }

    return false; // Not found in any scope
  }

  Method getMethod(const std::string& name, CallStackFrame& frame) {
    if (hasMethod(name)) {
      return methods[name];
    }

    throw MethodUndefinedError(current(frame), name);
  }

  std::variant<int, double, bool, std::string> getVariable(
      const std::string& name, CallStackFrame& frame) {
    // Check in the current frame
    if (frame.variables.find(name) != frame.variables.end()) {
      return frame.variables[name];
    }

    // Check in outer frames
    std::stack<CallStackFrame> tempStack(callStack); // Copy the call stack
    while (!tempStack.empty()) {
      CallStackFrame& outerFrame = tempStack.top();
      if (outerFrame.variables.find(name) != outerFrame.variables.end()) {
        return outerFrame.variables[name]; // Found in an outer frame
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(current(frame), name);
  }

  void interpretMethodInvocation(const std::string& name,
                                 CallStackFrame& frame) {
    next(frame); // Skip the name.
    Method method = getMethod(name, frame);

    Token tokenTerm = current(frame);
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(tokenTerm);
    }
    next(frame); // Skip "("

    // Interpret parameters.
    std::vector<Token> parameters = method.getParameters();
    int paramIndex = 0;

    bool closeParenthesisFound = false;
    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
        continue;
      }

      std::string paramName = parameters.at(paramIndex++).getText();
      BooleanExpressionBuilder booleanExpression;
      tokenTerm = current(frame);
      std::variant<int, double, bool, std::string> paramValue =
          interpretTerm(tokenTerm, booleanExpression, frame);
      if (peek(frame).getType() == TokenType::CLOSE_PAREN) {
        next(frame);
        closeParenthesisFound = true;
      }
      method.addParameterValue(paramName, paramValue);
      if (!closeParenthesisFound) {
        next(frame);
      }
    }
    next(frame); // Skip ")"

    CallStackFrame codeFrame(method.getCode());
    for (const auto& pair : frame.variables) {
      codeFrame.variables[pair.first] = pair.second;
    }

    // Check all parameters are passed.
    for (Token t : parameters) {
      std::string parameterName = t.getText();
      if (!method.hasParameter(parameterName)) {
        throw ParameterMissingError(tokenTerm, parameterName);
      } else {
        codeFrame.variables[parameterName] =
            method.getParameterValue(parameterName);
      }
    }
    codeFrame.setSubFrame();
    callStack.push(codeFrame);

    // Now interpret the method in its own context
    interpretMethod();
  }

  void interpretMethodParameters(Method& method, CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      Token tokenTerm = current(frame);
      throw SyntaxError(tokenTerm);
    }
    next(frame); // Skip "("

    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      Token parameterToken = current(frame);
      if (parameterToken.getType() == TokenType::IDENTIFIER) {
        method.addParameterToken(parameterToken);
      }
      next(frame);
    }

    next(frame); // Skip ")"
  }

  void interpretMethodDefinition(CallStackFrame& frame) {
    next(frame); // Skip "def"

    Method method;

    std::string name = current(frame).getText();
    method.setName(name);

    if (Keywords.is_keyword(name)) {
      Token tokenTerm = current(frame);
      throw IllegalNameError(tokenTerm, name);
    }

    next(frame); // Skip the name.
    interpretMethodParameters(method, frame);

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
      method.addToken(codeToken);
      next(frame);
    }

    if (current(frame).getText() == Keywords.End) {
      //next(frame);
    }

    methods[name] = method;
  }

  bool hasReturnValue(CallStackFrame& frame) {
    Token nextToken = peek(frame);
    TokenType tokenType = nextToken.getType();
    bool isLiteral = tokenType == TokenType::LITERAL;
    bool isString = tokenType == TokenType::STRING;
    bool isIdentifier = tokenType == TokenType::IDENTIFIER;
    bool isParenthesis = tokenType == TokenType::OPEN_PAREN;
    return isString || isLiteral || isIdentifier || isParenthesis;
  }

  void interpretReturn(CallStackFrame& frame) {
    bool hasValue = hasReturnValue(frame);
    next(frame); // Skip "return"

    std::variant<int, double, bool, std::string> returnValue;
    BooleanExpressionBuilder ifExpression;
    if (hasValue) {
      returnValue = interpretExpression(ifExpression, frame);
      if (ifExpression.isSet()) {
        returnValue = ifExpression.evaluate();
      }
    }

    frame.returnValue = returnValue;
    frame.setReturnFlag();
  }

  std::string interpretAssignment(CallStackFrame& frame) {
    std::string name;
    next(frame); // Skip the "@"

    if (current(frame).getType() == TokenType::IDENTIFIER &&
        current(frame).getValueType() == ValueType::String) {
      name = current(frame).toString();
      next(frame);

      if (current(frame).getType() == TokenType::OPERATOR &&
          current(frame).getValueType() == ValueType::String) {
        std::string op = current(frame).toString();
        next(frame);

        if (Operators.is_assignment_operator(op)) {
          handleAssignment(name, op, frame);
        } else {
          // logger.debug("Unknown operator `" + op + "`", "Interpreter::interpretAssignment");
        }
      } else {
        // logger.debug("Unimplemented token `" + current(frame).info() + "`", "Interpreter::interpretAssignment");
      }
    } else {
      // logger.debug("Unimplemented token `" + current(frame).info() + "`", "Interpreter::interpretAssignment");
    }

    return name;
  }

  void interpretConditional(CallStackFrame& frame) {
    if (current(frame).getText() != Keywords.If) {
      throw SyntaxError(current(frame));
    }

    next(frame); // Skip "if"

    Conditional conditional;
    int ifCount = 1;
    bool ifValue = false;
    bool hasTrueElseIfEvaluation =
        false; // An optimization to prevent unnecessary evaluation.
    std::string building = Keywords.If;

    // Eagerly evaluate the If conditions.
    BooleanExpressionBuilder ifExpression;
    std::variant<int, double, bool, std::string> value =
        interpretExpression(ifExpression, frame);
    if (ifExpression.isSet()) {
      value = ifExpression.evaluate();
    }

    if (get_value_type(value) == ValueType::Boolean) {
      ifValue = std::get<bool>(value);
      conditional.getIfStatement().setEvaluation(ifValue);
      next(frame);
    } else {
      throw ConversionError(current(frame));
    }

    while (ifCount > 0) {
      if (current(frame).getText() == Keywords.If) {
        ++ifCount;
      } else if (current(frame).getText() == Keywords.EndIf && ifCount > 0) {
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

          if (get_value_type(value) == ValueType::Boolean) {
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
      interpretMethod();
    }
  }

  void interpretImport(CallStackFrame& frame) {
    next(frame); // skip the "import"

    std::string scriptName = current(frame).getText() + ".kiwi";
    std::string scriptPath = FileIO::joinPath(_parentPath, scriptName);
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    next(frame);

    std::string content = FileIO::readFile(scriptPath);
    if (content.empty()) {
      return;
    }

    Lexer lexer(logger, scriptPath, content);
    files[scriptPath] = lexer.getLines();
    std::vector<Token> tokens = lexer.getAllTokens();
    CallStackFrame scriptFrame(tokens);
    for (const auto& pair : frame.variables) {
      scriptFrame.variables[pair.first] = pair.second;
    }

    callStack.push(scriptFrame);
    interpretMethod();
  }

  void interpretPrint(CallStackFrame& frame, bool printNewLine = false) {
    next(frame); // skip the "print"

    if (current(frame).getType() == TokenType::STRING &&
        current(frame).getValueType() == ValueType::String) {
      std::cout << evaluateString(frame);
    } else if (current(frame).getType() == TokenType::KEYWORD &&
               current(frame).getText() == Symbols.DeclVar) {
      next(frame);
      std::string name = current(frame).getText();

      if (!hasVariable(name, frame)) {
        throw KiwiError(current(frame), "Unknown term `" + name + "`");
      }

      BooleanExpressionBuilder booleanExpression;
      std::variant<int, double, bool, std::string> value =
          interpretExpression(booleanExpression, frame);
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }

      std::cout << get_value_string(value);
    } else {
      std::ostringstream error;
      error << "Not implemented `" << Keywords.Print << "`|`"
            << Keywords.PrintLn << "` for type `"
            << get_value_type_string(current(frame).getValueType()) << std::endl
            << "Value: `" << current(frame).getText() << "`";
      throw KiwiError(current(frame), error.str());
    }

    if (printNewLine) {
      std::cout << std::endl;
    }
  }

  void ensureBooleanExpressionHasRoot(
      Token& tokenTerm, BooleanExpressionBuilder& booleanExpression,
      std::variant<int, double, bool, std::string> value) {
    if (booleanExpression.isSet()) {
      return;
    }

    // We can't use non-boolean values in our expression.
    ValueType vt = get_value_type(value);
    if (vt != ValueType::Boolean) {
      throw ConversionError(tokenTerm);
    }

    bool booleanValue = std::get<bool>(value);
    booleanExpression.value(booleanValue);
  }

  void interpretBooleanExpression(Token& tokenTerm,
                                  BooleanExpressionBuilder& booleanExpression,
                                  std::string& op, CallStackFrame& frame) {
    std::variant<int, double, bool, std::string> nextTerm =
        interpretExpression(booleanExpression, frame);

    // Check if the last term is valueless.
    if (nextTerm.valueless_by_exception()) {
      throw SyntaxError(tokenTerm);
    }

    // We can't use non-boolean values in our expression.
    ValueType vt = get_value_type(nextTerm);
    if (vt != ValueType::Boolean) {
      throw ConversionError(tokenTerm);
    }

    bool nextTermValue = std::get<bool>(nextTerm);

    if (op == Operators.And) {
      booleanExpression.andOperation(
          std::make_shared<ValueNode>(nextTermValue));
    } else if (op == Operators.Or) {
      booleanExpression.orOperation(std::make_shared<ValueNode>(nextTermValue));
    } else if (op == Operators.Not) {
      ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, nextTerm);
      booleanExpression.notOperation();
    }
  }

  void interpretBitwiseExpression(
      Token& tokenTerm, std::string& op,
      std::variant<int, double, bool, std::string>& result,
      std::variant<int, double, bool, std::string> nextTerm) {
    if (op == Operators.BitwiseAnd) {
      result = std::visit(BitwiseAndVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.BitwiseOr) {
      result = std::visit(BitwiseOrVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.BitwiseXor) {
      result = std::visit(BitwiseXorVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.BitwiseLeftShift) {
      result = std::visit(BitwiseLeftShiftVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.BitwiseRightShift) {
      result =
          std::visit(BitwiseRightShiftVisitor(tokenTerm), result, nextTerm);
    }
  }

  void interpretRelationalExpression(
      Token& tokenTerm, BooleanExpressionBuilder& booleanExpression,
      std::string& op, std::variant<int, double, bool, std::string>& result,
      std::variant<int, double, bool, std::string> nextTerm) {
    if (op == Operators.Equal) {
      result = std::visit(EqualityVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.NotEqual) {
      result = std::visit(InequalityVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.LessThan) {
      result = std::visit(LessThanVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.LessThanOrEqual) {
      result = std::visit(LessThanOrEqualVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.GreaterThan) {
      result = std::visit(GreaterThanVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.GreaterThanOrEqual) {
      result =
          std::visit(GreaterThanOrEqualVisitor(tokenTerm), result, nextTerm);
    }

    ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, result);
  }

  void interpretArithmeticExpression(
      Token& tokenTerm, std::string& op,
      std::variant<int, double, bool, std::string>& result,
      std::variant<int, double, bool, std::string> nextTerm) {
    if (op == Operators.Add) {
      result = std::visit(AddVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.Subtract) {
      result = std::visit(SubtractVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.Multiply) {
      result = std::visit(MultiplyVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.Divide) {
      result = std::visit(DivideVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.Exponent) {
      result = std::visit(PowerVisitor(tokenTerm), result, nextTerm);
    } else if (op == Operators.Modulus) {
      result = std::visit(ModuloVisitor(tokenTerm), result, nextTerm);
    }
  }

  // WIP: expression interpreter
  std::variant<int, double, bool, std::string> interpretExpression(
      BooleanExpressionBuilder& booleanExpression, CallStackFrame& frame) {

    Token tokenTerm = current(frame);

    std::string tokenText = tokenTerm.getText();
    if (hasMethod(tokenText)) {
      interpretMethodInvocation(tokenText, frame);
      return frame.returnValue; // Assume returnValue is part of the frame
    }

    std::variant<int, double, bool, std::string> result =
        interpretTerm(tokenTerm, booleanExpression, frame);
    std::variant<int, double, bool, std::string> lastTerm;

    while (current(frame).getType() == TokenType::OPERATOR) {
      std::string op = current(frame).toString();
      next(frame);

      if (Operators.is_logical_operator(op)) {
        ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, result);
        interpretBooleanExpression(tokenTerm, booleanExpression, op, frame);
        result = booleanExpression.evaluate();
        break;
      }

      std::variant<int, double, bool, std::string> nextTerm =
          interpretTerm(tokenTerm, booleanExpression, frame, false);

      if (Operators.is_arithmetic_operator(op)) {
        interpretArithmeticExpression(tokenTerm, op, result, nextTerm);
        std::string peekNext = peek(frame).getText();
        if (current(frame).getType() == TokenType::LITERAL &&
            (Operators.is_relational_operator(peekNext) ||
             Operators.is_logical_operator(peekNext))) {
          next(frame);
        }

        if (peekNext == Symbols.CloseParenthesis ||
            Operators.is_arithmetic_operator(peekNext)) {
          next(frame);
        }
      }

      if (Operators.is_relational_operator(op)) {
        interpretRelationalExpression(tokenTerm, booleanExpression, op, result,
                                      nextTerm);
      }

      if (Operators.is_bitwise_operator(op)) {
        interpretBitwiseExpression(tokenTerm, op, result, nextTerm);
      }

      lastTerm = result;
    }

    return result;
  }

  std::variant<int, double, bool, std::string> interpretTerm(
      Token& termToken, BooleanExpressionBuilder& booleanExpression,
      CallStackFrame& frame, bool skipOnRetrieval = true) {
    if (current(frame).getText() == Symbols.DeclVar) {
      next(frame);
    }

    termToken = current(frame);

    if (current(frame).getType() == TokenType::OPEN_PAREN) {
      next(frame); // Skip the '('
      std::variant<int, double, bool, std::string> result =
          interpretExpression(booleanExpression, frame);
      if (current(frame).getType() == TokenType::CLOSE_PAREN) {
        Token nextToken = peek(frame);
        if (nextToken.getType() == TokenType::OPERATOR) {
          next(frame);
        }
      }
      return result;
    } else if (current(frame).getType() == TokenType::IDENTIFIER) {
      std::string variableName = current(frame).toString();
      if (hasVariable(variableName, frame)) {
        if (skipOnRetrieval) {
          Token nextToken = peek(frame);
          if (nextToken.getType() == TokenType::OPERATOR) {
            next(frame);
          }
        }
        return getVariable(variableName, frame);
      }
    } else if (current(frame).getType() == TokenType::OPERATOR) {
      std::string op = current(frame).toString();
      next(frame);
      if (op == Operators.BitwiseNot) {
        std::variant<int, double, bool, std::string> bitwiseResult =
            interpretExpression(booleanExpression, frame);
        bitwiseResult =
            std::visit(BitwiseNotVisitor(current(frame)), bitwiseResult);
        return bitwiseResult;
      } else {
        // We can assume it's a boolean expression.
        interpretBooleanExpression(termToken, booleanExpression, op, frame);
        return booleanExpression.evaluate();
      }
    } else if (current(frame).getValueType() == ValueType::Boolean) {
      return current(frame).toBoolean();
    } else if (current(frame).getValueType() == ValueType::Double) {
      return current(frame).toDouble();
    } else if (current(frame).getValueType() == ValueType::Integer) {
      return current(frame).toInteger();
    } else if (current(frame).getValueType() == ValueType::String) {
      return evaluateString(frame);
    }

    return 0; // Placeholder for unsupported types
  }

  std::string evaluateString(CallStackFrame& frame) {
    const std::string methodName = "evaluateString";
    std::string output;

    // TODO: an error should occur here.
    if (current(frame).getValueType() != ValueType::String) {
      return output;
    }

    std::string input = current(frame).toString();

    // Don't skip whitespace.
    Lexer lexer(logger, methodName, input, false);
    std::vector<Token> eval = lexer.getAllTokens();

    std::ostringstream string;
    int position = -1;
    int end = eval.size();

    while (position + 1 < end) {
      ++position;
      Token token = eval.at(position);

      if (token.getText() == Symbols.Interpolate && position + 1 < end &&
          eval.at(position + 1).getText() == Symbols.OpenCurlyBrace) {
        evaluateStringInterpolation(position, token, eval, string, frame);
        continue;
      } else if (token.getType() == TokenType::ESCAPED) {
        string << token.getText();
        continue;
      }

      string << token.getText();
    }

    output = string.str();

    return output;
  }

  void evaluateStringInterpolation(int& position, Token& token,
                                   std::vector<Token>& eval,
                                   std::ostringstream& string,
                                   CallStackFrame& frame) {
    position += 2; // Skip "${"

    token = eval.at(position);
    // loop to the end
    while (token.getText() != Symbols.CloseCurlyBrace) {
      if (token.getText() == Symbols.DeclVar) {
        token = eval.at(++position);

        if (hasVariable(token.getText(), frame)) {
          auto v = getVariable(token.getText(), frame);
          ValueType vt = get_value_type(v);

          if (vt == ValueType::Integer) {
            string << std::get<int>(v);
          } else if (vt == ValueType::Double) {
            string << std::get<double>(v);
          } else if (vt == ValueType::Boolean) {
            string << std::boolalpha << std::get<bool>(v);
          } else if (vt == ValueType::String) {
            string << std::get<std::string>(v);
          }
        } else {
          // logger.debug("Variable undefined: " + token.info(), "Interpreter::evaluateStringInterpolation");
        }
      } else {
        // logger.debug("Unhandled token: " + token.info(), "Interpreter::evaluateStringInterpolation");
      }

      token = eval.at(++position);
    }
  }

  void handleAssignment(std::string& name, std::string& op,
                        CallStackFrame& frame) {
    BooleanExpressionBuilder booleanExpression;
    std::variant<int, double, bool, std::string> value =
        interpretExpression(booleanExpression, frame);

    if (op == Operators.Assign) {
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }
      frame.variables[name] = value;
      Token nextToken = peek(frame);
      if (nextToken.getType() == TokenType::CLOSE_PAREN) {
        next(frame);
      }
      return;
    }

    if (!hasVariable(name, frame)) {
      throw VariableUndefinedError(current(frame), name);
    }

    std::variant<int, double, bool, std::string> currentValue =
        getVariable(name, frame);

    if (op == Operators.AddAssign) {
      currentValue =
          std::visit(AddVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.SubtractAssign) {
      currentValue =
          std::visit(SubtractVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.MultiplyAssign) {
      currentValue =
          std::visit(MultiplyVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.DivideAssign) {
      currentValue =
          std::visit(DivideVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.ExponentAssign) {
      currentValue =
          std::visit(PowerVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.ModuloAssign) {
      currentValue =
          std::visit(ModuloVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseAndAssign) {
      currentValue =
          std::visit(BitwiseAndVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseOrAssign) {
      currentValue =
          std::visit(BitwiseOrVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseXorAssign) {
      currentValue =
          std::visit(BitwiseXorVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      currentValue = std::visit(BitwiseLeftShiftVisitor(current(frame)),
                                currentValue, value);
    } else if (op == Operators.BitwiseRightShiftAssign) {
      currentValue = std::visit(BitwiseRightShiftVisitor(current(frame)),
                                currentValue, value);
    } else if (op == Operators.BitwiseNotAssign) {
      currentValue = std::visit(BitwiseNotVisitor(current(frame)), value);
    }

    frame.variables[name] = currentValue;
  }
};

#endif
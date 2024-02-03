#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <map>
#include <stack>
#include "errors/error.h"
#include "errors/handler.h"
#include "logging/logger.h"
#include "math/boolexpr.h"
#include "math/visitor.h"
#include "math/rng.h"
#include "objects/conditional.h"
#include "objects/method.h"
#include "objects/module.h"
#include "objects/sliceindex.h"
#include "parsing/builtins.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "typing/valuetype.h"
#include "globals.h"
#include "interp_builtin.h"
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
    return 0;
  }

 private:
  Logger& logger;
  std::map<std::string, std::vector<std::string>> files;
  std::map<std::string, Method> methods;
  std::map<std::string, Module> modules;
  std::vector<Token> _tokens;
  bool _errorState = false;
  bool _caught = false;
  std::string _parentPath;
  std::stack<CallStackFrame> callStack;
  std::stack<std::string> moduleStack;

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
        if (modules.find(token.getText()) != modules.end()) {
          continue;
        }
        if (peek(frame).getType() == TokenType::OPEN_PAREN ||
            peek(frame).getType() == TokenType::QUALIFIER) {
          continue;
        }

        if (!frame.isReturnFlagSet()) {
          ++frame.position;
        }
      }

      if (frame.isBreakSet() || frame.isContinueSet()) {
        if (callStack.size() > 1) {
          // Pop and propagate to the top frame to be used in the next iteration.
          bool loopBreak = frame.loopBreak;
          bool loopContinue = frame.loopContinue;
          auto topVariables = frame.variables;
          callStack.pop();
          auto& callerFrame = callStack.top();
          updateVariablesInCallerFrame(topVariables, callerFrame);
          callerFrame.loopBreak = loopBreak;
          callerFrame.loopContinue = loopContinue;
          return;
        }
      }

      if (frame.isReturnFlagSet()) {
        if (callStack.size() > 1) {
          // Handle the return value in the caller frame
          auto returnValue = frame.returnValue;
          auto topVariables = frame.variables;
          callStack.pop();  // Pop the current frame
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

    Value returnValue = frame.returnValue;
    auto topVariables = frame.variables;
    callStack.pop();
    auto& callerFrame = callStack.top();
    updateVariablesInCallerFrame(topVariables, callerFrame);
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

  void collectLoopBodyTokens(std::vector<Token>& tokens,
                             CallStackFrame& frame) {
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

      tokens.push_back(current(frame));
      next(frame);
    }
  }

  void interpretForLoop(CallStackFrame& frame) {
    std::string itemVariableName, indexVariableName;
    bool hasIndexVariable = false;

    // Check if the loop includes an index variable.
    if (current(frame).getText() == Symbols.DeclVar) {
      next(frame);
      itemVariableName = current(frame).getText();
      next(frame);  // Skip the item variable name

      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);  // Skip ','
        if (current(frame).getText() == Symbols.DeclVar) {
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

    // Ensure the collection variable is declared.

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
    collectLoopBodyTokens(loopTokens, frame);

    // Execute the loop
    size_t index = 0;
    for (const auto& item : collection->elements) {
      if (frame.loopBreak) {
        break;
      }

      if (frame.loopContinue) {
        frame.clearContinue();
        continue;
      }

      // Update loop variables
      frame.variables[itemVariableName] = item;
      if (hasIndexVariable) {
        frame.variables[indexVariableName] = static_cast<int>(index);
      }

      // Execute loop body
      CallStackFrame loopFrame(loopTokens);
      for (const auto& pair : frame.variables) {
        loopFrame.variables[pair.first] = pair.second;
      }
      callStack.push(loopFrame);
      interpretStackFrame();

      // Handle break, continue, etc., if you implement them

      index++;
    }
    frame.clearBreak();
    frame.clearContinue();
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
    collectLoopBodyTokens(loopTokens, frame);

    CallStackFrame oldFrame = frame;
    std::string tempId = getTemporaryId();
    std::vector<Token> tempAssignment =
        getTemporaryAssignment(tokenTerm, tempId);
    // Interpret the condition.
    while (true) {
      if (frame.loopBreak) {
        break;
      }

      if (frame.loopContinue) {
        frame.clearContinue();
        continue;
      }

      std::vector<Token> condition = conditionTokens;
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
    frame.clearBreak();
    frame.clearContinue();

    for (const auto& pair : frame.variables) {
      if (shouldUpdateFrameVariables(pair.first, oldFrame)) {
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
    } else if (keyword == Keywords.Module) {
      interpretModuleDefinition(frame);
    } else if (keyword == Keywords.Break) {
      frame.setBreak();
    } else if (keyword == Keywords.Next) {
      frame.setContinue();
    } else {
      throw UnrecognizedTokenError(token);
    }
  }

  void interpretQualifiedIdentifier(Token& token, std::string& tokenText,
                                    CallStackFrame& frame) {
    if (peek(frame).getType() == TokenType::QUALIFIER) {
      if (!hasModule(tokenText)) {
        throw ModuleUndefinedError(token, tokenText);
      }
      next(frame);  // Skip the identifier.
      next(frame);  // Skip the qualifier.
      token = current(frame);
      tokenText = tokenText + Symbols.Qualifier + token.getText();
    }
  }

  void interpretIdentifier(CallStackFrame& frame) {
    Token token = current(frame);
    std::string tokenText = token.getText();

    interpretQualifiedIdentifier(token, tokenText, frame);

    if (hasMethod(tokenText)) {
      interpretMethodInvocation(tokenText, frame);
    } else if (hasVariable(tokenText, frame)) {
      // Skip it (for now).
    } else if (FileIOBuiltIns.is_builtin(tokenText)) {
      interpretBuiltin(tokenText, frame);
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

  void updateVariablesInCallerFrame(std::map<std::string, Value> variables,
                                    CallStackFrame& callerFrame) {
    for (const auto& var : variables) {
      std::string varName = var.first;
      if (shouldUpdateFrameVariables(varName, callerFrame)) {
        Value varValue = var.second;
        callerFrame.variables[varName] = varValue;
      }
    }
  }

  bool hasModule(const std::string& name) {
    return modules.find(name) != modules.end();
  }

  bool hasMethod(const std::string& name) {
    return methods.find(name) != methods.end();
  }

  bool hasVariable(const std::string& name, CallStackFrame& frame) {
    if (frame.variables.find(name) != frame.variables.end()) {
      return true;  // Found in the current frame
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

  Module getModule(const std::string& name, CallStackFrame& frame) {
    if (hasModule(name)) {
      return modules[name];
    }

    throw ModuleUndefinedError(current(frame), name);
  }

  Method getMethod(const std::string& name, CallStackFrame& frame) {
    if (hasMethod(name)) {
      return methods[name];
    }

    throw MethodUndefinedError(current(frame), name);
  }

  Value getVariable(const std::string& name, CallStackFrame& frame) {
    // Check in the current frame
    if (frame.variables.find(name) != frame.variables.end()) {
      return frame.variables[name];
    }

    // Check in outer frames
    std::stack<CallStackFrame> tempStack(callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      CallStackFrame& outerFrame = tempStack.top();
      if (outerFrame.variables.find(name) != outerFrame.variables.end()) {
        return outerFrame.variables[name];  // Found in an outer frame
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(current(frame), name);
  }

  void interpretModuleImport(const std::string& name, CallStackFrame& frame) {
    next(frame);  // Skip the name.
    moduleStack.push(name);
    Module module = getModule(name, frame);

    CallStackFrame codeFrame(module.getCode());
    callStack.push(codeFrame);
    interpretStackFrame();
    moduleStack.pop();
  }

  std::vector<Value> interpretParameters(CallStackFrame& frame) {
    std::vector<Value> parameters;

    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(current(frame));
    }
    next(frame);  // Skip "("

    bool closeParenthesisFound = false;
    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
        continue;
      }

      BooleanExpressionBuilder booleanExpression;
      Token subTokenTerm = current(frame);
      Value paramValue = interpretTerm(subTokenTerm, booleanExpression, frame);
      if (peek(frame).getType() == TokenType::CLOSE_PAREN) {
        next(frame);
        closeParenthesisFound = true;
      }

      parameters.push_back(paramValue);

      if (!closeParenthesisFound) {
        next(frame);
      }
    }
    next(frame);  // Skip ")"
    return parameters;
  }

  void interpretBuiltin(const std::string& name, CallStackFrame& frame) {
    Token tokenTerm = current(frame);
    next(frame);  // Skip the name.

    auto parameters = interpretParameters(frame);

    frame.returnValue =
        BuiltinInterpreter::execute(tokenTerm, name, parameters);
  }

  void interpretMethodInvocation(const std::string& name,
                                 CallStackFrame& frame) {
    next(frame);  // Skip the name.
    Method method = getMethod(name, frame);

    Token tokenTerm = current(frame);
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(tokenTerm);
    }
    next(frame);  // Skip "("

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
      Value paramValue = interpretTerm(tokenTerm, booleanExpression, frame);
      if (peek(frame).getType() == TokenType::CLOSE_PAREN) {
        next(frame);
        closeParenthesisFound = true;
      }
      method.addParameterValue(paramName, paramValue);
      if (!closeParenthesisFound) {
        next(frame);
      }
    }
    next(frame);  // Skip ")"

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
    interpretStackFrame();
  }

  void interpretMethodParameters(Method& method, CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      Token tokenTerm = current(frame);
      throw SyntaxError(tokenTerm);
    }
    next(frame);  // Skip "("

    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      Token parameterToken = current(frame);
      if (parameterToken.getType() == TokenType::IDENTIFIER) {
        method.addParameterToken(parameterToken);
      }
      next(frame);
    }

    next(frame);  // Skip ")"
  }

  void interpretMethodDefinition(CallStackFrame& frame) {
    next(frame);  // Skip "def"

    Method method;

    std::string moduleName;
    if (!moduleStack.empty()) {
      moduleName = moduleStack.top();
    }

    std::string name = current(frame).getText();

    if (Keywords.is_keyword(name)) {
      Token tokenTerm = current(frame);
      throw IllegalNameError(tokenTerm, name);
    }

    if (!moduleName.empty()) {
      name = moduleName + Symbols.Qualifier + name;
    }

    method.setName(name);

    next(frame);  // Skip the name.
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
    const Token nextToken = peek(frame);
    const TokenType tokenType = nextToken.getType();
    bool isLiteral = tokenType == TokenType::LITERAL;
    bool isString = tokenType == TokenType::STRING;
    bool isIdentifier = tokenType == TokenType::IDENTIFIER;
    bool isParenthesis = tokenType == TokenType::OPEN_PAREN;
    bool isVariable = tokenType == TokenType::KEYWORD &&
                      nextToken.getText() == Symbols.DeclVar;
    return isString || isLiteral || isIdentifier || isParenthesis || isVariable;
  }

  void interpretReturn(CallStackFrame& frame) {
    bool hasValue = hasReturnValue(frame);
    next(frame);  // Skip "return"

    Value returnValue;
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
    next(frame);  // Skip the "@"

    if (current(frame).getType() == TokenType::IDENTIFIER) {
      name = current(frame).toString();
      next(frame);

      // WIP: add other conditions to exclude.
      if (Keywords.is_keyword(name)) {
        throw IllegalNameError(current(frame), name);
      }

      if (current(frame).getType() == TokenType::OPERATOR) {
        std::string op = current(frame).toString();
        next(frame);

        if (Operators.is_assignment_operator(op) ||
            op == Operators.ListAppend) {
          handleAssignment(name, op, frame);
        }
      } else if (current(frame).getType() == TokenType::OPEN_BRACKET) {
        // Look ahead to determine if it's a slice assignment or a simple list access.
        size_t lookAheadPos = frame.position;
        bool isSliceAssignment = false;
        Token lookAheadToken = frame.tokens[lookAheadPos];
        while (lookAheadPos < frame.tokens.size()) {
          if (lookAheadToken.getType() == TokenType::COLON ||
              (lookAheadToken.getType() == TokenType::OPERATOR)) {
            isSliceAssignment = true;
            break;
          }
          ++lookAheadPos;
          lookAheadToken = frame.tokens[lookAheadPos];
        }

        if (isSliceAssignment) {
          interpretSliceAssignment(frame, name);
          return name;
        }
      }
    }

    return name;
  }

  void interpretSliceAssignment(CallStackFrame& frame,
                                const std::string& listVariableName) {
    Value listValue = getVariable(listVariableName, frame);
    if (!std::holds_alternative<std::shared_ptr<List>>(listValue)) {
      throw InvalidOperationError(current(frame),
                                  "`" + listVariableName + "` is not a list.");
    }

    // Parse slice parameters (start:stop:step)
    SliceIndex slice = interpretSliceIndex(frame, listValue);

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
    Value targetListValue = getVariable(listVariableName, frame);
    auto targetListPtr = std::get<std::shared_ptr<List>>(targetListValue);

    auto rhsList = convert_value_to_list(rhsValues);
    updateListSlice(frame, insertOp, targetListPtr, slice, rhsList);
  }

  // Update the slice of a list with new values.
  void updateListSlice(CallStackFrame& frame, bool insertOp,
                       std::shared_ptr<List>& targetList,
                       const SliceIndex& slice,
                       const std::shared_ptr<List>& rhsValues) {
    if (!std::holds_alternative<int>(slice.indexOrStart)) {
      throw IndexError(current(frame), "Start index must be an integer.");
    } else if (!std::holds_alternative<int>(slice.stopIndex)) {
      throw IndexError(current(frame), "Stop index must be an integer.");
    } else if (!std::holds_alternative<int>(slice.stepValue)) {
      throw IndexError(current(frame), "Step value must be an integer.");
    }

    int start = std::get<int>(slice.indexOrStart);
    int stop = std::get<int>(slice.stopIndex);
    int step = std::get<int>(slice.stepValue);

    if (!slice.isSlice && insertOp) {
      // This is a single element assignment.
      stop = start;
    }

    // Convert negative indices and adjust ranges
    int listSize = static_cast<int>(targetList->elements.size());
    int rhsSize = static_cast<int>(rhsValues->elements.size());
    if (start < 0) {
      start += listSize;
    }
    if (stop < 0) {
      stop += listSize;
    }
    if (start < 0) {
      start = 0;
    }
    if (stop > listSize) {
      stop = listSize;
    }
    if (step < 0 && stop == listSize) {
      stop = -1;  // Special case for reverse slicing
    }

    if (step == 1) {
      // Simple case: step is 1
      auto& elems = targetList->elements;
      if (start >= stop) {
        // Insert or delete elements
        elems.erase(elems.begin() + start, elems.begin() + stop);
        elems.insert(elems.begin() + start, rhsValues->elements.begin(),
                     rhsValues->elements.end());
      } else {
        // Replace subrange of elements
        std::copy(rhsValues->elements.begin(), rhsValues->elements.end(),
                  elems.begin() + start);
      }
    } else {
      // Complex case: step != 1
      int rhsIndex = 0;
      for (int i = start; i != stop && rhsIndex < rhsSize; i += step) {
        if ((step > 0 && i < listSize) || (step < 0 && i >= 0)) {
          targetList->elements[i] = rhsValues->elements[rhsIndex++];
        } else {
          break;  // Avoid going out of bounds
        }
      }
    }
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

  Value interpretSlice(CallStackFrame& frame,
                       const std::string& listVariableName) {
    Value listValue = getVariable(listVariableName, frame);
    if (!std::holds_alternative<std::shared_ptr<List>>(listValue)) {
      throw InvalidOperationError(current(frame),
                                  "`" + listVariableName + "` is not a list.");
    }

    SliceIndex slice = interpretSliceIndex(frame, listValue);
    auto listPtr = std::get<std::shared_ptr<List>>(listValue);

    if (slice.isSlice) {
      if (!std::holds_alternative<int>(slice.indexOrStart)) {
        throw IndexError(current(frame), "Start index must be an integer.");
      } else if (!std::holds_alternative<int>(slice.stopIndex)) {
        throw IndexError(current(frame), "Stop index must be an integer.");
      } else if (!std::holds_alternative<int>(slice.stepValue)) {
        throw IndexError(current(frame), "Step value must be an integer.");
      }

      int start = std::get<int>(slice.indexOrStart),
          stop = std::get<int>(slice.stopIndex),
          step = std::get<int>(slice.stepValue);
      // Adjust negative indices
      int listSize = static_cast<int>(listPtr->elements.size());
      start = (start < 0) ? std::max(start + listSize, 0) : start;
      stop = (stop < 0) ? stop + listSize : std::min(stop, listSize);
      if (step < 0 && stop == listSize)
        stop = -1;  // Adjust stop for reverse slicing

      auto slicedList = std::make_shared<List>();
      if (step < 0) {
        for (int i = (start == 0 ? listSize - 1 : start); i >= stop;
             i += step) {
          if (i < 0 || i >= listSize)
            break;  // Prevent out-of-bounds access
          slicedList->elements.push_back(listPtr->elements[i]);
        }
      } else {
        for (int i = start; i < stop; i += step) {
          if (i >= listSize)
            break;  // Prevent out-of-bounds access
          slicedList->elements.push_back(listPtr->elements[i]);
        }
      }
      return slicedList;  // Return the sliced list as a Value
    } else {
      // Single index access
      if (!std::holds_alternative<int>(slice.indexOrStart)) {
        throw IndexError(current(frame), "Index value must be an integer.");
      }
      int index = std::get<int>(slice.indexOrStart);
      int listSize = listPtr->elements.size();
      if (index < 0)
        index += listSize;  // Adjust for negative index
      if (index < 0 || index >= listSize) {
        throw RangeError(current(frame), "List index out of range.");
      }
      return listPtr->elements[index];
    }
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

    if (std::holds_alternative<std::shared_ptr<List>>(listValue)) {
      listPtr->elements.push_back(listValue);
    } else {
      BooleanExpressionBuilder booleanExpression;
      auto valueToAppend = interpretExpression(booleanExpression, frame);
      listPtr->elements.push_back(valueToAppend);
    }
  }

  std::shared_ptr<List> interpretList(
      BooleanExpressionBuilder& booleanExpression, CallStackFrame& frame) {
    std::shared_ptr<List> list = std::make_shared<List>();

    next(frame);  // Skip the "["

    while (current(frame).getType() != TokenType::CLOSE_BRACKET) {
      auto value = interpretExpression(booleanExpression, frame);
      list->elements.push_back(value);

      if (peek(frame).getType() == TokenType::COMMA) {
        next(frame);  // Skip current value
        if (current(frame).getType() == TokenType::COMMA) {
          next(frame);
        }
      } else if (current(frame).getType() == TokenType::COMMA) {
        next(frame);
      } else if (peek(frame).getType() == TokenType::CLOSE_BRACKET) {
        next(frame);
      }
    }

    next(frame);  // Skip the "]"

    return list;
  }

  void interpretConditional(CallStackFrame& frame) {
    if (current(frame).getText() != Keywords.If) {
      throw SyntaxError(current(frame));
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
      if (current(frame).getType() != TokenType::KEYWORD) {
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

    if (current(frame).getType() == TokenType::CLOSE_PAREN) {
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

    if (current(frame).getText() == Keywords.End) {
      //next(frame);
    }

    modules[name] = module;
  }

  void interpretExternalImport(std::string name, CallStackFrame& frame) {
    std::string scriptName = name + ".kiwi";
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
    interpretStackFrame();
  }

  void interpretImport(CallStackFrame& frame) {
    next(frame);  // skip the "import"

    std::string tokenText = current(frame).getText();
    if (hasModule(tokenText)) {
      interpretModuleImport(tokenText, frame);
    } else {
      interpretExternalImport(tokenText, frame);
    }
  }

  void interpretPrint(CallStackFrame& frame, bool printNewLine = false) {
    next(frame);  // skip the "print"

    if (current(frame).getType() == TokenType::STRING) {
      std::cout << evaluateString(frame, true);
    } else if (current(frame).getType() == TokenType::KEYWORD &&
               current(frame).getText() == Symbols.DeclVar) {
      next(frame);
      std::string name = current(frame).getText();

      if (!hasVariable(name, frame)) {
        throw KiwiError(current(frame), "Unknown term `" + name + "`");
      }

      BooleanExpressionBuilder booleanExpression;
      Value value = interpretExpression(booleanExpression, frame);
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }

      std::cout << get_value_string(value);
    } else if (current(frame).getType() == TokenType::OPEN_BRACKET) {
      BooleanExpressionBuilder booleanExpression;
      Value value = interpretList(booleanExpression, frame);
      if (current(frame).getType() == TokenType::DOT) {
        value = interpretDotNotation(value, frame);
      }
      std::cout << get_value_string(value);
    } else {
      throw ConversionError(current(frame));
    }

    if (printNewLine) {
      std::cout << std::endl;
    }
  }

  void ensureBooleanExpressionHasRoot(
      Token& tokenTerm, BooleanExpressionBuilder& booleanExpression,
      Value value) {
    if (booleanExpression.isSet()) {
      return;
    }

    // We can't use non-boolean values in our expression.
    if (!std::holds_alternative<bool>(value)) {
      throw ConversionError(tokenTerm);
    }

    bool booleanValue = std::get<bool>(value);
    booleanExpression.value(booleanValue);
  }

  void interpretBooleanExpression(Token& tokenTerm,
                                  BooleanExpressionBuilder& booleanExpression,
                                  std::string& op, CallStackFrame& frame) {
    Value nextTerm = interpretExpression(booleanExpression, frame);

    // Check if the last term is valueless.
    if (nextTerm.valueless_by_exception()) {
      throw SyntaxError(tokenTerm);
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
      ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, nextTerm);
      booleanExpression.notOperation();
    }
  }

  void interpretBitwiseExpression(Token& tokenTerm, std::string& op,
                                  Value& result, Value nextTerm) {
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
      std::string& op, Value& result, Value nextTerm) {
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

  void interpretArithmeticExpression(Token& tokenTerm, std::string& op,
                                     Value& result, Value nextTerm) {
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

  Value interpretDotNotation(Value& value, CallStackFrame& frame) {
    if (peek(frame).getType() == TokenType::DOT) {
      next(frame);
    }
    if (current(frame).getType() == TokenType::DOT) {
      next(frame);
    }
    std::string op = current(frame).getText();
    next(frame);
    auto parameters = interpretParameters(frame);

    return BuiltinInterpreter::execute(current(frame), op, value, parameters);
  }

  // WIP: expression interpreter
  Value interpretExpression(BooleanExpressionBuilder& booleanExpression,
                            CallStackFrame& frame) {
    Token tokenTerm = current(frame);
    std::string tokenText = tokenTerm.getText();

    if (current(frame).getType() == TokenType::OPEN_BRACKET) {
      return interpretList(booleanExpression, frame);
    }

    interpretQualifiedIdentifier(tokenTerm, tokenText, frame);

    if (hasMethod(tokenText)) {
      interpretMethodInvocation(tokenText, frame);
      return frame.returnValue;
    } else if (FileIOBuiltIns.is_builtin(tokenText)) {
      interpretBuiltin(tokenText, frame);
      return frame.returnValue;
    }

    Token lastTerm = tokenTerm;
    Value result = interpretTerm(tokenTerm, booleanExpression, frame);

    if (peek(frame).getType() == TokenType::OPERATOR) {
      next(frame);
    } else if (peek(frame).getType() == TokenType::DOT) {
      return interpretDotNotation(result, frame);
    }

    while (current(frame).getType() == TokenType::OPERATOR) {
      std::string op = current(frame).toString();
      next(frame);

      if (Operators.is_logical_operator(op)) {
        ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, result);
        interpretBooleanExpression(tokenTerm, booleanExpression, op, frame);
        result = booleanExpression.evaluate();
        break;
      }

      lastTerm = current(frame);
      Value nextTerm =
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
    }

    return result;
  }

  Value interpretValueType(CallStackFrame& frame) {
    if (current(frame).getValueType() == ValueType::Boolean) {
      return current(frame).toBoolean();
    } else if (current(frame).getValueType() == ValueType::Double) {
      return current(frame).toDouble();
    } else if (current(frame).getValueType() == ValueType::Integer) {
      return current(frame).toInteger();
    } else if (current(frame).getValueType() == ValueType::String) {
      return evaluateString(frame);
    }

    throw ConversionError(current(frame));
  }

  Value interpretTerm(Token& termToken,
                      BooleanExpressionBuilder& booleanExpression,
                      CallStackFrame& frame, bool skipOnRetrieval = true) {
    if (current(frame).getText() == Symbols.DeclVar) {
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
    } else if (current(frame).getType() == TokenType::IDENTIFIER) {
      std::string variableName = current(frame).toString();
      if (hasVariable(variableName, frame)) {
        if (skipOnRetrieval) {
          if (peek(frame).getType() == TokenType::OPERATOR) {
            next(frame);
          }
        }
        return getVariable(variableName, frame);
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
    } else if (peek(frame).getType() == TokenType::DOT) {
      Value value = interpretValueType(frame);
      return interpretDotNotation(value, frame);
    } else {
      return interpretValueType(frame);
    }

    return 0;  // Placeholder for unsupported types
  }

  std::string evaluateString(CallStackFrame& frame, bool handleDotNotation = false) {
    const std::string methodName = "evaluateString";
    std::string output;

    // TODO: an error should occur here.
    if (current(frame).getValueType() != ValueType::String) {
      return output;
    }

    std::string input = current(frame).toString();

    if (peek(frame).getType() == TokenType::DOT) {
      if (!handleDotNotation) {
        return input;
      }
      
      Value currentValue = input;
      auto value = interpretDotNotation(currentValue, frame);
      if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
      } else {
        return get_value_string(value);
      }
    }

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
    position += 2;  // Skip "${"

    token = eval.at(position);

    // WIP: fix this.
    while (token.getText() != Symbols.CloseCurlyBrace) {
      if (token.getText() == Symbols.DeclVar) {
        token = eval.at(++position);

        if (hasVariable(token.getText(), frame)) {
          auto v = getVariable(token.getText(), frame);
          string << get_value_string(v);
        }
      }

      token = eval.at(++position);
    }
  }

  void handleAssignment(std::string& name, std::string& op,
                        CallStackFrame& frame) {
    BooleanExpressionBuilder booleanExpression;
    Value value = interpretExpression(booleanExpression, frame);

    if (current(frame).getType() == TokenType::DOT) {
      value = interpretDotNotation(value, frame);
    }

    if (op == Operators.Assign) {
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }
      frame.variables[name] = value;
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
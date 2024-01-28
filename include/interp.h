#ifndef KIWI_INTERP_H
#define KIWI_INTERP_H

#include <map>
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
#include "typing/valuetype.h"
#include "globals.h"

class Interpreter {
  // WIP: this is the new interpreter.
 public:
  Interpreter(Logger& logger) : logger(logger), variables() {}

  int interpret(Lexer& lexer, std::string parentPath = "") {
    files[lexer.getFile()] = lexer.getLines();
    return interpret(lexer.getAllTokens(), parentPath);
  }

  int interpret(std::vector<Token> tokens, std::string parentPath = "") {
    _tokens = tokens;
    _position = 0;
    _end = _tokens.size();
    _parentPath = parentPath;

    if (_end == 0) {
      return 0;
    }

    return interpret();
  }

 private:
  Logger& logger;
  std::map<std::string, std::variant<int, double, bool, std::string>> variables;
  std::map<std::string, std::vector<std::string>> files;
  std::map<std::string, Method> methods;
  std::vector<Token> _tokens;
  int _position;
  int _end;
  bool _errorState = false;
  bool _caught = false;
  std::string _parentPath;
  std::variant<int, double, bool, std::string> returnValue;

  int interpret(int end = -1) {
    while ((end < 0 && _position < _end) || (end >= 0 && _position <= end)) {
      if (_position + 1 == _end || (end > 0 && _position == end)) {
        break;
      }

      // WIP: implement "try-catch"
      if (_errorState && !_caught) {
        exit(1);
      }

      try {
        // Don't parse comments.
        if (current().getType() == TokenType::COMMENT) {
          next();
          continue;
        }

        if (current().getType() == TokenType::KEYWORD) {
          if (current().getText() == Symbols.DeclVar) {
            interpretAssignment();
          } else if (Keywords.is_loop_keyword(current().getText())) {
            interpretLoop();
          } else if (current().getText() == Keywords.Method) {
            interpretMethodDefinition();
          } else if (current().getText() == Keywords.Return) {
            interpretReturn();
            next();
          } else {
            next();
          }
          continue;
        }

        if (current().getType() == TokenType::IDENTIFIER &&
            (current().getText() == Keywords.PrintLn ||
             current().getText() == Keywords.Print)) {
          interpretPrint(current().getText() == Keywords.PrintLn);
        } else if (current().getType() == TokenType::IDENTIFIER &&
                   current().getText() == Keywords.Import) {
          interpretImport();
          continue;
        } else if (current().getType() == TokenType::CONDITIONAL) {
          interpretConditional();
          continue;
        } else {
          std::string tokenText = current().getText();
          if (methods.find(tokenText) != methods.end()) {
            interpretMethodInvocation(tokenText);
          }
        }

        next();
      } catch (const KiwiError& e) {
        _errorState = true;
        std::vector<std::string> lines = files[e.getToken().getFile()];
        return ErrorHandler::handleError(e, lines);
      } catch (const std::exception& e) {
        _errorState = true;
        throw;
      }
    }

    return 0;
  }

  Token current() { return _tokens.at(_position); }

  Token next() {
    if (_position + 1 < _end) {
      ++_position;
    }

    return _tokens.at(_position);
  }

  Token peek() {
    if (_position + 1 < _end) {
      return _tokens.at(_position + 1);
    }

    return _tokens.at(_position);
  }

  std::string getTemporaryId() {
    std::string tempId = "temporary_" + RNG::getInstance().random16();
    return tempId;
  }

  std::vector<Token> getTemporaryAssignment(const std::string& tempId) {
    std::vector<Token> tokens;
    std::string file = current().getFile();
    tokens.push_back(Token::create(TokenType::KEYWORD, file, Symbols.DeclVar, 0, 0));
    tokens.push_back(Token::create(TokenType::IDENTIFIER, file, tempId, 0, 0));
    tokens.push_back(
        Token::create(TokenType::OPERATOR, file, Operators.Assign, 0, 0));

    return tokens;
  }

  void interpretWhileLoop() {
    WhileLoop loop;
    while (current().getText() != Keywords.Do) {
      Token t = current();
      loop.addConditionToken(t);
      next();
    }

    next();  // Skip "do"

    int loops = 1;
    while (loops != 0) {
      if (Keywords.is_required_end_keyword(current().getText())) {
        ++loops;
      } else if (current().getText() == Keywords.End) {
        --loops;

        // Stop here.
        if (loops == 0) {
          next();
          continue;
        }
      }

      Token t = current();
      loop.addToken(t);
      next();
    }

    std::string tempId = getTemporaryId();
    std::vector<Token> tempAssignment = getTemporaryAssignment(tempId);
    // Interpret the condition.
    while (true) {
      std::vector<Token> condition = loop.getCondition();
      auto it = condition.begin() + 0;
      condition.insert(it, tempAssignment.begin(), tempAssignment.end());
      condition.push_back(Token::createNoOp());

      injectTokens(condition);
      interpretAssignment();

      if (variables.find(tempId) == variables.end()) {
        throw SyntaxError(current());
      }

      std::variant<int, double, bool, std::string> value = variables[tempId];
      ValueType vt = get_value_type(value);

      if (vt != ValueType::Boolean) {
        throw ConversionError(current());
      }

      // Stop here.
      if (!std::get<bool>(value)) {
        break;
      }

      std::vector<Token> code = loop.getCode();
      injectTokens(code);

      // Interpret the loop code.
      interpret(_position + static_cast<int>(code.size()));
    }

    // Cleanup temporary variable.
    if (variables.find(tempId) != variables.end()) {
      variables.erase(tempId);
    }
  }

  void interpretLoop() {
    std::string loop = current().getText();
    next();  // Skip "while"|"for"

    if (loop == Keywords.While) {
      interpretWhileLoop();
    }
  }

  void interpretMethodInvocation(const std::string& name) {
    next(); // Skip the name.
    Method method = methods[name];

    Token tokenTerm = current();
    if (current().getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(tokenTerm);
    }
    next(); // Skip "("

    // Interpret parameters.
    std::vector<std::string> parametersPassed;
    std::vector<Token> parameters = method.getParameters();
    std::vector<Token> code = method.getCode();

    while (current().getType() != TokenType::CLOSE_PAREN) {
      std::string parameterName = interpretAssignment();
      parametersPassed.push_back(parameterName);
    }
    next(); // Skip ")"
    
    for (Token t : parameters) {
      std::string parameterName = t.getText();
      if (std::find(parametersPassed.begin(), parametersPassed.end(), parameterName) == parametersPassed.end()) {
        throw ParameterMissingError(tokenTerm, parameterName);
      }
    }

    injectTokens(code);
    int executePosition = _position + static_cast<int>(code.size());
    interpret(executePosition);

    // Cleanup.
  }

  void interpretMethodParameters(Method& method) {
    if (current().getType() != TokenType::OPEN_PAREN) {
      Token tokenTerm = current();
      throw SyntaxError(tokenTerm);
    }
    next(); // Skip "("

    while (current().getType() != TokenType::CLOSE_PAREN) {
      Token parameterToken = current();
      if (parameterToken.getType() == TokenType::IDENTIFIER) {
        method.addParameterToken(parameterToken);
      }
      next();
    }

    next(); // Skip ")"
  }

  void interpretMethodDefinition() {
    next(); // Skip "def"

    Method method;

    std::string name = current().getText();
    method.setName(name);

    if (Keywords.is_keyword(name)) {
      Token tokenTerm = current();
      throw IllegalNameError(tokenTerm, name);
    }

    next(); // Skip the name.
    interpretMethodParameters(method);

    int counter = 1;
    while (counter > 0) {
      if (current().getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          break;
        }
      }
      else if (Keywords.is_required_end_keyword(current().getText())) {
        ++counter;
      }

      Token codeToken = current();
      method.addToken(codeToken);
      next();
    }

    methods[name] = method;
  }

  void interpretReturn() {
    next(); // Skip "return"

    BooleanExpressionBuilder ifExpression;
    std::variant<int, double, bool, std::string> value =
        interpretExpression(ifExpression);
    if (ifExpression.isSet()) {
      value = ifExpression.evaluate();
    }

    returnValue = value;
  }

  std::string interpretAssignment() {
    std::string name;
    next();  // Skip the "@"

    if (current().getType() == TokenType::IDENTIFIER &&
        current().getValueType() == ValueType::String) {
      name = current().toString();
      next();

      if (current().getType() == TokenType::OPERATOR &&
          current().getValueType() == ValueType::String) {
        std::string op = current().toString();
        next();

        if (Operators.is_assignment_operator(op)) {
          handleAssignment(name, op);
        } else {
          // logger.debug("Unknown operator `" + op + "`", "Interpreter::interpretAssignment");
        }
      } else {
        // logger.debug("Unimplemented token `" + current().info() + "`", "Interpreter::interpretAssignment");
      }
    } else {
      // logger.debug("Unimplemented token `" + current().info() + "`", "Interpreter::interpretAssignment");
    }

    return name;
  }

  void interpretConditional() {
    if (current().getText() != Keywords.If) {
      throw SyntaxError(current());
    }

    next();  // Skip "if"

    Conditional conditional;
    int ifCount = 1;
    bool ifValue = false;
    bool hasTrueElseIfEvaluation =
        false;  // An optimization to prevent unnecessary evaluation.
    std::string building = Keywords.If;

    // Eagerly evaluate the If conditions.
    BooleanExpressionBuilder ifExpression;
    std::variant<int, double, bool, std::string> value =
        interpretExpression(ifExpression);
    if (ifExpression.isSet()) {
      value = ifExpression.evaluate();
    }

    if (get_value_type(value) == ValueType::Boolean) {
      ifValue = std::get<bool>(value);
      conditional.getIfStatement().setEvaluation(ifValue);
    } else {
      throw ConversionError(current());
    }

    while (ifCount > 0) {
      if (current().getText() == Keywords.If) {
        ++ifCount;
      } else if (current().getText() == Keywords.EndIf && ifCount > 0) {
        --ifCount;

        // Stop here.
        if (ifCount == 0) {
          next();
          continue;
        }
      } else if (current().getText() == Keywords.Else && ifCount == 1) {
        building = Keywords.Else;
        next();
      } else if (current().getText() == Keywords.ElseIf && ifCount == 1) {
        building = Keywords.ElseIf;
        next();

        conditional.addElseIfStatement();

        // No need to evaluate if the previous condition is true.
        if (!ifValue && !hasTrueElseIfEvaluation) {
          // Eagerly evaluate ElseIf conditions.
          BooleanExpressionBuilder elseIfExpression;
          value = interpretExpression(elseIfExpression);

          if (elseIfExpression.isSet()) {
            value = elseIfExpression.evaluate();
          }

          if (get_value_type(value) == ValueType::Boolean) {
            bool elseIfValue = std::get<bool>(value);
            if (elseIfValue) {
              hasTrueElseIfEvaluation = true;
            }
            conditional.getElseIfStatement().setEvaluation(elseIfValue);
            Token bodyToken = current();
            conditional.getElseIfStatement().addToken(bodyToken);
            next();
          } else {
            throw ConversionError(current());
          }
        }
        continue;
      }

      // Distribute tokens to be executed.
      Token bodyToken = current();
      if (building == Keywords.If) {
        conditional.getIfStatement().addToken(bodyToken);
      } else if (building == Keywords.ElseIf) {
        conditional.getElseIfStatement().addToken(bodyToken);
      } else if (building == Keywords.Else) {
        conditional.getElseStatement().addToken(bodyToken);
      }

      next();
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
    injectTokens(executableTokens);
    int executePosition = _position + static_cast<int>(executableTokens.size());
    interpret(executePosition);
  }

  void injectTokens(std::vector<Token>& executableTokens) {
    if (executableTokens.empty()) {
      return;
    }

    auto it = _tokens.begin() + _position;
    _tokens.insert(it, executableTokens.begin(), executableTokens.end());
    _end = _tokens.size();
  }

  void interpretImport() {
    next();  // skip the "import"

    std::string scriptName = current().getText() + ".kiwi";
    std::string scriptPath = FileIO::joinPath(_parentPath, scriptName);
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    next();

    std::string content = FileIO::readFile(scriptPath);
    if (content.empty()) {
      return;
    }

    Lexer lexer(logger, scriptPath, content);
    files[scriptPath] = lexer.getLines();
    std::vector<Token> tokens = lexer.getAllTokens();
    injectTokens(tokens);
  }

  void interpretPrint(bool printNewLine = false) {
    next();  // skip the "print"

    if (current().getType() == TokenType::STRING &&
        current().getValueType() == ValueType::String) {
      std::cout << evaluateString();
    } else if (current().getType() == TokenType::KEYWORD &&
               current().getText() == Symbols.DeclVar) {
      next();
      std::string name = current().getText();

      if (variables.find(name) == variables.end()) {
        throw KiwiError(current(), "Unknown term `" + name + "`");
      }

      BooleanExpressionBuilder booleanExpression;
      std::variant<int, double, bool, std::string> value =
          interpretExpression(booleanExpression);
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }

      std::cout << get_value_string(value);
    } else {
      std::ostringstream error;
      error << "Not implemented `" << Keywords.Print << "`|`"
            << Keywords.PrintLn << "` for type `"
            << get_value_type_string(current().getValueType()) << std::endl
            << "Value: `" << current().getText() << "`";
      throw KiwiError(current(), error.str());
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
                                  std::string& op) {
    std::variant<int, double, bool, std::string> nextTerm =
        interpretExpression(booleanExpression);

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
      BooleanExpressionBuilder& booleanExpression) {
    Token tokenTerm = current();

    std::string tokenText = tokenTerm.getText();
    if (methods.find(tokenText) != methods.end()) {
      interpretMethodInvocation(tokenText);
      return returnValue;
    }

    std::variant<int, double, bool, std::string> result =
        interpretTerm(tokenTerm, booleanExpression);
    std::variant<int, double, bool, std::string> lastTerm;

    while (current().getType() == TokenType::OPERATOR) {
      std::string op = current().toString();
      next();

      if (Operators.is_logical_operator(op)) {
        ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression, result);
        interpretBooleanExpression(tokenTerm, booleanExpression, op);
        result = booleanExpression.evaluate();
        break;
      }

      std::variant<int, double, bool, std::string> nextTerm =
          interpretTerm(tokenTerm, booleanExpression);

      if (Operators.is_arithmetic_operator(op)) {
        interpretArithmeticExpression(tokenTerm, op, result, nextTerm);
        std::string peekNext = peek().getText();
        if (current().getType() == TokenType::LITERAL &&
            (Operators.is_relational_operator(peekNext) ||
             Operators.is_logical_operator(peekNext))) {
          next();
        }
      }

      if (Operators.is_relational_operator(op)) {
        interpretRelationalExpression(tokenTerm, booleanExpression, op, result,
                                      nextTerm);
      }

      lastTerm = result;
    }

    return result;
  }

  std::variant<int, double, bool, std::string> interpretTerm(
      Token& termToken, BooleanExpressionBuilder& booleanExpression) {
    if (current().getText() == Symbols.DeclVar) {
      next();
    }

    termToken = current();

    if (current().getType() == TokenType::OPEN_PAREN) {
      next();  // Skip the '('
      std::variant<int, double, bool, std::string> result =
          interpretExpression(booleanExpression);
      next();  // Skip the ')'
      return result;
    } else if (current().getType() == TokenType::IDENTIFIER) {
      std::string variableName = current().toString();
      if (variables.find(variableName) != variables.end()) {
        next();
        return variables[variableName];
      }
    } else if (current().getType() == TokenType::OPERATOR) {
      std::string op = current().toString();
      next();
      interpretBooleanExpression(termToken, booleanExpression, op);
    } else if (current().getValueType() == ValueType::Boolean) {
      return current().toBoolean();
    } else if (current().getValueType() == ValueType::Double) {
      return current().toDouble();
    } else if (current().getValueType() == ValueType::Integer) {
      return current().toInteger();
    } else if (current().getValueType() == ValueType::String) {
      return evaluateString();
    }

    return 0;  // Placeholder for unsupported types
  }

  std::string evaluateString() {
    const std::string methodName = "evaluateString";
    std::string output;

    // TODO: an error should occur here.
    if (current().getValueType() != ValueType::String) {
      return output;
    }

    std::string input = current().toString();

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
        evaluateStringInterpolation(position, token, eval, string);
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
                                   std::ostringstream& string) {
    position += 2;  // Skip "${"

    token = eval.at(position);
    // loop to the end
    while (token.getText() != Symbols.CloseCurlyBrace) {
      if (token.getText() == Symbols.DeclVar) {
        token = eval.at(++position);

        if (variables.find(token.getText()) != variables.end()) {
          auto v = variables[token.getText()];
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

  void handleAssignment(std::string& name, std::string& op) {
    BooleanExpressionBuilder booleanExpression;
    std::variant<int, double, bool, std::string> value =
        interpretExpression(booleanExpression);

    if (op == Operators.Assign) {
      if (booleanExpression.isSet()) {
        value = booleanExpression.evaluate();
      }
      variables[name] = value;
      return;
    }

    if (variables.find(name) == variables.end()) {
      throw VariableUndefinedError(current(), name);
    }

    std::variant<int, double, bool, std::string> currentValue = variables[name];

    if (op == Operators.AddAssign) {
      currentValue = std::visit(AddVisitor(current()), currentValue, value);
    } else if (op == Operators.SubtractAssign) {
      currentValue =
          std::visit(SubtractVisitor(current()), currentValue, value);
    } else if (op == Operators.MultiplyAssign) {
      currentValue =
          std::visit(MultiplyVisitor(current()), currentValue, value);
    } else if (op == Operators.DivideAssign) {
      currentValue = std::visit(DivideVisitor(current()), currentValue, value);
    } else if (op == Operators.ExponentAssign) {
      currentValue = std::visit(PowerVisitor(current()), currentValue, value);
    } else if (op == Operators.ModuloAssign) {
      currentValue = std::visit(ModuloVisitor(current()), currentValue, value);
    }

    variables[name] = currentValue;
  }
};

#endif
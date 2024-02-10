#ifndef KIWI_INTERPHELPER_H
#define KIWI_INTERPHELPER_H

#include <vector>
#include "errors/error.h"
#include "math/boolexpr.h"
#include "math/visitor.h"
#include "objects/method.h"
#include "objects/sliceindex.h"
#include "parsing/keywords.h"
#include "parsing/lexer.h"
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "stackframe.h"

struct InterpHelper {
  static Token current(CallStackFrame& frame) {
    if (frame.position >= frame.tokens.size()) {
      return Token::createEndOfFrame();
    }
    return frame.tokens[frame.position];
  }

  static void next(CallStackFrame& frame) {
    if (frame.position < frame.tokens.size()) {
      frame.position++;
    } else {
      // To complete the frame, or not to complete the frame, that is the question;
    }
  }

  static Token peek(CallStackFrame& frame) {
    size_t nextPosition = frame.position + 1;
    if (nextPosition < frame.tokens.size()) {
      return frame.tokens[nextPosition];
    } else {
      return Token::createEndOfFrame();
    }
  }

  static void ensureBooleanExpressionHasRoot(
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

  static bool isSliceAssignmentExpression(CallStackFrame& frame) {
    size_t pos = frame.position;
    bool isSliceAssignment = false;
    Token token = frame.tokens[pos];
    while (pos < frame.tokens.size()) {
      if (token.getType() == TokenType::COLON ||
          token.getType() == TokenType::OPERATOR) {
        isSliceAssignment = true;
        break;
      }
      token = frame.tokens[++pos];
    }
    return isSliceAssignment;
  }

  static bool isListExpression(CallStackFrame& frame) {
    size_t position = frame.position;
    if (position >= frame.tokens.size() || frame.tokens[position].getType() != TokenType::OPEN_BRACKET) {
      return false;
    }

    int bracketCount = 1;
    ++position;

    while (position < frame.tokens.size() && bracketCount > 0) {
      Token token = frame.tokens[position];
      TokenType type = token.getType();

      if (type == TokenType::OPEN_BRACKET) {
        ++bracketCount;
      } else if (type == TokenType::CLOSE_BRACKET) {
        --bracketCount;
      } else if (type == TokenType::OPEN_BRACE) {
        int braceCount = 1;
        ++position; // Skip "["
        while (position < frame.tokens.size() && braceCount > 0) {
          token = frame.tokens[position];
          if (token.getType() == TokenType::OPEN_BRACE) {
            ++braceCount;
          } else if (token.getType() == TokenType::CLOSE_BRACE) {
            --braceCount;
          }
          ++position;
        }
        continue;
      } else if (type == TokenType::COLON || type == TokenType::RANGE) {
        return false;
      }

      ++position;
    }

    return bracketCount == 0;
  }

  static bool isRangeExpression(CallStackFrame& frame) {
    size_t pos = frame.position;
    bool isRange = false;
    Token token = frame.tokens[pos];
    while (pos < frame.tokens.size()) {
      if (token.getType() == TokenType::RANGE) {
        isRange = true;
        break;
      }
      token = frame.tokens[++pos];
    }
    return isRange;
  }

  static bool hasReturnValue(CallStackFrame& frame) {
    const Token nextToken = peek(frame);
    const TokenType tokenType = nextToken.getType();
    bool isLiteral = tokenType == TokenType::LITERAL;
    bool isString = tokenType == TokenType::STRING;
    bool isIdentifier = tokenType == TokenType::IDENTIFIER;
    bool isParenthesis = tokenType == TokenType::OPEN_PAREN;
    bool isVariable = tokenType == TokenType::DECLVAR;
    bool isBracketed = tokenType == TokenType::OPEN_BRACKET;
    bool isInstanceInvocation =
        tokenType == TokenType::KEYWORD && nextToken.getText() == Keywords.This;
    return isString || isLiteral || isIdentifier || isParenthesis ||
           isVariable || isBracketed || isInstanceInvocation;
  }

  static bool shouldUpdateFrameVariables(const std::string& varName,
                                         CallStackFrame& nextFrame) {
    return nextFrame.variables.find(varName) != nextFrame.variables.end();
  }

  static void updateVariablesInCallerFrame(
      std::map<std::string, Value> variables, CallStackFrame& callerFrame) {
    for (const auto& var : variables) {
      std::string varName = var.first;
      if (shouldUpdateFrameVariables(varName, callerFrame)) {
        Value varValue = var.second;
        callerFrame.variables[varName] = varValue;
      }
    }
  }

  static std::string getTemporaryId() {
    return "temporary_" + RNG::getInstance().random16();
  }

  static void collectBodyTokens(std::vector<Token>& tokens,
                                CallStackFrame& frame) {
    int counter = 1;
    while (counter != 0) {
      if (Keywords.is_required_end_keyword(current(frame).getText())) {
        ++counter;
      } else if (current(frame).getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(frame);
          continue;
        }
      }

      tokens.push_back(current(frame));
      next(frame);
    }
  }

  static std::vector<Token> getTemporaryAssignment(Token& tokenTerm,
                                                   const std::string& tempId) {
    std::vector<Token> tokens;
    std::string file = tokenTerm.getFile();
    tokens.push_back(
        Token::create(TokenType::DECLVAR, file, Keywords.DeclVar, 0, 0));
    tokens.push_back(Token::create(TokenType::IDENTIFIER, file, tempId, 0, 0));
    tokens.push_back(
        Token::create(TokenType::OPERATOR, file, Operators.Assign, 0, 0));

    return tokens;
  }

  static void updateListSlice(CallStackFrame& frame, bool insertOp,
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

  static void interpretRelationalExpression(
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

    InterpHelper::ensureBooleanExpressionHasRoot(tokenTerm, booleanExpression,
                                                 result);
  }

  static void interpretArithmeticExpression(Token& tokenTerm, std::string& op,
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

  static void interpretBitwiseExpression(Token& tokenTerm, std::string& op,
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

  static Value interpretAssignOp(const std::string& op, Value& currentValue,
                                 Value& value, CallStackFrame& frame) {
    if (op == Operators.AddAssign) {
      return std::visit(AddVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.SubtractAssign) {
      return std::visit(SubtractVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.MultiplyAssign) {
      return std::visit(MultiplyVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.DivideAssign) {
      return std::visit(DivideVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.ExponentAssign) {
      return std::visit(PowerVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.ModuloAssign) {
      return std::visit(ModuloVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseAndAssign) {
      return std::visit(BitwiseAndVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseOrAssign) {
      return std::visit(BitwiseOrVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseXorAssign) {
      return std::visit(BitwiseXorVisitor(current(frame)), currentValue, value);
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      return std::visit(BitwiseLeftShiftVisitor(current(frame)), currentValue,
                        value);
    } else if (op == Operators.BitwiseRightShiftAssign) {
      return std::visit(BitwiseRightShiftVisitor(current(frame)), currentValue,
                        value);
    } else if (op == Operators.BitwiseNotAssign) {
      return std::visit(BitwiseNotVisitor(current(frame)), value);
    }

    throw InvalidOperationError(current(frame),
                                "Invalid operator `" + op + "`");
  }

  static Value interpretListSlice(const SliceIndex& slice,
                                  const std::shared_ptr<List>& list,
                                  CallStackFrame& frame) {
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
      int listSize = static_cast<int>(list->elements.size());
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
          slicedList->elements.push_back(list->elements[i]);
        }
      } else {
        for (int i = start; i < stop; i += step) {
          if (i >= listSize)
            break;  // Prevent out-of-bounds access
          slicedList->elements.push_back(list->elements[i]);
        }
      }
      return slicedList;  // Return the sliced list as a Value
    } else {
      // Single index access
      if (!std::holds_alternative<int>(slice.indexOrStart)) {
        throw IndexError(current(frame), "Index value must be an integer.");
      }
      int index = std::get<int>(slice.indexOrStart);
      int listSize = list->elements.size();
      if (index < 0)
        index += listSize;  // Adjust for negative index
      if (index < 0 || index >= listSize) {
        throw RangeError(current(frame), "List index out of range.");
      }
      return list->elements[index];
    }
  }

  static void interpretParameterizedCatch(CallStackFrame& frame,
                                          std::string& errorVariableName,
                                          Value& errorValue) {
    next(frame);  // Skip "("

    if (current(frame).getType() != TokenType::DECLVAR) {
      throw SyntaxError(current(frame),
                        "Syntax error in catch variable declaration.");
    }
    next(frame);  // Skip "@"

    if (current(frame).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          current(frame),
          "Syntax error in catch variable declaration. Missing identifier.");
    }

    errorVariableName = current(frame).getText();
    next(frame);  // Skip the identifier.

    if (current(frame).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(frame),
                        "Syntax error in catch variable declaration.");
    }
    next(frame);  // Skip ")"

    errorValue = frame.getErrorMessage();
  }

  static std::string interpretModuleHome(std::string& modulePath,
                                         CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::STRING ||
        !Strings::begins_with(modulePath, "@")) {
      return "";
    }

    std::string moduleHome;

    // Get everything between the @ and the /, that is the home.
    Lexer lexer("", modulePath);
    const auto& tokens = lexer.getAllTokens();
    auto lastToken = Token::createEmpty();
    size_t pos = 0;
    bool build = false;
    std::string moduleName;

    while (pos < tokens.size()) {
      const auto& token = tokens.at(pos);

      // If the last token was "@"
      if (pos + 1 < tokens.size() &&
          lastToken.getType() == TokenType::DECLVAR) {
        if (tokens.at(pos + 1).getText() == Operators.Divide) {
          moduleHome = token.getText();
          pos += 2;  // Skip module home and "/"
          build = true;
          continue;
        }
      }

      if (build) {
        moduleName += token.getText();
      } else {
        lastToken = token;
      }
      ++pos;
    }

    if (!moduleName.empty()) {
      modulePath = moduleName;
    }

    return moduleHome;
  }

  static std::string interpretBaseClass(CallStackFrame& frame) {
    std::string baseClassName;
    if (current(frame).getType() == TokenType::OPERATOR) {
      if (current(frame).getText() != Operators.LessThan) {
        throw SyntaxError(
            current(frame),
            "Expected inheritance operator, `<`, in class definition.");
      }
      next(frame);

      if (current(frame).getType() != TokenType::IDENTIFIER) {
        throw SyntaxError(current(frame), "Expected base class name.");
      }

      baseClassName = current(frame).getText();
      next(frame);  // Skip base class.
    }
    return baseClassName;
  }

  static Method interpretMethodDeclaration(CallStackFrame& frame) {
    Method method;

    while (current(frame).getText() != Keywords.Method) {
      if (current(frame).getText() == Keywords.Abstract) {
        method.setFlag(MethodFlags::Abstract);
      } else if (current(frame).getText() == Keywords.Override) {
        method.setFlag(MethodFlags::Override);
      } else if (current(frame).getText() == Keywords.Private) {
        method.setFlag(MethodFlags::Private);
      } else if (current(frame).getText() == Keywords.Static) {
        method.setFlag(MethodFlags::Static);
      }
      next(frame);
    }
    next(frame);  // Skip "def"

    Token tokenTerm = current(frame);

    std::string name = current(frame).getText();
    method.setName(name);
    next(frame);  // Skip the name.
    interpretMethodParameters(method, frame);
    int counter = 1;

    if (method.isFlagSet(MethodFlags::Abstract)) {
      return method;
    }

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

      if (current(frame).getType() == TokenType::ENDOFFRAME) {
        throw SyntaxError(tokenTerm,
                          "Invalid method declaration `" + name + "`");
      }
    }

    return method;
  }

  static void interpretMethodParameters(Method& method, CallStackFrame& frame) {
    if (current(frame).getType() != TokenType::OPEN_PAREN) {
      Token tokenTerm = current(frame);
      throw SyntaxError(tokenTerm);
    }
    next(frame);  // Skip "("

    while (current(frame).getType() != TokenType::CLOSE_PAREN) {
      Token parameterToken = current(frame);
      if (parameterToken.getType() == TokenType::IDENTIFIER) {
        method.addParameterName(parameterToken.getText());
      }
      next(frame);
    }

    next(frame);  // Skip ")"
  }
};

#endif
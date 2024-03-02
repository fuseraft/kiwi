#ifndef KIWI_INTERPHELPER_H
#define KIWI_INTERPHELPER_H

#include <vector>
#include "errors/error.h"
#include "math/visitor.h"
#include "objects/method.h"
#include "objects/sliceindex.h"
#include "parsing/keywords.h"
#include "parsing/lexer.h"
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "stackframe.h"

struct InterpHelper {
  static Token current(std::shared_ptr<TokenStream> stream) {
    if (stream->position >= stream->tokens.size()) {
      return Token::createStreamEnd();
    }
    return stream->tokens.at(stream->position);
  }

  static void next(std::shared_ptr<TokenStream> stream) {
    if (stream->position < stream->tokens.size()) {
      stream->position++;
    }
  }

  static Token peek(std::shared_ptr<TokenStream> stream) {
    size_t nextPosition = stream->position + 1;
    if (nextPosition < stream->tokens.size()) {
      return stream->tokens[nextPosition];
    } else {
      return Token::createStreamEnd();
    }
  }

  static Method interpretLambda(std::shared_ptr<TokenStream> stream) {
    next(stream);  // Skip "lambda"
    Method lambda;
    lambda.setName(getTemporaryId());

    interpretMethodParameters(lambda, stream);

    if (current(stream).getText() != Keywords.Do) {
      throw SyntaxError(current(stream), "Expected `do` in lambda expression.");
    }
    next(stream);  // Skip "do"

    std::vector<Token> lambdaTokens;
    collectBodyTokens(lambdaTokens, stream);
    for (Token t : lambdaTokens) {
      lambda.addToken(t);
    }

    lambda.setFlag(MethodFlags::Lambda);

    return lambda;
  }

  static bool isSliceAssignmentExpression(std::shared_ptr<TokenStream> stream) {
    size_t pos = stream->position;
    bool isSliceAssignment = false;
    auto token = stream->tokens.at(pos);
    while (pos < stream->tokens.size()) {
      if (token.getType() == TokenType::COLON ||
          token.getType() == TokenType::OPERATOR) {
        isSliceAssignment = true;
        break;
      }
      token = stream->tokens.at(++pos);
    }
    return isSliceAssignment;
  }

  static bool isListExpression(std::shared_ptr<TokenStream> stream) {
    size_t position = stream->position + 1;  // Skip the "["
    int bracketCount = 1;

    while (position < stream->tokens.size() && bracketCount > 0) {
      Token token = stream->tokens.at(position);
      TokenType type = token.getType();

      if (type == TokenType::OPEN_BRACKET) {
        ++bracketCount;
      } else if (type == TokenType::CLOSE_BRACKET) {
        --bracketCount;
      } else if (type == TokenType::OPEN_BRACE) {
        int braceCount = 1;
        ++position;  // Skip "["
        while (position < stream->tokens.size() && braceCount > 0) {
          token = stream->tokens.at(position);
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

  static bool isRangeExpression(std::shared_ptr<TokenStream> stream) {
    size_t pos = stream->position + 1;  // Skip the "["
    size_t size = stream->tokens.size();
    bool isRange = false;
    auto token = stream->tokens.at(pos);
    int counter = 1;
    while (pos < size && counter > 0) {
      if (token.getType() == TokenType::OPEN_BRACKET) {
        ++counter;
      } else if (token.getType() == TokenType::CLOSE_BRACKET) {
        --counter;

        if (counter == 0) {
          break;
        }
      }

      if (token.getType() == TokenType::RANGE) {
        isRange = true;
        break;
      }

      token = stream->tokens.at(++pos);
    }
    return isRange;
  }

  static bool hasReturnValue(std::shared_ptr<TokenStream> stream) {
    const Token nextToken = peek(stream);
    const TokenType tokenType = nextToken.getType();
    bool isLiteral = tokenType == TokenType::LITERAL;
    bool isString = tokenType == TokenType::STRING;
    bool isIdentifier = tokenType == TokenType::IDENTIFIER;
    bool isParenthesis = tokenType == TokenType::OPEN_PAREN;
    bool isBraced = tokenType == TokenType::OPEN_BRACE;
    bool isVariable = tokenType == TokenType::DECLVAR;
    bool isBracketed = tokenType == TokenType::OPEN_BRACKET;
    bool isInstanceInvocation =
        tokenType == TokenType::KEYWORD && nextToken.getText() == Keywords.This;
    return isString || isLiteral || isIdentifier || isParenthesis ||
           isVariable || isBracketed || isInstanceInvocation || isBraced;
  }

  static bool shouldUpdateFrameVariables(
      const std::string& varName,
      const std::shared_ptr<CallStackFrame> nextFrame) {
    return nextFrame->variables.find(varName) != nextFrame->variables.end();
  }

  static void updateVariablesInCallerFrame(
      std::unordered_map<std::string, Value> variables,
      std::shared_ptr<CallStackFrame> callerFrame) {
    for (const auto& var : variables) {
      if (shouldUpdateFrameVariables(var.first, callerFrame)) {
        callerFrame->variables[var.first] = std::move(var.second);
      }
    }
  }

  static std::string getTemporaryId() {
    return "temporary_" + RNG::getInstance().random16();
  }

  static void collectBodyTokens(std::vector<Token>& tokens,
                                std::shared_ptr<TokenStream> stream) {
    int counter = 1;
    while (stream->canRead() && counter != 0) {
      if (Keywords.is_block_keyword(current(stream).getText())) {
        ++counter;
      } else if (current(stream).getText() == Keywords.End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          next(stream);
          continue;
        }
      }

      tokens.push_back(current(stream));
      next(stream);
    }
  }

  static std::vector<Token> getTemporaryAssignment(Token& tokenTerm,
                                                   const std::string& tempId) {
    std::vector<Token> tokens;
    auto file = tokenTerm.getFile();
    tokens.push_back(
        Token::create(TokenType::DECLVAR, file, Keywords.DeclVar, 0, 0));
    tokens.push_back(Token::create(TokenType::IDENTIFIER, file, tempId, 0, 0));
    tokens.push_back(
        Token::create(TokenType::OPERATOR, file, Operators.Assign, 0, 0));

    return tokens;
  }

  static void updateListSlice(std::shared_ptr<TokenStream> stream,
                              bool insertOp, std::shared_ptr<List>& targetList,
                              const SliceIndex& slice,
                              const std::shared_ptr<List>& rhsValues) {
    if (!std::holds_alternative<int>(slice.indexOrStart)) {
      throw IndexError(current(stream), "Start index must be an integer.");
    } else if (!std::holds_alternative<int>(slice.stopIndex)) {
      throw IndexError(current(stream), "Stop index must be an integer.");
    } else if (!std::holds_alternative<int>(slice.stepValue)) {
      throw IndexError(current(stream), "Step value must be an integer.");
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
        elems.erase(elems.begin() + start, elems.begin() + stop);
        elems.insert(elems.begin() + start, rhsValues->elements.begin(),
                     rhsValues->elements.end());
      } else {
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

  static Value interpretAssignOp(std::shared_ptr<TokenStream> stream,
                                 const std::string& op, Value& currentValue,
                                 Value& value) {
    if (op == Operators.AddAssign) {
      return std::visit(AddVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.SubtractAssign) {
      return std::visit(SubtractVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.MultiplyAssign) {
      return std::visit(MultiplyVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.DivideAssign) {
      return std::visit(DivideVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.ExponentAssign) {
      return std::visit(PowerVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.ModuloAssign) {
      return std::visit(ModuloVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.BitwiseAndAssign) {
      return std::visit(BitwiseAndVisitor(current(stream)), currentValue,
                        value);
    } else if (op == Operators.BitwiseOrAssign) {
      return std::visit(BitwiseOrVisitor(current(stream)), currentValue, value);
    } else if (op == Operators.BitwiseXorAssign) {
      return std::visit(BitwiseXorVisitor(current(stream)), currentValue,
                        value);
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      return std::visit(BitwiseLeftShiftVisitor(current(stream)), currentValue,
                        value);
    } else if (op == Operators.BitwiseRightShiftAssign) {
      return std::visit(BitwiseRightShiftVisitor(current(stream)), currentValue,
                        value);
    } else if (op == Operators.BitwiseNotAssign) {
      return std::visit(BitwiseNotVisitor(current(stream)), value);
    }

    throw InvalidOperationError(current(stream),
                                "Invalid operator `" + op + "`");
  }

  static Value interpretListSlice(std::shared_ptr<TokenStream> stream,
                                  const SliceIndex& slice,
                                  const std::shared_ptr<List>& list) {
    if (slice.isSlice) {
      if (!std::holds_alternative<int>(slice.indexOrStart)) {
        throw IndexError(current(stream), "Start index must be an integer.");
      } else if (!std::holds_alternative<int>(slice.stopIndex)) {
        throw IndexError(current(stream), "Stop index must be an integer.");
      } else if (!std::holds_alternative<int>(slice.stepValue)) {
        throw IndexError(current(stream), "Step value must be an integer.");
      }

      int start = std::get<int>(slice.indexOrStart),
          stop = std::get<int>(slice.stopIndex),
          step = std::get<int>(slice.stepValue);

      // Adjust negative indices
      int listSize = static_cast<int>(list->elements.size());
      start = (start < 0) ? std::max(start + listSize, 0) : start;
      stop = (stop < 0) ? stop + listSize : std::min(stop, listSize);

      // Adjust stop for reverse slicing
      if (step < 0 && stop == listSize) {
        stop = -1;
      }

      auto slicedList = std::make_shared<List>();

      if (step < 0) {
        for (int i = (start == 0 ? listSize - 1 : start); i >= stop;
             i += step) {
          // Prevent out-of-bounds access
          if (i < 0 || i >= listSize) {
            break;
          }
          slicedList->elements.push_back(list->elements[i]);
        }
      } else {
        for (int i = start; i < stop; i += step) {
          // Prevent out-of-bounds access
          if (i >= listSize) {
            break;
          }
          slicedList->elements.push_back(list->elements[i]);
        }
      }
      return slicedList;  // Return the sliced list as a Value
    } else {
      // Single index access
      if (!std::holds_alternative<int>(slice.indexOrStart)) {
        throw IndexError(current(stream), "Index value must be an integer.");
      }

      int index = std::get<int>(slice.indexOrStart);
      int listSize = list->elements.size();

      if (index < 0) {
        index += listSize;  // Adjust for negative index
      }

      if (index < 0 || index >= listSize) {
        throw RangeError(current(stream), "List index out of range.");
      }

      return list->elements[index];
    }
  }

  static void interpretParameterizedCatch(std::shared_ptr<TokenStream> stream,
                                          std::shared_ptr<CallStackFrame> frame,
                                          std::string& errorVariableName,
                                          Value& errorValue) {
    next(stream);  // Skip "("

    if (current(stream).getType() != TokenType::DECLVAR) {
      throw SyntaxError(current(stream),
                        "Syntax error in catch variable declaration.");
    }
    next(stream);  // Skip "@"

    if (current(stream).getType() != TokenType::IDENTIFIER) {
      throw SyntaxError(
          current(stream),
          "Syntax error in catch variable declaration. Missing identifier.");
    }

    errorVariableName = current(stream).getText();
    next(stream);  // Skip the identifier.

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(current(stream),
                        "Syntax error in catch variable declaration.");
    }
    next(stream);  // Skip ")"

    errorValue = frame->getErrorMessage();
  }

  static std::string interpretModuleHome(std::string& modulePath,
                                         std::shared_ptr<TokenStream> stream) {
    if (current(stream).getType() != TokenType::STRING ||
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

  static std::string interpretBaseClass(std::shared_ptr<TokenStream> stream) {
    std::string baseClassName;
    if (current(stream).getType() == TokenType::OPERATOR) {
      if (current(stream).getText() != Operators.LessThan) {
        throw SyntaxError(
            current(stream),
            "Expected inheritance operator, `<`, in class definition.");
      }
      next(stream);

      if (current(stream).getType() != TokenType::IDENTIFIER) {
        throw SyntaxError(current(stream), "Expected base class name.");
      }

      baseClassName = current(stream).getText();
      next(stream);  // Skip base class.
    }
    return baseClassName;
  }

  static Method interpretMethodDeclaration(
      std::shared_ptr<TokenStream> stream) {
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
    interpretMethodParameters(method, stream);
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

  static std::vector<std::string> getParameterSet(
      std::shared_ptr<TokenStream> stream) {
    auto tokenTerm = current(stream);
    if (current(stream).getType() != TokenType::OPEN_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected open-parenthesis, `(`, in parameter set expression.");
    }
    next(stream);  // Skip "("

    std::vector<std::string> paramSet;

    while (stream->canRead() &&
           current(stream).getType() != TokenType::CLOSE_PAREN) {
      auto parameterToken = current(stream);
      if (parameterToken.getType() == TokenType::IDENTIFIER) {
        paramSet.push_back(parameterToken.getText());
      }
      next(stream);
    }

    if (current(stream).getType() != TokenType::CLOSE_PAREN) {
      throw SyntaxError(
          tokenTerm,
          "Expected close-parenthesis, `)`, in parameter set expression.");
    }
    next(stream);  // Skip ")"
    return paramSet;
  }

  static void interpretMethodParameters(Method& method,
                                        std::shared_ptr<TokenStream> stream) {
    for (const auto& param : getParameterSet(stream)) {
      method.addParameterName(param);
    }
  }
};

#endif
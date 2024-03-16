#ifndef KIWI_INTERPHELPER_H
#define KIWI_INTERPHELPER_H

#include <vector>
#include "errors/error.h"
#include "math/visitor.h"
#include "objects/method.h"
#include "objects/sliceindex.h"
#include "parsing/keywords.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "util/string.h"
#include "stackframe.h"

struct InterpHelper {
  static bool isSliceAssignmentExpression(
      const std::shared_ptr<TokenStream>& stream) {
    size_t pos = stream->position;
    bool isSliceAssignment = false;

    const auto& tokens = stream->tokens;

    while (pos < tokens.size()) {
      if (tokens.at(pos).getType() == KTokenType::COLON ||
          tokens.at(pos).getType() == KTokenType::OPERATOR) {
        isSliceAssignment = true;
        break;
      }

      ++pos;
    }

    return isSliceAssignment;
  }

  static bool isListExpression(const std::shared_ptr<TokenStream>& stream) {
    size_t position = stream->position + 1;  // Skip the "["
    int bracketCount = 1;

    const auto& tokens = stream->tokens;
    const size_t tokensSize = tokens.size();

    while (position < tokensSize && bracketCount > 0) {
      const auto& token = tokens.at(position);
      const auto type = token.getType();

      if (type == KTokenType::OPEN_BRACKET) {
        ++bracketCount;
      } else if (type == KTokenType::CLOSE_BRACKET) {
        --bracketCount;
      } else if (type == KTokenType::OPEN_BRACE) {
        int braceCount = 1;
        ++position;  // Skip the current brace
        while (position < tokensSize && braceCount > 0) {
          const auto& innerToken = tokens.at(position);
          if (innerToken.getType() == KTokenType::OPEN_BRACE) {
            ++braceCount;
          } else if (innerToken.getType() == KTokenType::CLOSE_BRACE) {
            --braceCount;
          }
          ++position;
        }
        continue;
      } else if (type == KTokenType::COLON || type == KTokenType::RANGE) {
        return false;
      }

      ++position;
    }

    return bracketCount == 0;
  }

  static bool isRangeExpression(const std::shared_ptr<TokenStream>& stream) {
    size_t pos = stream->position + 1;  // Skip the "["
    const auto& tokens = stream->tokens;
    size_t size = tokens.size();
    bool isRange = false;
    int counter = 1;

    while (pos < size && counter > 0) {
      const auto type = tokens.at(pos++).getType();

      if (type == KTokenType::OPEN_BRACKET) {
        ++counter;
      } else if (type == KTokenType::CLOSE_BRACKET) {
        --counter;
        if (counter == 0) {
          break;
        }
      }

      if (type == KTokenType::RANGE) {
        isRange = true;
        break;
      }
    }

    return isRange;
  }

  static bool hasReturnValue(std::shared_ptr<TokenStream> stream) {
    const Token nextToken = stream->peek();
    const auto KTokenType = nextToken.getType();
    bool isLiteral = KTokenType == KTokenType::LITERAL;
    bool isString = KTokenType == KTokenType::STRING;
    bool isIdentifier = KTokenType == KTokenType::IDENTIFIER;
    bool isParenthesis = KTokenType == KTokenType::OPEN_PAREN;
    bool isBraced = KTokenType == KTokenType::OPEN_BRACE;
    bool isBracketed = KTokenType == KTokenType::OPEN_BRACKET;
    bool isInstanceInvocation = KTokenType == KTokenType::KEYWORD &&
                                nextToken.getSubType() == KName::KW_This;
    return isString || isLiteral || isIdentifier || isParenthesis ||
           isBracketed || isInstanceInvocation || isBraced;
  }

  static bool shouldUpdateFrameVariables(
      const std::string& varName,
      const std::shared_ptr<CallStackFrame> nextFrame) {
    return nextFrame->variables.find(varName) != nextFrame->variables.end();
  }

  static void updateVariablesInCallerFrame(
      const std::unordered_map<std::string, Value>& variables,
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

  static std::vector<Token> collectBodyTokens(
      std::shared_ptr<TokenStream>& stream) {
    std::vector<Token> tokens;
    int counter = 1;

    while (stream->canRead() && counter != 0) {
      const Token& currentToken = stream->current();
      const KName subType = currentToken.getSubType();

      if (Keywords.is_block_keyword(subType)) {
        ++counter;
      } else if (subType == KName::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();
          continue;
        }
      }

      tokens.push_back(currentToken);
      stream->next();
    }

    return tokens;
  }

  static void collectBodyTokens(std::vector<Token>& tokens,
                                std::shared_ptr<TokenStream>& stream) {
    int counter = 1;

    while (stream->canRead() && counter != 0) {
      const Token& currentToken = stream->current();
      const KName subType = currentToken.getSubType();

      if (Keywords.is_block_keyword(subType)) {
        ++counter;
      } else if (subType == KName::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();
          continue;
        }
      }

      tokens.push_back(currentToken);
      stream->next();
    }
  }

  static std::vector<Token> getTemporaryAssignment(const Token& term,
                                                   const std::string& tempId) {
    std::vector<Token> tokens;
    auto file = term.getFile();
    tokens.push_back(Token::create(KTokenType::IDENTIFIER, KName::Default,
                                   file, tempId, 0, 0));
    tokens.push_back(Token::create(KTokenType::OPERATOR,
                                   KName::Ops_Assign, file,
                                   Operators.Assign, 0, 0));

    return tokens;
  }

  static void updateListSlice(std::shared_ptr<TokenStream> stream,
                              bool insertOp, std::shared_ptr<List>& targetList,
                              const SliceIndex& slice,
                              const std::shared_ptr<List>& rhsValues) {
    if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
      throw IndexError(stream->current(), "Start index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
      throw IndexError(stream->current(), "Stop index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
      throw IndexError(stream->current(), "Step value must be an integer.");
    }

    int start = std::get<k_int>(slice.indexOrStart);
    int stop = std::get<k_int>(slice.stopIndex);
    int step = std::get<k_int>(slice.stepValue);

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
                                 const KName& op,
                                 const Value& currentValue,
                                 const Value& value) {
    switch (op) {
      case KName::Ops_AddAssign:
        return std::visit(AddVisitor(stream->current()), currentValue, value);

      case KName::Ops_SubtractAssign:
        return std::visit(SubtractVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_MultiplyAssign:
        return std::visit(MultiplyVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_DivideAssign:
        return std::visit(DivideVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_ExponentAssign:
        return std::visit(PowerVisitor(stream->current()), currentValue, value);

      case KName::Ops_ModuloAssign:
        return std::visit(ModuloVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseAndAssign:
        return std::visit(BitwiseAndVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseOrAssign:
        return std::visit(BitwiseOrVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseXorAssign:
        return std::visit(BitwiseXorVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseLeftShiftAssign:
        return std::visit(BitwiseLeftShiftVisitor(stream->current()),
                          currentValue, value);

      case KName::Ops_BitwiseRightShiftAssign:
        return std::visit(BitwiseRightShiftVisitor(stream->current()),
                          currentValue, value);

      case KName::Ops_BitwiseNotAssign:
        return std::visit(BitwiseNotVisitor(stream->current()), value);

      default:
        break;
    }

    throw InvalidOperationError(stream->current(), "Invalid operator.");
  }

  static Value interpretListSlice(std::shared_ptr<TokenStream> stream,
                                  const SliceIndex& slice,
                                  const std::shared_ptr<List>& list) {
    if (slice.isSlice) {
      if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
        throw IndexError(stream->current(), "Start index must be an integer.");
      } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
        throw IndexError(stream->current(), "Stop index must be an integer.");
      } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
        throw IndexError(stream->current(), "Step value must be an integer.");
      }

      int start = static_cast<int>(std::get<k_int>(slice.indexOrStart)),
          stop = static_cast<int>(std::get<k_int>(slice.stopIndex)),
          step = static_cast<int>(std::get<k_int>(slice.stepValue));

      // Adjust negative indices
      int listSize = static_cast<int>(list->elements.size());
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
      if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
        throw IndexError(stream->current(), "Index value must be an integer.");
      }

      int index = std::get<k_int>(slice.indexOrStart);
      int listSize = list->elements.size();

      if (index < 0) {
        index += listSize;  // Adjust for negative index
      }

      if (index < 0 || index >= listSize) {
        throw RangeError(stream->current(), "List index out of range.");
      }

      return list->elements[index];
    }
  }

  static void interpretParameterizedCatch(std::shared_ptr<TokenStream> stream,
                                          std::shared_ptr<CallStackFrame> frame,
                                          std::string& errorVariableName,
                                          Value& errorValue) {
    stream->next();  // Skip "("

    if (stream->current().getType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(
          stream->current(),
          "Syntax error in catch variable declaration. Missing identifier.");
    }

    errorVariableName = stream->current().getText();
    stream->next();  // Skip the identifier.

    if (stream->current().getType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(stream->current(),
                        "Syntax error in catch variable declaration.");
    }
    stream->next();  // Skip ")"

    errorValue = frame->getErrorMessage();
  }

  static std::string interpretModuleHome(std::string& modulePath,
                                         std::shared_ptr<TokenStream> stream) {
    if (stream->current().getType() != KTokenType::STRING ||
        !String::beginsWith(modulePath, "@")) {
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
          lastToken.getType() == KTokenType::DECLVAR) {
        if (tokens.at(pos + 1).getSubType() == KName::Ops_Divide) {
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
    if (stream->current().getType() == KTokenType::OPERATOR) {
      if (stream->current().getSubType() != KName::Ops_LessThan) {
        throw SyntaxError(
            stream->current(),
            "Expected inheritance operator, `<`, in class definition.");
      }
      stream->next();

      if (stream->current().getType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(stream->current(), "Expected base class name.");
      }

      baseClassName = stream->current().getText();
      stream->next();  // Skip base class.
    }
    return baseClassName;
  }
};

#endif
#ifndef KIWI_PARSING_PARSER_H
#define KIWI_PARSING_PARSER_H

#include <memory>
#include <optional>

#include "ast.h"
#include "keywords.h"
#include "math/rng.h"
#include "tokens.h"
#include "tracing/error.h"
#include "tracing/handler.h"
#include "typing/value.h"

class Parser {
 public:
  Parser() {}

  std::unique_ptr<ASTNode> parseTokenStreamCollection(
      std::vector<k_stream> streams);
  std::unique_ptr<ASTNode> parseTokenStream(k_stream& stream, bool isScript);

 private:
  bool hasValue();
  std::unique_ptr<ASTNode> parsePackAssignment(
      std::unique_ptr<ASTNode> baseNode);
  std::unique_ptr<ASTNode> parseAssignment(std::unique_ptr<ASTNode> baseNode,
                                           const k_string& identifierName);
  std::unique_ptr<ASTNode> parseComment();
  std::unique_ptr<ASTNode> parseFunction();
  std::unique_ptr<ASTNode> parseFunctionCall(const k_string& identifierName,
                                             const KName& type);
  std::unique_ptr<ASTNode> parseLambdaCall(std::unique_ptr<ASTNode> lambdaNode);
  std::unique_ptr<ASTNode> parseKeyword();
  std::unique_ptr<ASTNode> parseClass();
  std::unique_ptr<ASTNode> parseInterface();
  std::unique_ptr<ASTNode> parseLambda();
  std::unique_ptr<ASTNode> parseStatement();
  std::unique_ptr<ASTNode> parseForLoop();
  std::unique_ptr<ASTNode> parseRepeatLoop();
  std::unique_ptr<ASTNode> parseWhileLoop();
  std::unique_ptr<ASTNode> parseTry();
  std::unique_ptr<ASTNode> parseConditional();
  std::unique_ptr<ASTNode> parseIf();
  std::unique_ptr<ASTNode> parseCase();
  std::unique_ptr<ASTNode> parseReturn();
  std::unique_ptr<ASTNode> parseThrow();
  std::unique_ptr<ASTNode> parseExit();
  std::unique_ptr<ASTNode> parseParse();
  std::unique_ptr<ASTNode> parseNext();
  std::unique_ptr<ASTNode> parseBreak();
  std::unique_ptr<ASTNode> parseImport();
  std::unique_ptr<ASTNode> parseExport();
  std::unique_ptr<ASTNode> parsePackage();
  std::unique_ptr<ASTNode> parseExpression();
  std::unique_ptr<ASTNode> parseLogicalOr();
  std::unique_ptr<ASTNode> parseLogicalAnd();
  std::unique_ptr<ASTNode> parseBitwiseOr();
  std::unique_ptr<ASTNode> parseBitwiseXor();
  std::unique_ptr<ASTNode> parseBitwiseAnd();
  std::unique_ptr<ASTNode> parseEquality();
  std::unique_ptr<ASTNode> parseComparison();
  std::unique_ptr<ASTNode> parseBitshift();
  std::unique_ptr<ASTNode> parseAdditive();
  std::unique_ptr<ASTNode> parseMultiplicative();
  std::unique_ptr<ASTNode> parseUnary();
  std::unique_ptr<ASTNode> parsePrimary();
  std::unique_ptr<ASTNode> parseLiteral();
  std::unique_ptr<ASTNode> parseHashLiteral();
  std::unique_ptr<ASTNode> parseListLiteral();
  std::unique_ptr<ASTNode> parseRangeLiteral();
  std::unique_ptr<ASTNode> parseIndexingInternal(
      std::unique_ptr<ASTNode> baseNode);
  std::unique_ptr<ASTNode> parseIndexing(const k_string& identifierName);
  std::unique_ptr<ASTNode> parseIndexing(
      std::unique_ptr<ASTNode> indexedObject);
  std::unique_ptr<ASTNode> parseMemberAccess(std::unique_ptr<ASTNode> left);
  std::unique_ptr<ASTNode> parseMemberAssignment(
      std::unique_ptr<ASTNode> object, const k_string& memberName);
  std::unique_ptr<ASTNode> parseFunctionCallOnMember(
      std::unique_ptr<ASTNode> object, const k_string& methodName,
      const KName& type);
  std::unique_ptr<ASTNode> parseIdentifier(bool packed);
  std::unique_ptr<ASTNode> parseQualifiedIdentifier(const k_string& prefix);
  std::unique_ptr<ASTNode> parsePrint();

  // Utility methods to help with token matching and advancing the stream
  // Instead of passing streams everywhere, I'm going to just keep it local to the parser.
  Token next();
  bool match(KTokenType expectedType);
  bool matchSubType(KName expectedSubType);
  bool lookAhead(std::vector<KName> names);
  KTokenType tokenType();
  KName tokenName();
  Token peek();

  Token kToken = Token::createEmpty();
  k_stream kStream;
  Token getErrorToken();
  std::unordered_map<k_string, k_string> mangledNames;
};

KTokenType Parser::tokenType() {
  return kToken.getType();
}

KName Parser::tokenName() {
  return kToken.getSubType();
}

Token Parser::peek() {
  return kStream->peek();
}

bool Parser::lookAhead(std::vector<KName> names) {
  size_t pos = kStream->position;
  size_t nameLength = names.size();
  for (; pos + 1 < kStream->size(); ++pos) {
    size_t matches = 0;
    
    for (size_t i = 0; i < nameLength; ++i) {
      if (kStream->at(pos + i).getSubType() == names.at(i)) {
        ++matches;
      } else {
        --matches;
      }
    }

    if (matches == nameLength) {
      return true;
    }
  }

  return false;
}

Token Parser::next() {
  kStream->next();
  kToken = kStream->current();
  return kToken;
}

Token Parser::getErrorToken() {
  if (tokenType() != KTokenType::STREAM_END) {
    return kToken;
  }

  kStream->rewind();
  return kStream->current();
}

bool Parser::match(KTokenType expectedType) {
  if (tokenType() == expectedType) {
    next();
    return true;
  }
  return false;
}

bool Parser::matchSubType(KName expectedSubType) {
  if (tokenName() == expectedSubType) {
    next();
    return true;
  }
  return false;
}

bool Parser::hasValue() {
  switch (tokenType()) {
    case KTokenType::LITERAL:
    case KTokenType::STRING:
    case KTokenType::TYPENAME:
    case KTokenType::IDENTIFIER:
    case KTokenType::OPEN_PAREN:
    case KTokenType::OPEN_BRACE:
    case KTokenType::OPEN_BRACKET:
      return true;

    case KTokenType::KEYWORD:
      return tokenName() == KName::KW_This;

    case KTokenType::OPERATOR:
      return Operators.is_unary_op(tokenName());

    default:
      return false;
  }
}

std::unique_ptr<ASTNode> Parser::parseTokenStreamCollection(
    std::vector<k_stream> streams) {
  auto root = std::make_unique<ProgramNode>();
  for (const auto& stream : streams) {
    kStream = std::move(stream);
    kToken = kStream->current();

    try {
      while (tokenType() != KTokenType::STREAM_END) {
        auto statement = parseStatement();
        if (statement) {
          root->statements.push_back(std::move(statement));
        }
      }
    } catch (const KiwiError& e) {
      ErrorHandler::handleError(e);
    }
  }

  return root;
}

std::unique_ptr<ASTNode> Parser::parseTokenStream(k_stream& stream,
                                                  bool isScript = false) {
  kStream = std::move(stream);
  kToken = kStream->current();  // Set to beginning.

  auto root = std::make_unique<ProgramNode>();
  root->isScript = isScript;

  try {
    while (tokenType() != KTokenType::STREAM_END) {
      auto statement = parseStatement();
      if (statement) {
        root->statements.push_back(std::move(statement));
      }
    }
  } catch (const KiwiError& e) {
    ErrorHandler::handleError(e);
  }

  return root;
}

std::unique_ptr<ASTNode> Parser::parseConditional() {
  if (tokenName() == KName::KW_If) {
    return parseIf();
  } else if (tokenName() == KName::KW_Case) {
    return parseCase();
  }

  throw SyntaxError(getErrorToken(),
                    "Expected if-statement or case-statement.");
}

std::unique_ptr<ASTNode> Parser::parseKeyword() {
  switch (tokenName()) {
    case KName::KW_PrintLn:
    case KName::KW_Print:
      return parsePrint();

    case KName::KW_For:
      return parseForLoop();

    case KName::KW_While:
      return parseWhileLoop();

    case KName::KW_This:
      return parseIdentifier(false);

    case KName::KW_Repeat:
      return parseRepeatLoop();

    case KName::KW_Try:
      return parseTry();

    case KName::KW_Return:
      return parseReturn();

    case KName::KW_Throw:
      return parseThrow();

    case KName::KW_Exit:
      return parseExit();

    case KName::KW_Parse:
      return parseParse();

    case KName::KW_Next:
      return parseNext();

    case KName::KW_Break:
      return parseBreak();

    case KName::KW_Pass:
      return std::make_unique<NoOpNode>();

    case KName::KW_Package:
      return parsePackage();

    case KName::KW_Import:
      return parseImport();

    case KName::KW_Export:
      return parseExport();

    case KName::KW_Class:
      return parseClass();

    case KName::KW_Interface:
      return parseInterface();

    case KName::KW_Method:
      return parseFunction();

    default:
      throw SyntaxError(getErrorToken(),
                        "Unexpected keyword '" + kToken.getText() + "'.");
  }

  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
  auto nodeToken = kToken;
  std::unique_ptr<ASTNode> node;

  switch (nodeToken.getType()) {
    case KTokenType::COMMENT:
      node = parseComment();
      break;

    case KTokenType::COMMA:
      match(KTokenType::COMMA);
      return nullptr;

    case KTokenType::KEYWORD:
      node = parseKeyword();
      break;

    case KTokenType::CONDITIONAL:
      node = parseConditional();
      break;

    case KTokenType::OPEN_BRACE:
    case KTokenType::OPEN_BRACKET:
    case KTokenType::OPEN_PAREN:
    case KTokenType::LITERAL:
    case KTokenType::OPERATOR:
    case KTokenType::IDENTIFIER:
    case KTokenType::STRING:
      return parseExpression();

    default:
      throw TokenStreamError(getErrorToken(), "Unexpected token in statement.");
  }

  if (node) {
    node->token = nodeToken;
    return node;
  }

  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseComment() {
  match(KTokenType::COMMENT);
  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseClass() {
  matchSubType(KName::KW_Class);

  if (tokenType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(getErrorToken(), "Expected identifier for class name.");
  }

  auto className = kToken.getText();
  next();

  k_string baseClass;

  // Extends
  if (matchSubType(KName::Ops_LessThan)) {
    if (tokenType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(getErrorToken(),
                        "Expected identifier for base class name.");
    }

    baseClass = kToken.getText();
    next();
  }

  std::vector<k_string> interfaces;
  if (match(KTokenType::COLON)) {
    while (tokenType() != KTokenType::KEYWORD) {
      if (tokenType() == KTokenType::IDENTIFIER) {
        interfaces.push_back(kToken.getText());
      }
      next();
    }
  }

  std::vector<std::unique_ptr<ASTNode>> methods;
  bool isStatic = false, isPrivate = false;

  while (tokenName() != KName::KW_End) {
    if (matchSubType(KName::KW_Static)) {
      isStatic = true;
      continue;
    } else if (matchSubType(KName::KW_Private)) {
      isPrivate = true;
      continue;
    }

    auto statement = parseStatement();

    if (statement->type == ASTNodeType::FUNCTION_DECLARATION) {
      auto func = static_cast<FunctionDeclarationNode*>(statement.get());
      func->isPrivate = isPrivate;
      func->isStatic = isStatic;

      methods.push_back(std::move(statement));
    }

    isStatic = false;
    isPrivate = false;
  }

  next();  // Consume 'end'

  return std::make_unique<ClassNode>(className, baseClass,
                                     std::move(interfaces), std::move(methods));
}

std::unique_ptr<ASTNode> Parser::parseInterface() {
  // WIP:
  // std::vector<std::unique_ptr<ASTNode>> methods;
  // while (tokenName() != KName::KW_End) {
  //   if (tokenName() == KName::KW_Method) {}
  // }
  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseFunction() {
  match(KTokenType::KEYWORD);  // Consume 'fn'

  if (tokenType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(getErrorToken(), "Expected identifier after 'fn'.");
  }

  std::string functionName = kToken.getText();
  next();

  k_string mangler = "_" + RNG::getInstance().random8() + "_";

  // Parse parameters
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  if (tokenType() == KTokenType::OPEN_PAREN) {
    next();  // Consume '('

    while (tokenType() != KTokenType::CLOSE_PAREN) {
      if (tokenType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(getErrorToken(), "Expected parameter name.");
      }

      auto paramName = kToken.getText();
      auto mangledName = mangler + paramName;
      mangledNames[paramName] = mangledName;
      std::unique_ptr<ASTNode> defaultValue = nullptr;
      next();

      // Check for default value
      if (tokenType() == KTokenType::OPERATOR &&
          tokenName() == KName::Ops_Assign) {
        next();  // Consume '='
        defaultValue = parseExpression();
      }

      parameters.emplace_back(mangledName, std::move(defaultValue));

      if (tokenType() == KTokenType::COMMA) {
        next();
      } else if (tokenType() != KTokenType::CLOSE_PAREN) {
        throw SyntaxError(getErrorToken(),
                          "Expected ',' or ')' in parameter list.");
      }
    }

    next();  // Consume ')'
  }

  // Parse the function body
  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  mangledNames.clear();

  auto functionDeclaration = std::make_unique<FunctionDeclarationNode>();
  functionDeclaration->name = functionName;
  functionDeclaration->parameters = std::move(parameters);
  functionDeclaration->body = std::move(body);
  return functionDeclaration;
}

std::unique_ptr<ASTNode> Parser::parseForLoop() {
  matchSubType(KName::KW_For);  // Consume 'for'

  k_string mangler = "_" + RNG::getInstance().random8() + "_";
  std::unordered_set<k_string> subMangled;

  if (tokenType() == KTokenType::IDENTIFIER) {
    mangledNames[kToken.getText()] = mangler + kToken.getText();
    subMangled.emplace(mangler + kToken.getText());
  }

  auto valueIterator = parseIdentifier(false);
  std::optional<std::unique_ptr<ASTNode>> indexIterator = std::nullopt;

  if (match(KTokenType::COMMA)) {
    if (tokenType() == KTokenType::IDENTIFIER) {
      mangledNames[kToken.getText()] = mangler + kToken.getText();
      subMangled.emplace(mangler + kToken.getText());
    }
    indexIterator = parseIdentifier(false);
  }

  if (!matchSubType(KName::KW_In)) {
    throw SyntaxError(getErrorToken(), "Expected 'in' in for-loop.");
  }

  auto dataSet = parseExpression();

  if (!matchSubType(KName::KW_Do)) {
    throw SyntaxError(getErrorToken(), "Expected 'do' in for-loop.");
  }

  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  for (const auto& mangledName : subMangled) {
    mangledNames.erase(mangledName);
  }

  auto forLoop = std::make_unique<ForLoopNode>();
  forLoop->valueIterator = std::move(valueIterator);
  forLoop->indexIterator =
      indexIterator ? std::move(indexIterator.value()) : nullptr;
  forLoop->dataSet = std::move(dataSet);
  forLoop->body = std::move(body);
  return forLoop;
}

std::unique_ptr<ASTNode> Parser::parseWhileLoop() {
  matchSubType(KName::KW_While);  // Consume 'while'

  auto condition = parseExpression();

  if (!matchSubType(KName::KW_Do)) {
    throw SyntaxError(getErrorToken(), "Expected 'do' in for-loop.");
  }

  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  auto whileLoop = std::make_unique<WhileLoopNode>();
  whileLoop->condition = std::move(condition);
  whileLoop->body = std::move(body);
  return whileLoop;
}

std::unique_ptr<ASTNode> Parser::parseRepeatLoop() {
  matchSubType(KName::KW_Repeat);  // Consume 'repeat'

  auto count = parseExpression();
  std::optional<std::unique_ptr<ASTNode>> alias = std::nullopt;

  if (matchSubType(KName::KW_As)) {
    if (!tokenType() == KTokenType::IDENTIFIER) {
      throw SyntaxError(getErrorToken(),
                        "Expected identifier in repeat-loop value alias.");
    }

    alias = parseIdentifier(false);
  }

  if (!matchSubType(KName::KW_Do)) {
    throw SyntaxError(getErrorToken(), "Expected 'do' in for-loop.");
  }

  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  auto repeatLoop = std::make_unique<RepeatLoopNode>();
  repeatLoop->count = std::move(count);
  repeatLoop->alias = alias ? std::move(alias.value()) : nullptr;
  repeatLoop->body = std::move(body);
  return repeatLoop;
}

std::unique_ptr<ASTNode> Parser::parseReturn() {
  matchSubType(KName::KW_Return);
  auto node = std::make_unique<ReturnNode>();

  if (hasValue()) {
    node->returnValue = parseExpression();
  }

  if (matchSubType(KName::KW_When)) {
    if (!hasValue()) {
      throw SyntaxError(getErrorToken(), "Expected condition after 'when'.");
    }

    node->condition = parseExpression();
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseThrow() {
  matchSubType(KName::KW_Throw);
  auto node = std::make_unique<ThrowNode>();

  if (hasValue()) {
    node->errorValue = parseExpression();
  }

  if (matchSubType(KName::KW_When)) {
    if (!hasValue()) {
      throw SyntaxError(getErrorToken(), "Expected condition after 'when'.");
    }

    node->condition = parseExpression();
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseExit() {
  matchSubType(KName::KW_Exit);
  auto node = std::make_unique<ExitNode>();

  if (hasValue()) {
    node->exitValue = parseExpression();
  }

  if (matchSubType(KName::KW_When)) {
    if (!hasValue()) {
      throw SyntaxError(getErrorToken(), "Expected condition after 'when'.");
    }

    node->condition = parseExpression();
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseBreak() {
  matchSubType(KName::KW_Break);
  auto node = std::make_unique<BreakNode>();

  if (matchSubType(KName::KW_When)) {
    if (!hasValue()) {
      throw SyntaxError(getErrorToken(), "Expected condition after 'when'.");
    }

    node->condition = parseExpression();
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseNext() {
  matchSubType(KName::KW_Next);
  auto node = std::make_unique<NextNode>();

  if (matchSubType(KName::KW_When)) {
    if (!hasValue()) {
      throw SyntaxError(getErrorToken(), "Expected condition after 'when'.");
    }

    node->condition = parseExpression();
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseParse() {
  matchSubType(KName::KW_Parse);

  if (!hasValue()) {
    throw SyntaxError(getErrorToken(), "Expected value after 'parse'.");
  }

  return std::make_unique<ParseNode>(parseExpression());
}

std::unique_ptr<ASTNode> Parser::parseExport() {
  matchSubType(KName::KW_Export);

  if (!hasValue()) {
    throw SyntaxError(getErrorToken(), "Expected value after 'export'.");
  }

  return std::make_unique<ExportNode>(parseExpression());
}

std::unique_ptr<ASTNode> Parser::parseImport() {
  matchSubType(KName::KW_Import);

  if (!hasValue()) {
    throw SyntaxError(getErrorToken(), "Expected value after 'import'.");
  }

  return std::make_unique<ImportNode>(parseExpression());
}

std::unique_ptr<ASTNode> Parser::parsePackage() {
  matchSubType(KName::KW_Package);

  if (tokenType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(getErrorToken(), "Expected identifier for package name.");
  }

  auto packageName = parseIdentifier(false);

  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  auto package = std::make_unique<PackageNode>(std::move(packageName));
  package->body = std::move(body);

  return package;
}

std::unique_ptr<ASTNode> Parser::parseCase() {
  if (!matchSubType(KName::KW_Case)) {
    throw SyntaxError(getErrorToken(), "Expected case-statement.");
  }

  auto node = std::make_unique<CaseNode>();

  if (hasValue()) {
    node->testValue = parseExpression();
  }

  while (tokenName() != KName::KW_End) {
    if (matchSubType(KName::KW_When)) {
      auto caseWhen = std::make_unique<CaseWhenNode>();
      if (!hasValue()) {
        throw SyntaxError(getErrorToken(),
                          "Expected condition or value for case-when.");
      }

      caseWhen->condition = parseExpression();

      while (tokenName() != KName::KW_When && tokenName() != KName::KW_Else &&
             tokenName() != KName::KW_End) {
        auto stmt = parseStatement();
        if (stmt) {
          caseWhen->body.push_back(std::move(stmt));
        }
      }

      node->whenNodes.push_back(std::move(caseWhen));
    } else if (matchSubType(KName::KW_Else)) {
      while (tokenName() != KName::KW_End) {
        auto stmt = parseStatement();
        if (stmt) {
          node->elseBody.push_back(std::move(stmt));
        }
      }
    }
  }

  next();  // Consume 'end'

  return node;
}

std::unique_ptr<ASTNode> Parser::parseIf() {
  if (!matchSubType(KName::KW_If)) {
    throw SyntaxError(getErrorToken(), "Expected if-statement.");
  }

  if (!hasValue()) {
    throw SyntaxError(getErrorToken(), "Expected condition after 'if'.");
  }

  auto node = std::make_unique<IfNode>();

  node->condition = parseExpression();

  int blocks = 1;
  auto building = KName::KW_If;

  while (kStream->canRead() && blocks > 0) {
    auto subType = tokenName();
    if (subType == KName::KW_End && blocks >= 1) {
      --blocks;

      // Stop here.
      if (blocks == 0) {
        next();
        break;
      }
    } else if (blocks == 1 && subType == KName::KW_Else) {
      if (building != KName::KW_Else) {
        next();
        building = KName::KW_Else;
      }
    } else if (blocks == 1 && subType == KName::KW_ElseIf) {
      next();
      building = KName::KW_ElseIf;

      auto elsif = std::make_unique<IfNode>();

      if (!hasValue()) {
        throw SyntaxError(getErrorToken(), "Expected condition after 'elsif'.");
      }

      elsif->condition = parseExpression();
      node->elseifNodes.push_back(std::move(elsif));
    }

    auto stmt = parseStatement();
    if (building == KName::KW_If && stmt) {
      node->body.push_back(std::move(stmt));
    } else if (building == KName::KW_ElseIf && stmt) {
      node->elseifNodes.back()->body.push_back(std::move(stmt));
    } else if (building == KName::KW_Else && stmt) {
      node->elseBody.push_back(std::move(stmt));
    }
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseTry() {
  matchSubType(KName::KW_Try);  // Consume 'try'

  std::vector<std::unique_ptr<ASTNode>> tryBody;
  std::vector<std::unique_ptr<ASTNode>> catchBody;
  std::vector<std::unique_ptr<ASTNode>> finallyBody;
  std::optional<std::unique_ptr<ASTNode>> errorType = std::nullopt;
  std::optional<std::unique_ptr<ASTNode>> errorMessage = std::nullopt;

  int blocks = 1;
  auto building = KName::KW_Try;

  while (kStream->canRead() && blocks > 0) {
    auto subType = tokenName();

    if (subType == KName::KW_End && blocks >= 1) {
      --blocks;

      // Stop here.
      if (blocks == 0) {
        next();
        break;
      }
    } else if (blocks == 1 && subType == KName::KW_Catch) {
      if (building != KName::KW_Catch) {
        next();  // Consume 'catch'
        if (match(KTokenType::OPEN_PAREN)) {
          if (!tokenType() == KTokenType::IDENTIFIER) {
            throw SyntaxError(getErrorToken(),
                              "Expected identifier in catch parameters.");
          }
          auto firstParameter = parseIdentifier(false);
          if (match(KTokenType::COMMA)) {
            if (!tokenType() == KTokenType::IDENTIFIER) {
              throw SyntaxError(getErrorToken(),
                                "Expected identifier in catch parameters.");
            }

            errorType = std::move(firstParameter);
            errorMessage = std::move(parseIdentifier(false));
          } else {
            errorMessage = std::move(firstParameter);
          }

          if (!match(KTokenType::CLOSE_PAREN)) {
            throw SyntaxError(getErrorToken(),
                              "Expected ')' in catch parameter expression.");
          }
        }
        building = KName::KW_Catch;
        continue;
      }
    } else if (blocks == 1 && subType == KName::KW_Finally) {
      if (building != KName::KW_Finally) {
        next();  // Consume 'finally'
        building = KName::KW_Finally;
        continue;
      }
    }

    auto stmt = parseStatement();
    if (building == KName::KW_Try && stmt) {
      tryBody.push_back(std::move(stmt));
    } else if (building == KName::KW_Catch && stmt) {
      catchBody.push_back(std::move(stmt));
    } else if (building == KName::KW_Finally && stmt) {
      finallyBody.push_back(std::move(stmt));
    }
  }

  auto tryCatchFinally = std::make_unique<TryNode>();
  tryCatchFinally->tryBody = std::move(tryBody);
  tryCatchFinally->catchBody = std::move(catchBody);
  tryCatchFinally->finallyBody = std::move(finallyBody);
  tryCatchFinally->errorMessage =
      errorMessage ? std::move(errorMessage.value()) : nullptr;
  tryCatchFinally->errorType =
      errorType ? std::move(errorType.value()) : nullptr;
  return tryCatchFinally;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(
    const k_string& identifierName, const KName& type) {
  next();

  std::vector<std::unique_ptr<ASTNode>> arguments;
  while (tokenType() != KTokenType::CLOSE_PAREN) {
    arguments.push_back(parseExpression());

    if (tokenType() == KTokenType::COMMA) {
      next();
    } else if (tokenType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(getErrorToken(),
                        "Expected ')' or ',' in function call.");
    }
  }

  next();

  return std::make_unique<FunctionCallNode>(identifierName, type,
                                            std::move(arguments));
}

std::unique_ptr<ASTNode> Parser::parseLambdaCall(
    std::unique_ptr<ASTNode> lambdaNode) {
  next();  // Consume the '('

  std::vector<std::unique_ptr<ASTNode>> arguments;
  while (tokenType() != KTokenType::CLOSE_PAREN) {
    arguments.push_back(parseExpression());

    if (tokenType() == KTokenType::COMMA) {
      next();
    } else if (tokenType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(getErrorToken(), "Expected ')' or ',' in lambda call.");
    }
  }

  next();

  return std::make_unique<LambdaCallNode>(std::move(lambdaNode),
                                          std::move(arguments));
}

std::unique_ptr<ASTNode> Parser::parseLambda() {
  match(KTokenType::LAMBDA);  // Consume 'with'

  // Parse parameters
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  if (tokenType() == KTokenType::OPEN_PAREN) {
    next();  // Consume '('

    while (tokenType() != KTokenType::CLOSE_PAREN) {
      if (tokenType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(getErrorToken(), "Expected parameter name.");
      }

      std::string paramName = kToken.getText();
      std::unique_ptr<ASTNode> defaultValue = nullptr;
      next();

      // Check for default value
      if (tokenType() == KTokenType::OPERATOR &&
          tokenName() == KName::Ops_Assign) {
        next();  // Consume '='
        defaultValue = parseExpression();
      }

      parameters.emplace_back(paramName, std::move(defaultValue));

      if (tokenType() == KTokenType::COMMA) {
        next();
      } else if (tokenType() != KTokenType::CLOSE_PAREN) {
        throw SyntaxError(getErrorToken(),
                          "Expected ',' or ')' in parameter list.");
      }
    }

    next();  // Consume ')'
  }

  if (!matchSubType(KName::KW_Do)) {
    throw SyntaxError(getErrorToken(), "Expected 'do' in lambda expression.");
  }

  // Parse the lambda body
  std::vector<std::unique_ptr<ASTNode>> body;
  while (tokenName() != KName::KW_End) {
    auto stmt = parseStatement();
    if (stmt) {
      body.push_back(std::move(stmt));
    }
  }

  next();  // Consume 'end'

  auto lambda = std::make_unique<LambdaNode>();
  lambda->parameters = std::move(parameters);
  lambda->body = std::move(body);

  return lambda;
}

std::unique_ptr<ASTNode> Parser::parsePrint() {
  auto printNode = std::make_unique<PrintNode>();
  printNode->printNewline = tokenName() == KName::KW_PrintLn;
  match(KTokenType::KEYWORD);  // Consume 'print'/'println'
  printNode->expression = parseExpression();
  return printNode;
}

std::unique_ptr<ASTNode> Parser::parseLiteral() {
  auto literalNode = std::make_unique<LiteralNode>();
  literalNode->value = kToken.getValue();
  next();  // Consume literal
  return literalNode;
}

std::unique_ptr<ASTNode> Parser::parseHashLiteral() {
  std::map<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>> elements;
  std::vector<k_string> keys;

  match(KTokenType::OPEN_BRACE);  // Consume '{'

  while (tokenType() != KTokenType::CLOSE_BRACE) {
    if (tokenType() != KTokenType::STRING &&
        tokenType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(getErrorToken(),
                        "Expected a string or an identifier for hash key.");
    }

    auto keyString = kToken.getText();
    auto key = parseExpression();

    if (!match(KTokenType::COLON)) {
      throw SyntaxError(getErrorToken(), "Expected ':' in hash literal");
    }

    keys.push_back(keyString);

    auto value = parseExpression();
    elements.emplace(std::move(key), std::move(value));

    if (tokenType() == KTokenType::COMMA) {
      next();  // Consume ','
    } else if (tokenType() != KTokenType::CLOSE_BRACE) {
      throw SyntaxError(getErrorToken(), "Expected '}' or ',' in hash literal");
    }
  }

  match(KTokenType::CLOSE_BRACE);  // Consume '}'

  return std::make_unique<HashLiteralNode>(std::move(elements),
                                           std::move(keys));
}

std::unique_ptr<ASTNode> Parser::parseListLiteral() {
  std::vector<std::unique_ptr<ASTNode>> elements;

  match(KTokenType::OPEN_BRACKET);  // Consume '['
  auto isRange = false;

  while (tokenType() != KTokenType::CLOSE_BRACKET) {
    elements.push_back(parseExpression());

    if (!isRange && tokenType() == KTokenType::COMMA) {
      next();  // Consume ','
    } else if (!isRange && tokenType() == KTokenType::RANGE) {
      isRange = true;
      next();  // Consume '..'
    } else if (tokenType() != KTokenType::CLOSE_BRACKET) {
      if (!isRange) {
        throw SyntaxError(getErrorToken(),
                          "Expected ']' or ',' in list literal.");
      } else {
        throw SyntaxError(getErrorToken(),
                          "Expected ']' or '..' in range literal.");
      }
    }
  }

  if (!match(KTokenType::CLOSE_BRACKET)) {
    throw SyntaxError(getErrorToken(),
                      "Expected ']' in list or range literal.");
  }

  if (isRange) {
    if (elements.size() != 2) {
      throw SyntaxError(getErrorToken(),
                        "Expected start and end values in range literal.");
    }

    return std::make_unique<RangeLiteralNode>(std::move(elements.at(0)),
                                              std::move(elements.at(1)));
  }

  return std::make_unique<ListLiteralNode>(std::move(elements));
}

std::unique_ptr<ASTNode> Parser::parseIndexing(
    std::unique_ptr<ASTNode> indexedObject) {
  return parseIndexingInternal(std::move(indexedObject));
}

std::unique_ptr<ASTNode> Parser::parseIndexing(const k_string& identifierName) {
  std::unique_ptr<ASTNode> base =
      std::make_unique<IdentifierNode>(identifierName);
  return parseIndexingInternal(std::move(base));
}

std::unique_ptr<ASTNode> Parser::parseIndexingInternal(
    std::unique_ptr<ASTNode> baseNode) {
  if (!match(KTokenType::OPEN_BRACKET)) {
    return baseNode;
  }

  if (match(KTokenType::CLOSE_BRACKET)) {
    return baseNode;
  }

  auto isSlice = false;
  Token indexValueToken = getErrorToken();

  std::optional<std::unique_ptr<ASTNode>> start = std::nullopt;
  std::optional<std::unique_ptr<ASTNode>> stop = std::nullopt;
  std::optional<std::unique_ptr<ASTNode>> step = std::nullopt;

  if (tokenType() != KTokenType::COLON &&
      tokenType() != KTokenType::QUALIFIER) {
    start = parseExpression();
  }

  if (match(KTokenType::COLON) || tokenType() == KTokenType::QUALIFIER) {
    isSlice = true;

    if (tokenType() != KTokenType::COLON &&
        tokenType() != KTokenType::QUALIFIER &&
        tokenType() != KTokenType::CLOSE_BRACKET) {
      stop = parseExpression();
    }

    if (match(KTokenType::COLON) || match(KTokenType::QUALIFIER)) {
      if (tokenType() != KTokenType::CLOSE_BRACKET) {
        step = parseExpression();
      }
    }
  }

  match(KTokenType::CLOSE_BRACKET);
  std::unique_ptr<ASTNode> node;

  if (isSlice) {
    node = std::make_unique<SliceNode>(
        std::move(baseNode), start ? std::move(start.value()) : nullptr,
        stop ? std::move(stop.value()) : nullptr,
        step ? std::move(step.value()) : nullptr);
  } else {
    auto indexExpression = std::move(start.value());
    if (indexExpression->type != ASTNodeType::LITERAL &&
        indexExpression->type != ASTNodeType::IDENTIFIER &&
        indexExpression->type != ASTNodeType::FUNCTION_CALL &&
        indexExpression->type != ASTNodeType::BINARY_OPERATION &&
        indexExpression->type != ASTNodeType::METHOD_CALL) {
      throw SyntaxError(indexValueToken, "Invalid index value in indexer.");
    }

    node = std::make_unique<IndexingNode>(std::move(baseNode),
                                          std::move(indexExpression));
  }

  if (Operators.is_assignment_operator(tokenName())) {
    auto op = tokenName();
    next();
    auto initializer = parseExpression();
    node = std::make_unique<IndexAssignmentNode>(std::move(node), op,
                                                 std::move(initializer));
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseMemberAccess(
    std::unique_ptr<ASTNode> left) {
  while (tokenType() == KTokenType::DOT) {
    next();  // Consume '.'

    if (tokenType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(getErrorToken(),
                        "Expected identifier after '.' in member access.");
    }

    auto op = tokenName();
    auto memberName = kToken.getText();
    next();

    if (tokenType() == KTokenType::OPEN_PAREN) {
      left = parseFunctionCallOnMember(std::move(left), memberName, op);
    } else if (Operators.is_assignment_operator(tokenName())) {
      left = parseMemberAssignment(std::move(left), memberName);
    } else {
      left = std::make_unique<MemberAccessNode>(std::move(left), memberName);
    }
  }

  return left;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCallOnMember(
    std::unique_ptr<ASTNode> object, const k_string& methodName,
    const KName& type) {
  next();  // Consume '('

  // Parse function arguments
  std::vector<std::unique_ptr<ASTNode>> arguments;
  while (tokenType() != KTokenType::CLOSE_PAREN) {
    arguments.push_back(parseExpression());

    if (tokenType() == KTokenType::COMMA) {
      next();  // Consume ','
    } else if (tokenType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(getErrorToken(),
                        "Expected ')' or ',' in function call.");
    }
  }

  next();  // Consume the closing parenthesis ')'

  return std::make_unique<MethodCallNode>(std::move(object), methodName, type,
                                          std::move(arguments));
}

std::unique_ptr<ASTNode> Parser::parseMemberAssignment(
    std::unique_ptr<ASTNode> object, const k_string& memberName) {
  auto type = tokenName();
  next();

  auto initializer = parseExpression();
  return std::make_unique<MemberAssignmentNode>(std::move(object), memberName,
                                                type, std::move(initializer));
}

std::unique_ptr<ASTNode> Parser::parsePackAssignment(
    std::unique_ptr<ASTNode> baseNode) {
  /*
  a, b, c =< 0, 1, 2             # a = 0, b = 1, c = 2
  a, b =< get_zero_and_one()     # a = 0, b = 1
  */
  auto assignment = std::make_unique<PackAssignmentNode>();

  assignment->left.push_back(std::move(baseNode));

  while (kStream->canRead() && tokenType() == KTokenType::COMMA) {
    match(KTokenType::COMMA);
    if (tokenType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(getErrorToken(),
                        "Expected identifier in pack assignment variable set.");
    }

    auto identifierName = kToken.getText();
    if (mangledNames.find(identifierName) != mangledNames.end()) {
      identifierName = mangledNames[identifierName];
    }
    next();

    assignment->left.push_back(std::make_unique<IdentifierNode>(identifierName));
  }

  if (!matchSubType(KName::Ops_Assign)) {
    throw SyntaxError(getErrorToken(),
                      "Expected an unpack operator, '=<', in pack assignment.");
  }

  if (!matchSubType(KName::Ops_LessThan)) {
    throw SyntaxError(
        getErrorToken(),
        "Expected an unpack operator, '=<', in pack assignment.");
  }

  const size_t lhsLength = assignment->left.size();

  while (kStream->canRead() && hasValue()) {
    // we have everything we need.
    if (assignment->right.size() == lhsLength) {
      break;
    }

    auto rhs = parseExpression();
    assignment->right.push_back(std::move(rhs));

    if (tokenType() == KTokenType::COMMA) {
      next();
    } else {
      // we're at the end of the statement.
      break;
    }
  }

  return assignment;
}

std::unique_ptr<ASTNode> Parser::parseAssignment(
    std::unique_ptr<ASTNode> baseNode, const k_string& identifierName) {
  if (!Operators.is_assignment_operator(tokenName())) {
    throw SyntaxError(getErrorToken(),
                      "Expected an assignment operator in assignment.");
  }

  auto type = tokenName();
  next();

  auto initializer = parseExpression();

  return std::make_unique<AssignmentNode>(std::move(baseNode), identifierName,
                                          type, std::move(initializer));
}

std::unique_ptr<ASTNode> Parser::parseQualifiedIdentifier(
    const k_string& prefix) {
  if (!match(KTokenType::QUALIFIER)) {
    throw SyntaxError(getErrorToken(), "Expected a qualifier.");
  }

  if (tokenType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(getErrorToken(),
                      "Expected an identifier after qualifier.");
  }

  auto rightIdentifierName = kToken.getText();
  next();

  auto qualifiedName = prefix + "::" + rightIdentifierName;

  std::unique_ptr<ASTNode> qualifiedNode =
      std::make_unique<IdentifierNode>(qualifiedName);

  if (tokenType() == KTokenType::DOT) {
    qualifiedNode = parseMemberAccess(std::move(qualifiedNode));
  } else if (tokenType() == KTokenType::OPEN_PAREN) {
    qualifiedNode = parseFunctionCall(qualifiedName, tokenName());
  } else if (tokenType() == KTokenType::OPEN_BRACKET) {
    qualifiedNode = parseIndexing(qualifiedName);
  } else if (tokenType() == KTokenType::QUALIFIER) {
    qualifiedNode = parseQualifiedIdentifier(qualifiedName);
  }

  return qualifiedNode;
}

std::unique_ptr<ASTNode> Parser::parseIdentifier(bool packed) {
  bool isInstance = matchSubType(KName::KW_This);

  if (tokenType() != KTokenType::IDENTIFIER) {
    if (isInstance) {
      return std::make_unique<SelfNode>();
    }

    throw SyntaxError(getErrorToken(), "Expected an identifier.");
  }

  auto type = tokenName();
  auto identifierName = (isInstance ? "@" : "") + kToken.getText();

  if (mangledNames.find(identifierName) != mangledNames.end()) {
    identifierName = mangledNames[identifierName];
  }

  next();

  std::unique_ptr<ASTNode> node;

  if (isInstance) {
    node = std::make_unique<SelfNode>(identifierName);
  } else {
    node = std::make_unique<IdentifierNode>(identifierName);
  }

  if (tokenType() == KTokenType::DOT) {
    node = parseMemberAccess(std::move(node));
  } else if (tokenType() == KTokenType::OPEN_PAREN) {
    node = parseFunctionCall(identifierName, type);
  } else if (tokenType() == KTokenType::OPEN_BRACKET) {
    node = parseIndexing(identifierName);
  } else if (tokenType() == KTokenType::OPERATOR &&
             Operators.is_assignment_operator(tokenName())) {
    node = parseAssignment(std::move(node), identifierName);
  } else if (tokenType() == KTokenType::QUALIFIER &&
             peek().getType() == KTokenType::IDENTIFIER) {
    node = parseQualifiedIdentifier(identifierName);
  } else if (tokenType() == KTokenType::COMMA && !packed && lookAhead({ KName::Ops_Assign, KName::Ops_LessThan })) {
    node = parsePackAssignment(std::move(node));
  } else {
    node = std::make_unique<IdentifierNode>(identifierName);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
  auto node = parseLogicalOr();
  if (tokenType() == KTokenType::QUESTION) {
    next();  // Consume '?'
    auto trueBranch = parseExpression();
    if (!match(KTokenType::COLON)) {
      throw SyntaxError(getErrorToken(), "Expected ':' in ternary operation.");
    }
    auto falseBranch = parseExpression();  // Parse the false branch

    return std::make_unique<TernaryOperationNode>(
        std::move(node), std::move(trueBranch), std::move(falseBranch));
  }

  if (node->type == ASTNodeType::LAMBDA &&
      tokenType() == KTokenType::OPEN_PAREN) {
    node = parseLambdaCall(std::move(node));
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
  auto left = parseLogicalAnd();
  while (kStream->canRead() && tokenName() == KName::Ops_Or) {
    next();  // Consume '||'
    auto right = parseLogicalAnd();
    left = std::make_unique<BinaryOperationNode>(std::move(left), KName::Ops_Or,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
  auto left = parseBitwiseOr();
  while (kStream->canRead() && tokenName() == KName::Ops_And) {
    next();  // Consume '&&'
    auto right = parseBitwiseOr();
    left = std::make_unique<BinaryOperationNode>(
        std::move(left), KName::Ops_And, std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseOr() {
  auto left = parseBitwiseXor();
  while (kStream->canRead() && tokenName() == KName::Ops_BitwiseOr) {
    next();  // Consume '|'
    auto right = parseBitwiseXor();
    left = std::make_unique<BinaryOperationNode>(
        std::move(left), KName::Ops_BitwiseOr, std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseXor() {
  auto left = parseBitwiseAnd();
  while (kStream->canRead() && tokenName() == KName::Ops_BitwiseXor) {
    next();  // Consume '^'
    auto right = parseBitwiseAnd();
    left = std::make_unique<BinaryOperationNode>(
        std::move(left), KName::Ops_BitwiseXor, std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseAnd() {
  auto left = parseEquality();
  while (kStream->canRead() && tokenName() == KName::Ops_BitwiseAnd) {
    next();  // Consume '&'
    auto right = parseEquality();
    left = std::make_unique<BinaryOperationNode>(
        std::move(left), KName::Ops_BitwiseAnd, std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
  auto left = parseComparison();
  while (kStream->canRead() && Operators.is_equality_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseComparison();
    left = std::make_unique<BinaryOperationNode>(std::move(left), op,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseComparison() {
  auto left = parseBitshift();
  while (kStream->canRead() && Operators.is_comparison_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseBitshift();
    left = std::make_unique<BinaryOperationNode>(std::move(left), op,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseBitshift() {
  auto left = parseAdditive();
  while (kStream->canRead() && Operators.is_bitwise_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseAdditive();
    left = std::make_unique<BinaryOperationNode>(std::move(left), op,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseAdditive() {
  auto left = parseMultiplicative();
  while (kStream->canRead() && Operators.is_additive_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseMultiplicative();
    left = std::make_unique<BinaryOperationNode>(std::move(left), op,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicative() {
  auto left = parseUnary();
  while (kStream->canRead() && Operators.is_multiplicative_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseUnary();
    left = std::make_unique<BinaryOperationNode>(std::move(left), op,
                                                 std::move(right));
  }
  return left;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
  while (kStream->canRead() && Operators.is_unary_op(tokenName())) {
    auto op = tokenName();
    next();  // Skip operator
    auto right = parseUnary();
    return std::make_unique<UnaryOperationNode>(op, std::move(right));
  }
  auto primary = parsePrimary();
  return primary;  //return interpretValueInvocation(stream, frame, primary);
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
  std::unique_ptr<ASTNode> node;
  auto nodeToken = kToken;

  switch (tokenType()) {
    case KTokenType::IDENTIFIER:
    case KTokenType::KEYWORD:
      node = parseIdentifier(false);
      break;

    case KTokenType::LITERAL:
    case KTokenType::STRING:
    case KTokenType::TYPENAME:
      node = parseLiteral();
      break;

    case KTokenType::OPEN_PAREN: {
      next();  // Skip "("
      if (tokenType() == KTokenType::CLOSE_PAREN) {
        throw SyntaxError(getErrorToken(),
                          "Expected a value between '(' and ')'.");
      }
      auto result = parseExpression();
      match(KTokenType::CLOSE_PAREN);
      node = std::move(result);
    } break;

    case KTokenType::OPEN_BRACKET:
      node = parseListLiteral();
      break;

    case KTokenType::OPEN_BRACE:
      node = parseHashLiteral();
      break;

    default:
      if (tokenName() == KName::KW_Lambda) {
        node = parseLambda();
      } else {
        throw SyntaxError(getErrorToken(),
                          "Unexpected token '" + kToken.getText() + "'.");
      }
      break;
  }

  if (tokenType() == KTokenType::DOT) {
    node = parseMemberAccess(std::move(node));
  } else if (tokenType() == KTokenType::OPEN_BRACKET) {
    node = parseIndexing(std::move(node));
  }

  node->token = nodeToken;

  return node;
}

#endif
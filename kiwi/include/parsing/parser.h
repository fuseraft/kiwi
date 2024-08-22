#ifndef KIWI_PARSING_PARSER_H
#define KIWI_PARSING_PARSER_H

#include <memory>

#include "ast.h"
#include "keywords.h"
#include "tokens.h"
#include "tracing/error.h"
#include "typing/value.h"

class Parser {
 public:
  Parser() {}

  std::unique_ptr<ASTNode> parseTokenStream(k_stream& stream);

 private:
  std::unique_ptr<ASTNode> parseAssignment(const k_string& identifierName);
  std::unique_ptr<ASTNode> parseComment();
  std::unique_ptr<ASTNode> parseFunction();
  std::unique_ptr<ASTNode> parseFunctionCall(const k_string& identifierName);
  std::unique_ptr<ASTNode> parseStatement();
  std::unique_ptr<ASTNode> parseExpression();
  std::unique_ptr<ASTNode> parseLiteral();
  std::unique_ptr<ASTNode> parseListLiteral();
  std::unique_ptr<ASTNode> parseIndexing(const k_string& identifierName);
  std::unique_ptr<ASTNode> parseIdentifier();
  std::unique_ptr<ASTNode> parsePrint();

  // Utility methods to help with token matching and advancing the stream
  // Instead of passing streams everywhere, I'm going to just keep it local to the parser.
  Token next();
  bool match(KTokenType expectedType);
  bool matchSubType(KName expectedSubType);

  Token kToken = Token::createEmpty();
  k_stream kStream;
};

Token Parser::next() {
  kStream->next();
  kToken = kStream->current();
  return kToken;
}

bool Parser::match(KTokenType expectedType) {
  if (kToken.getType() == expectedType) {
    next();
    return true;
  }
  return false;
}

bool Parser::matchSubType(KName expectedSubType) {
  if (kToken.getSubType() == expectedSubType) {
    next();
    return true;
  }
  return false;
}

std::unique_ptr<ASTNode> Parser::parseTokenStream(k_stream& stream) {
  kStream = std::move(stream);
  kToken = kStream->current();  // Set to beginning.

  // Root program node
  auto root = std::make_unique<ProgramNode>();

  while (kToken.getType() != KTokenType::STREAM_END) {
    auto statement = parseStatement();
    if (statement) {
      root->statements.push_back(std::move(statement));
    }
  }

  return root;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
  switch (kToken.getType()) {
    case KTokenType::COMMENT:
      return parseComment();

    case KTokenType::KEYWORD:
      if (kToken.getSubType() == KName::KW_Method) {
        return parseFunction();
      } else if (kToken.getSubType() == KName::KW_PrintLn ||
                 kToken.getSubType() == KName::KW_Print) {
        return parsePrint();
      }
      // WIP: need to add more...
      break;

    case KTokenType::IDENTIFIER:
      return parseExpression();

    default:
      throw TokenStreamError(kToken, "Unexpected token in statement.");
  }

  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseComment() {
  match(KTokenType::COMMENT);
  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseFunction() {
  match(KTokenType::KEYWORD);  // Consume "fn"

  // Get function name
  if (kToken.getType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(kToken, "Expected identifier after 'fn'.");
  }

  std::string functionName = kToken.getText();
  next();  // Move past function name

  // Parse parameters
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> parameters;
  if (kToken.getType() == KTokenType::OPEN_PAREN) {
    next();  // Consume '('

    while (kToken.getType() != KTokenType::CLOSE_PAREN) {
      if (kToken.getType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(kToken, "Expected parameter name");
      }

      std::string paramName = kToken.getText();
      std::unique_ptr<ASTNode> defaultValue = nullptr;
      next();

      // Check for default value
      if (kToken.getType() == KTokenType::OPERATOR && kToken.getText() == "=") {
        next();  // Consume '='
        defaultValue = parseExpression();
      }

      parameters.emplace_back(paramName, std::move(defaultValue));

      if (kToken.getType() == KTokenType::COMMA) {
        next();
      } else if (kToken.getType() != KTokenType::CLOSE_PAREN) {
        throw SyntaxError(kToken, "Expected ',' or ')' in parameter list");
      }
    }

    next();  // Consume ')'
  }

  // Parse the function body
  std::vector<std::unique_ptr<ASTNode>> body;
  while (kToken.getSubType() != KName::KW_End) {
    body.push_back(parseStatement());
  }

  next();  // Consume 'end'

  auto functionDeclaration = std::make_unique<FunctionDeclarationNode>();
  functionDeclaration->name = functionName;
  functionDeclaration->parameters = std::move(parameters);
  functionDeclaration->body = std::move(body);
  return functionDeclaration;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(
    const k_string& identifierName) {
  // This is a function call
  next();  // Consume the '('

  // Parse function arguments
  std::vector<std::unique_ptr<ASTNode>> arguments;
  while (kToken.getType() != KTokenType::CLOSE_PAREN) {
    arguments.push_back(parseExpression());

    if (kToken.getType() == KTokenType::COMMA) {
      next();
    } else if (kToken.getType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(kToken, "Expected ')' or ',' in function call");
    }
  }

  next();  // Consume the closing parenthesis ')'

  return std::make_unique<FunctionCallNode>(identifierName,
                                            std::move(arguments));
}

std::unique_ptr<ASTNode> Parser::parsePrint() {
  auto printNode = std::make_unique<PrintNode>();
  printNode->printNewline = kToken.getSubType() == KName::KW_PrintLn;
  match(KTokenType::KEYWORD);  // Consume "print"|"println"
  printNode->expression = parseExpression();
  return printNode;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
  if (kToken.getType() == KTokenType::IDENTIFIER) {
    return parseIdentifier();
  } else if (kToken.getType() == KTokenType::LITERAL ||
             kToken.getType() == KTokenType::STRING) {
    return parseLiteral();
  } else if (kToken.getType() == KTokenType::OPEN_BRACKET) {
    return parseListLiteral();
  }
  // WIP: this is going to be fun.
  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseLiteral() {
  auto literalNode = std::make_unique<LiteralNode>();
  literalNode->value = kToken.getValue();
  next();  // Consume literal
  return literalNode;
}

std::unique_ptr<ASTNode> Parser::parseListLiteral() {
  std::vector<std::unique_ptr<ASTNode>> elements;

  match(KTokenType::OPEN_BRACKET);  // Consume '['

  while (kToken.getType() != KTokenType::CLOSE_BRACKET) {
    elements.push_back(parseExpression());

    if (kToken.getType() == KTokenType::COMMA) {
      next();  // Consume ','
    } else if (kToken.getType() != KTokenType::CLOSE_BRACKET) {
      throw SyntaxError(kToken, "Expected ']' or ',' in list literal");
    }
  }

  match(KTokenType::CLOSE_BRACKET);  // Consume ']'

  return std::make_unique<ListLiteralNode>(std::move(elements));
}

std::unique_ptr<ASTNode> Parser::parseIndexing(const k_string& identifierName) {
  next();                                    // Consume '['
  auto indexExpression = parseExpression();  // Parse the index expression
  match(KTokenType::CLOSE_BRACKET);          // Consume ']'

  return std::make_unique<IndexingNode>(identifierName,
                                        std::move(indexExpression));
}

std::unique_ptr<ASTNode> Parser::parseAssignment(
    const k_string& identifierName) {
  if (!Operators.is_assignment_operator(kToken.getSubType())) {
    throw SyntaxError(kToken, "Expected an assignment operator in assignment.");
  }

  auto type = kToken.getSubType();
  next();

  auto initializer = parseExpression();
  return std::make_unique<AssignmentNode>(identifierName, type,
                                          std::move(initializer));
}

std::unique_ptr<ASTNode> Parser::parseIdentifier() {
  if (kToken.getType() != KTokenType::IDENTIFIER) {
    throw SyntaxError(kToken, "Expected an identifier");
  }

  auto identifierName = kToken.getText();
  next();

  if (kToken.getType() == KTokenType::OPEN_PAREN) {
    return parseFunctionCall(identifierName);
  } else if (kToken.getType() == KTokenType::OPEN_BRACKET) {
    return parseIndexing(identifierName);
  } else if (kToken.getType() == KTokenType::OPERATOR) {
    return parseAssignment(identifierName);
  } else {
    return std::make_unique<IdentifierNode>(identifierName);
  }
}

#endif
#ifndef LEXER_H
#define LEXER_H

#include "../logging/logger.h"
#include "tokens.h"
#include "keywords.h"
#include <vector>

class Lexer {
public:
    Lexer(Logger &logger, const std::string &source, bool skipWhitespace = true) 
        : logger(logger), source(source), currentPosition(0), _skipWhitespace(skipWhitespace) {}

    Token getNextToken() {
        Token token = _getNextToken();
        logger.debug(token.info(), "Lexer::getNextToken");
        return token;
    }

    std::vector<Token> getAllTokens() {
        std::vector<Token> tokens;

        while (true) {
            Token token = getNextToken();
            
            if (token.type == TokenType::ENDOFFILE) {
                break;
            }

            tokens.push_back(token);
        }

        return tokens;
    }

private:
    Logger     &logger;
    std::string source;
    size_t      currentPosition;
    bool        _skipWhitespace;

    Token _getNextToken() {
        skipWhitespace();

        if (currentPosition >= source.length()) {
            return Token(TokenType::ENDOFFILE, "", 0);
        }

        char currentChar = source[currentPosition++];

        if (isalpha(currentChar)) {
            return parseIdentifier(currentChar);
        }
        else if (isdigit(currentChar)) {
            return parseLiteral(currentChar);
        }
        else if (currentChar == '"') {
            return parseString();
        }
        else if (currentChar == '#') {
            return parseComment();
        }
        else if (currentChar == '@') {
            return Token(TokenType::KEYWORD, "@", "@");
        }
        else if (currentChar == '$') {
            return Token(TokenType::OPERATOR, "$", "$");
        }
        else if (currentChar == '\n') {
            return Token(TokenType::NEWLINE, "\n", "\n");
        }
        else if (currentChar == '(') {
            return Token(TokenType::OPEN_PAREN, "(", "(");
        }
        else if (currentChar == ')') {
            return Token(TokenType::CLOSE_PAREN, ")", ")");
        }
        else if (currentChar == '\\') {
            if (currentPosition < source.length()) {
                char nextChar = source[currentPosition++];

                switch (nextChar) {
                    case 't':
                        return Token(TokenType::ESCAPED, "\t", "\t");
                    case 'n':
                        return Token(TokenType::ESCAPED, "\n", "\n");
                }
            }

            ++currentPosition;

            return Token(TokenType::IDENTIFIER, "\\", "\\");
        } 
        else {
            std::string s;
            s = currentChar;

            if (Operators.is_arithmetic_operator_char(currentChar) && currentPosition < source.length()) {
                char nextChar = source[currentPosition];

                if (nextChar == '=') {
                    s += nextChar;
                    ++currentPosition;
                }
            }

            return Token(TokenType::OPERATOR, s, s);
        }
    }

    void skipWhitespace() {
        if (!_skipWhitespace) {
            return;
        }

        while (currentPosition < source.length() && isspace(source[currentPosition])) {
            currentPosition++;
        }
    }

    Token parseKeyword(std::string& identifier) {
        TokenType tokenType;

        if (Keywords.is_conditional_keyword(identifier)) {
            tokenType = TokenType::CONDITIONAL;
        }

        return Token(tokenType, identifier, identifier);
    }

    Token parseIdentifier(char initialChar) {
        std::string identifier(1, initialChar);

        while (currentPosition < source.length() && isalnum(source[currentPosition])) {
            identifier += source[currentPosition++];
        }

        if (Keywords.is_keyword(identifier)) {
            return parseKeyword(identifier);
        }

        logger.debug(identifier, "Lexer::parseIdentifier");
        return Token(TokenType::IDENTIFIER, identifier, identifier);
    }

    Token parseLiteral(char initialChar) {
        std::string literal(1, initialChar);

        while (currentPosition < source.length() && (isdigit(source[currentPosition]) || source[currentPosition] == '.')) {
            literal += source[currentPosition++];
        }

        logger.debug(literal, "Lexer::parseLiteral");

        if (literal.find('.') != std::string::npos) {
            return Token(TokenType::LITERAL, literal, std::stod(literal));
        }
        else {
            return Token(TokenType::LITERAL, literal, std::stoi(literal));
        }
    }

    Token parseString() {
        std::string str;

        while (currentPosition < source.length() && source[currentPosition] != '"') {
            str += source[currentPosition++];
        }

        logger.debug(str, "Lexer::parseString");
        currentPosition++; // skip closing quote
        return Token(TokenType::STRING, str, str);
    }

    Token parseComment() {
        std::string comment;
        
        while (currentPosition < source.length() && source[currentPosition] != '\n') {
            comment += source[currentPosition++];
        }
        
        logger.debug(comment, "Lexer::parseComment");
        return Token(TokenType::COMMENT, comment, comment);
    }
};

#endif
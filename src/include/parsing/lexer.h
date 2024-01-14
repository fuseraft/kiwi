#ifndef LEXER_H
#define LEXER_H

#include "../logging/logger.h"
#include "tokens.h"
#include "keywords.h"
#include <vector>

class Lexer {
public:
    Lexer(Logger &logger, const std::string &source, bool skipWhitespace = true) 
        : logger(logger), source(source), currentPosition(0), _skipWhitespace(skipWhitespace) {
        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            lines.push_back(line);
        }
    }

    std::vector<Token> getAllTokens() {
        std::vector<Token> tokens;
        lineNumber = 0;
        linePosition = 0;

        while (true) {
            Token token = _getNextToken();
            
            if (token.getType() == TokenType::ENDOFFILE) {
                break;
            }

            tokens.push_back(token);
        }

        return tokens;
    }

    std::vector<std::string> getLines() {
        return lines;
    }

private:
    Logger     &logger;
    std::string source;
    size_t      currentPosition;
    bool        _skipWhitespace;
    int         lineNumber;
    int         linePosition;
    std::vector<std::string> lines;

    char getCurrentChar() {
        char c = source[currentPosition++];
        if (c == '\n') {
            lineNumber++;
            linePosition = 0;
        }
        else {
            linePosition++;
        }
        return c;
    }

    Token _getNextToken() {
        skipWhitespace();

        if (currentPosition >= source.length()) {
            return Token::create(TokenType::ENDOFFILE, "", 0, lineNumber, linePosition);
        }

        char currentChar = getCurrentChar();

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
            return Token::create(TokenType::KEYWORD, "@", lineNumber, linePosition);
        }
        else if (currentChar == '$') {
            return Token::create(TokenType::OPERATOR, "$", lineNumber, linePosition);
        }
        else if (currentChar == '\n') {
            return Token::create(TokenType::NEWLINE, "\n", lineNumber, linePosition);
        }
        else if (currentChar == '(') {
            return Token::create(TokenType::OPEN_PAREN, "(", lineNumber, linePosition);
        }
        else if (currentChar == ')') {
            return Token::create(TokenType::CLOSE_PAREN, ")", lineNumber, linePosition);
        }
        else if (currentChar == '\\') {
            if (currentPosition < source.length()) {
                char nextChar = getCurrentChar();

                switch (nextChar) {
                    case 't':
                        return Token::create(TokenType::ESCAPED, "\t", lineNumber, linePosition);
                    case 'n':
                        return Token::create(TokenType::ESCAPED, "\n", lineNumber, linePosition);
                }
            }

            getCurrentChar();

            return Token::create(TokenType::IDENTIFIER, "\\", lineNumber, linePosition);
        } 
        else {
            std::string s;
            s = currentChar;

            if (Operators.is_arithmetic_operator_char(currentChar) && currentPosition < source.length()) {
                char nextChar = source[currentPosition];

                if (nextChar == '=') {
                    s += nextChar;
                    getCurrentChar();
                }
            }

            return Token::create(TokenType::OPERATOR, s, lineNumber, linePosition);
        }
    }

    void skipWhitespace() {
        if (!_skipWhitespace) {
            return;
        }

        while (currentPosition < source.length() && isspace(source[currentPosition])) {
            getCurrentChar();
        }
    }

    Token parseKeyword(std::string& identifier) {
        TokenType tokenType;

        if (Keywords.is_conditional_keyword(identifier)) {
            tokenType = TokenType::CONDITIONAL;
        }

        return Token::create(tokenType, identifier, lineNumber, linePosition);
    }

    Token parseIdentifier(char initialChar) {
        std::string identifier(1, initialChar);

        while (currentPosition < source.length() && isalnum(source[currentPosition])) {
            identifier += getCurrentChar();
        }

        if (Keywords.is_keyword(identifier)) {
            return parseKeyword(identifier);
        }

        return Token::create(TokenType::IDENTIFIER, identifier, lineNumber, linePosition);
    }

    Token parseLiteral(char initialChar) {
        std::string literal(1, initialChar);

        while (currentPosition < source.length() && (isdigit(source[currentPosition]) || source[currentPosition] == '.')) {
            literal += getCurrentChar();
        }

        if (literal.find('.') != std::string::npos) {
            return Token::create(TokenType::LITERAL, literal, std::stod(literal), lineNumber, linePosition);
        }
        else {
            return Token::create(TokenType::LITERAL, literal, std::stoi(literal), lineNumber, linePosition);
        }
    }

    Token parseString() {
        std::string str;

        while (currentPosition < source.length() && source[currentPosition] != '"') {
            str += getCurrentChar();
        }

        getCurrentChar(); // skip closing quote
        return Token::create(TokenType::STRING, str, lineNumber, linePosition);
    }

    Token parseComment() {
        std::string comment;
        
        while (currentPosition < source.length() && source[currentPosition] != '\n') {
            comment += getCurrentChar();
        }
        
        return Token::create(TokenType::COMMENT, comment, lineNumber, linePosition);
    }
};

#endif
#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "tokens.h"

class Lexer {
public:
    Lexer(const std::string& source) : source(source), currentPosition(0) {}

    Token getNextToken() {
        skipWhitespace();

        if (currentPosition >= source.length())
            return Token(TokenType::ENDOFFILE, 0);

        char currentChar = source[currentPosition++];

        if (isalpha(currentChar))
            return parseIdentifier(currentChar);
        else if (isdigit(currentChar))
            return parseLiteral(currentChar);
        else if (currentChar == '"')
            return parseString();
        else if (currentChar == '#')
            return parseComment();
        else if (currentChar == '@')
            return Token(TokenType::KEYWORD, "@");
        else if (currentChar == '$')
            return Token(TokenType::OPERATOR, "$");
        else if (currentChar == '\n')
            return Token(TokenType::NEWLINE, "\n");
        else {
            std::string s;
            s = currentChar;
            return Token(TokenType::OPERATOR, s);
        }
    }

    std::vector<Token> getAllTokens() {
        std::vector<Token> tokens;
        while (true) {
            Token token = getNextToken();
            if (token.type == TokenType::ENDOFFILE)
                break;
            tokens.push_back(token);
        }

        return tokens;
    }

private:
    void skipWhitespace() {
        while (currentPosition < source.length() && isspace(source[currentPosition]))
            currentPosition++;
    }

    Token parseIdentifier(char initialChar) {
        std::string identifier(1, initialChar);
        while (currentPosition < source.length() && isalnum(source[currentPosition]))
            identifier += source[currentPosition++];
        return Token(TokenType::IDENTIFIER, identifier);
    }

    Token parseLiteral(char initialChar) {
        std::string literal(1, initialChar);
        while (currentPosition < source.length() && (isdigit(source[currentPosition]) || source[currentPosition] == '.'))
            literal += source[currentPosition++];

        if (literal.find('.') != std::string::npos)
            return Token(TokenType::LITERAL, std::stod(literal));
        else
            return Token(TokenType::LITERAL, std::stoi(literal));
    }

    Token parseString() {
        std::string str;
        while (currentPosition < source.length() && source[currentPosition] != '"')
            str += source[currentPosition++];
        currentPosition++; // skip closing quote
        return Token(TokenType::STRING, str);
    }

    Token parseComment() {
        std::string comment;
        while (currentPosition < source.length() && source[currentPosition] != '\n')
            comment += source[currentPosition++];
        return Token(TokenType::COMMENT, comment);
    }

    std::string source;
    size_t currentPosition;
};

#endif
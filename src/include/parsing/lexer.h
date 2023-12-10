#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "tokens.h"

class Lexer {
public:
    Lexer(const std::string& source, bool skipWhitespace = true) : source(source), currentPosition(0), _skipWhitespace(skipWhitespace) {}

    Token getNextToken() {
        if (_skipWhitespace)
            skipWhitespace();

        if (currentPosition >= source.length())
            return Token(TokenType::ENDOFFILE, "", 0);

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
            return Token(TokenType::KEYWORD, "@", "@");
        else if (currentChar == '$')
            return Token(TokenType::OPERATOR, "$", "$");
        else if (currentChar == '\n')
            return Token(TokenType::NEWLINE, "\n", "\n");
        else if (currentChar == '\\') {
            if (currentPosition < source.length()) {
                char nextChar = source[currentPosition];
                switch (nextChar) {
                    case 't':
                        return Token(TokenType::ESCAPED, "\t", "\t");                    
                    case 'n':
                        return Token(TokenType::ESCAPED, "\n", "\n");
                }
            }
            return Token(TokenType::IDENTIFIER, "\\", "\\");
        }
        else {
            std::string s;
            s = currentChar;
            return Token(TokenType::OPERATOR, s, s);
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
    std::string source;
    size_t currentPosition;
    bool _skipWhitespace;

    void skipWhitespace() {
        while (currentPosition < source.length() && isspace(source[currentPosition]))
            currentPosition++;
    }

    Token parseIdentifier(char initialChar) {
        std::string identifier(1, initialChar);
        while (currentPosition < source.length() && isalnum(source[currentPosition]))
            identifier += source[currentPosition++];
        return Token(TokenType::IDENTIFIER, identifier, identifier);
    }

    Token parseLiteral(char initialChar) {
        std::string literal(1, initialChar);
        while (currentPosition < source.length() && (isdigit(source[currentPosition]) || source[currentPosition] == '.'))
            literal += source[currentPosition++];

        if (literal.find('.') != std::string::npos)
            return Token(TokenType::LITERAL, literal, std::stod(literal));
        else
            return Token(TokenType::LITERAL, literal, std::stoi(literal));
    }

    Token parseString() {
        std::string str;
        while (currentPosition < source.length() && source[currentPosition] != '"')
            str += source[currentPosition++];
        currentPosition++; // skip closing quote
        return Token(TokenType::STRING, str, str);
    }

    Token parseComment() {
        std::string comment;
        while (currentPosition < source.length() && source[currentPosition] != '\n')
            comment += source[currentPosition++];
        return Token(TokenType::COMMENT, comment, comment);
    }
};

#endif
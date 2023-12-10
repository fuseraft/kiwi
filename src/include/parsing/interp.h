#ifndef INTERP_H
#define INTERP_H

#include <map>
#include <variant>
#include "tokens.h"
#include "lexer.h"

class Interpreter {
    // WIP: this will be the new interpreter.
    public:
        Interpreter() : variables() {}

        void interpret(std::vector<Token> tokens) {
            setTokens(tokens);
            while (_position < _end) {
                if (_position + 1 == _end)
                    break;

                next();

                // Don't parse comments.
                if (current().type == TokenType::COMMENT)
                    continue;

                if (current().type == TokenType::KEYWORD && current().text == "@") {
                    interpretAssignment();
                }
                else if (current().type == TokenType::IDENTIFIER && current().text == "println") {
                    interpretPrintln();
                }
            }
        }

    private:
        std::map<std::string, std::variant<int, double, bool, std::string>> variables;
        std::vector<Token> _tokens;
        int _position;
        int _end;

        Token current() {
            return _tokens[_position];
        }

        Token next() {
            if (_position + 1 < _end)
                return _tokens[++_position];
            return current();
        }

        Token peek() {
            if (_position + 1 < _end)
                return _tokens[_position + 1];
            return current();
        }

        void setTokens(std::vector<Token> tokens) {
            _tokens = tokens;
            _position = -1;
            _end = _tokens.size();
        }

        std::string evaluateString() {
            std::string output;

            // TODO: an error should occur here.
            if (current().value_type != ValueType::String)
                return output;

            std::string input = current().toString();

            // Don't skip whitespace.
            Lexer lexer(input, false);
            std::vector<Token> eval = lexer.getAllTokens();

            std::ostringstream string;
            int position = -1;
            int end = eval.size();
            while (position + 1 < end) {
                ++position;
                Token token = eval[position];
                
                // string interpolation
                if (token.text == "$" && position + 1 < end && eval[position + 1].text == "{") {
                    position += 2; // Skip "${"

                    token = eval[position];
                    // loop to the end
                    while (token.text != "}") {
                        if (token.text == "@") {
                            token = eval[++position];
                            if (variables.find(token.text) != variables.end()) {
                                auto v = variables[token.text];
                                ValueType vt = get_value_type(v);
                                if (vt == ValueType::Integer)
                                    string << std::get<int>(v);
                                else if (vt == ValueType::Double)
                                    string << std::get<double>(v);
                                else if (vt == ValueType::Boolean)
                                    string << std::boolalpha << std::get<bool>(v);
                                else if (vt == ValueType::String)
                                    string << std::get<std::string>(v);
                            }
                        }
                        token = eval[++position];
                    }

                    continue;
                }
                else if (token.type == TokenType::ESCAPED)
                {
                    string << token.text;
                    ++position;
                    continue;
                }

                string << token.text;
            }
            output = string.str();

            return output;
        }

        void interpretPrintln() {
            next(); // skip the "println"

            if (current().type == TokenType::STRING && current().value_type == ValueType::String) {
                std::cout << evaluateString();
            }
            else {
                std::cout << "debug";
            }

            std::cout << std::endl;
        }

        void interpretAssignment() {
            next(); // skip the "@"

            if (current().type == TokenType::IDENTIFIER && current().value_type == ValueType::String) {
                std::string name = current().toString();
                next();

                if (current().type == TokenType::OPERATOR && current().value_type == ValueType::String) {
                    std::string op = current().toString();
                    next();

                    if (op == Operators.Assign) {
                        if (current().value_type == ValueType::String)
                            variables[name] = current().toString();
                        else if (current().value_type == ValueType::Double)
                            variables[name] = current().toDouble();
                        else if (current().value_type == ValueType::Integer)
                            variables[name] = current().toInteger();
                        else if (current().value_type == ValueType::Boolean)
                            variables[name] = current().toBoolean();
                    }
                }
            }
        }
};

#endif
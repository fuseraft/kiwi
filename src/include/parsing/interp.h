#ifndef INTERP_H
#define INTERP_H

#include <map>
#include <variant>
#include "tokens.h"
#include "lexer.h"
#include "interp_visitor.h"

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

                if (current().type == TokenType::KEYWORD && current().text == Symbols.DeclVar)
                    interpretAssignment();
                else if (current().type == TokenType::IDENTIFIER && current().text == Keywords.PrintLn)
                    interpretPrint(true);
                else if (current().type == TokenType::IDENTIFIER && current().text == Keywords.Print)
                    interpretPrint();
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

        void setTokens(const std::vector<Token> tokens) {
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
                
                if (token.text == Symbols.Interpolate && position + 1 < end && eval[position + 1].text == Symbols.OpenCurlyBrace) {
                    evaluateStringInterpolation(position, token, eval, string);
                    continue;
                }
                else if (token.type == TokenType::ESCAPED)
                {
                    string << token.text;
                    continue;
                }

                string << token.text;
            }
            output = string.str();

            return output;
        }

        void evaluateStringInterpolation(int &position, Token &token,
                                         std::vector<Token> &eval,
                                         std::ostringstream &string) {
            position += 2; // Skip "${"

            token = eval[position];
            // loop to the end
            while (token.text != Symbols.CloseCurlyBrace) {
                if (token.text == Symbols.DeclVar) {
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
        }

        void interpretPrint(bool printNewLine = false) {
            next(); // skip the "print"

            if (current().type == TokenType::STRING && current().value_type == ValueType::String)
                std::cout << evaluateString();
            else if (current().type == TokenType::KEYWORD && current().text == Symbols.DeclVar) {
                next();
                std::string name = current().text;
                if (variables.find(name) != variables.end()) {
                    std::variant<int, double, bool, std::string> value = interpretExpression();
                    ValueType vtype = get_value_type(value);
                    if (vtype == ValueType::Integer)
                        std::cout << std::get<int>(value);
                    else if (vtype == ValueType::Double)
                        std::cout << std::get<double>(value);
                    else if (vtype == ValueType::Boolean)
                        std::cout << std::boolalpha << std::get<bool>(value);
                    else if (vtype == ValueType::String)
                        std::cout << std::get<std::string>(value);
                }
                else
                    throw std::runtime_error("Unknown term `" + name + "`");
            }
            else {
                std::ostringstream error;
                error << "Not implemented `" << Keywords.Print << "`|`" << Keywords.PrintLn << "` for type `"
                    << get_value_type_string(current().value_type) << std::endl
                    << "Value: `" << current().text << "`";
                throw std::runtime_error(error.str());
            }

            if (printNewLine)
                std::cout << std::endl;
        }

        void interpretAssignment() {
            next(); // Skip the "@"

            if (current().type == TokenType::IDENTIFIER && current().value_type == ValueType::String) {
                std::string name = current().toString();
                next();

                if (current().type == TokenType::OPERATOR && current().value_type == ValueType::String) {
                    std::string op = current().toString();
                    next();

                    if (op == Operators.Assign) {
                        std::variant<int, double, bool, std::string> value = interpretExpression();
                        variables[name] = value;
                    }
                }
            }
        }

        // WIP: expression interpreter
        std::variant<int, double, bool, std::string> interpretExpression() {
            std::variant<int, double, bool, std::string> result = interpretTerm();

            while (current().type == TokenType::OPERATOR && current().value_type == ValueType::String) {
                std::string op = current().toString();
                next();

                std::variant<int, double, bool, std::string> nextTerm = interpretTerm();
                
                if (op == Operators.Add) {
                    result = std::visit(AddVisitor(), result, nextTerm);
                } else if (op == Operators.Subtract) {
                    result = std::visit(SubtractVisitor(), result, nextTerm);
                } else if (op == Operators.Multiply) {
                    result = std::visit(MultiplyVisitor(), result, nextTerm);
                } else if (op == Operators.Divide) {
                    result = std::visit(DivideVisitor(), result, nextTerm);
                } else if (op == Operators.Exponent) {
                    result = std::visit(PowerVisitor(), result, nextTerm);
                }
            }

            return result;
        }

        std::variant<int, double, bool, std::string> interpretTerm() {
            if (current().type == TokenType::OPEN_PAREN) {
                next(); // Skip the '('
                std::variant<int, double, bool, std::string> result = interpretExpression();
                next(); // Skip the ')'
                return result;
            } else if (current().type == TokenType::IDENTIFIER) {
                std::string variableName = current().toString();
                next();
                if (variables.find(variableName) != variables.end())
                    return variables[variableName];
            } else if (current().value_type == ValueType::Double) {
                double doubleValue = current().toDouble();
                next();
                return doubleValue;
            } else if (current().value_type == ValueType::Integer) {
                int intValue = current().toInteger();
                next();
                return intValue;
            } else if (current().value_type == ValueType::String) {
                std::string stringValue = evaluateString();
                return stringValue;
            }

            // Handle other cases or raise an error for unsupported types
            return 0; // Placeholder for unsupported types
        }
};

#endif
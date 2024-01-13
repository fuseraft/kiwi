#ifndef INTERP_H
#define INTERP_H

#include "../logging/logger.h"
#include "value_type.h"
#include "interp_visitor.h"
#include "lexer.h"
#include "tokens.h"
#include "keywords.h"
#include <map>
#include <variant>

class Interpreter {
    // WIP: this is the new interpreter.
public:
    Interpreter(Logger &logger) : logger(logger), variables() {}

    void interpret(std::vector<Token> tokens) {
        if (tokens.size() == 0) {
            return;
        }

        setTokens(tokens);

        while (_position < _end) {
            if (_position + 1 == _end) {
                break;
            }

            // Don't parse comments.
            if (current().type == TokenType::COMMENT) {
                next();
                continue;
            }

            if (current().type == TokenType::KEYWORD && current().text == Symbols.DeclVar) {
                interpretAssignment();
                continue;
            }
            
            if (current().type == TokenType::IDENTIFIER && (current().text == Keywords.PrintLn || current().text == Keywords.Print)) {
                interpretPrint(current().text == Keywords.PrintLn);
            }
            else {
                logger.debug("Unhandled token " + current().info(), "Interpreter::interpret");
            }

            next();
        }
    }

private:
    Logger &logger;
    std::map<std::string, std::variant<int, double, bool, std::string>> variables;
    std::vector<Token> _tokens;
    int _position;
    int _end;

    Token current() { return _tokens[_position]; }

    Token next() {
        if (_position + 1 < _end) {
            logger.debug("Next token: " + _tokens[_position + 1].info(), "Interpreter::next");
            ++_position;
        }

        return _tokens[_position];
    }

    Token peek() {
        if (_position + 1 < _end) {
            return _tokens[_position + 1];
        }

        return _tokens[_position];
    }

    void setTokens(const std::vector<Token> tokens) {
        _tokens = tokens;
        _position = 0;
        _end = _tokens.size();
    }

    std::string evaluateString() {
        std::string output;

        // TODO: an error should occur here.
        if (current().value_type != ValueType::String) {
            return output;
        }

        std::string input = current().toString();

        // Don't skip whitespace.
        Lexer lexer(logger, input, false);
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
            else if (token.type == TokenType::ESCAPED) {
                string << token.text;
                continue;
            }

            string << token.text;
        }

        output = string.str();
        logger.debug(output, "Interpreter::evaluateString");

        return output;
    }

    void evaluateStringInterpolation(
        int &position, Token &token, std::vector<Token> &eval,
        std::ostringstream &string) {
        position += 2; // Skip "${"

        token = eval[position];
        // loop to the end
        while (token.text != Symbols.CloseCurlyBrace) {
            if (token.text == Symbols.DeclVar) {
                token = eval[++position];

                if (variables.find(token.text) != variables.end()) {
                    auto v  = variables[token.text];
                    ValueType vt = get_value_type(v);

                    if (vt == ValueType::Integer) {
                        string << std::get<int>(v);
                    }
                    else if (vt == ValueType::Double) {
                        string << std::get<double>(v);
                    }
                    else if (vt == ValueType::Boolean) {
                        string << std::boolalpha << std::get<bool>(v);
                    }
                    else if (vt == ValueType::String) {
                        string << std::get<std::string>(v);
                    }
                    else {
                        logger.debug("Unhandled value type: " + token.info(), "Interpreter::evaluateStringInterpolation");
                    }
                } 
                else {
                    logger.debug("Variable undefined: " + token.info(), "Interpreter::evaluateStringInterpolation");
                }
            } 
            else {
                logger.debug("Unhandled token: " + token.info(), "Interpreter::evaluateStringInterpolation");
            }

            token = eval[++position];
        }
    }

    void interpretPrint(bool printNewLine = false) {
        next(); // skip the "print"

        if (current().type == TokenType::STRING && current().value_type == ValueType::String) {
            std::cout << evaluateString();
        }
        else if (current().type == TokenType::KEYWORD && current().text == Symbols.DeclVar) {
            next();
            std::string name = current().text;

            if (variables.find(name) != variables.end()) {
                std::variant<int, double, bool, std::string> value =
                    interpretExpression();
                ValueType vtype = get_value_type(value);

                if (vtype == ValueType::Integer) {
                    std::cout << std::get<int>(value);
                }
                else if (vtype == ValueType::Double) {
                    std::cout << std::get<double>(value);
                }
                else if (vtype == ValueType::Boolean) {
                    std::cout << std::boolalpha << std::get<bool>(value);
                }
                else if (vtype == ValueType::String) {
                    std::cout << std::get<std::string>(value);
                }
                else {
                    logger.debug("Unhandled value type: " + current().info(), "Interpreter::interpretPrint");
                }
            }
            else {
                logger.error("Unknown term `" + name + "`", "Interpreter::interpretPrint");
                throw std::runtime_error("Unknown term `" + name + "`");
            }
        }
        else {
            std::ostringstream error;
            error << "Not implemented `" << Keywords.Print << "`|`"
                  << Keywords.PrintLn << "` for type `"
                  << get_value_type_string(current().value_type) << std::endl
                  << "Value: `" << current().text << "`";
            logger.error(error.str(), "Interpreter::interpretPrint");
            throw std::runtime_error(error.str());
        }

        if (printNewLine) {
            std::cout << std::endl;
        }
    }

    void handleAssignment(std::string& name, std::string& op) {
        std::variant<int, double, bool, std::string> value = interpretExpression();

        if (op == Operators.Assign) {
            variables[name] = value;
            return;
        }

        if (variables.find(name) == variables.end()) {
            throw std::runtime_error("Variable undefined: `" + name + "`");
        }

        std::variant<int, double, bool, std::string> currentValue = variables[name];
        
        if (op == Operators.AddAssign) {
            currentValue = std::visit(AddVisitor(), currentValue, value);
        }
        else if (op == Operators.SubtractAssign) {
            currentValue = std::visit(SubtractVisitor(), currentValue, value);
        }
        else if (op == Operators.MultiplyAssign) {
            currentValue = std::visit(MultiplyVisitor(), currentValue, value);
        }
        else if (op == Operators.DivideAssign) {
            currentValue = std::visit(DivideVisitor(), currentValue, value);
        }
        else if (op == Operators.ExponentAssign) {
            currentValue = std::visit(PowerVisitor(), currentValue, value);
        }

        variables[name] = currentValue;
    }

    void interpretAssignment() {
        next(); // Skip the "@"

        if (current().type == TokenType::IDENTIFIER && current().value_type == ValueType::String) {
            std::string name = current().toString();
            next();

            if (current().type == TokenType::OPERATOR && current().value_type == ValueType::String) {
                std::string op = current().toString();
                next();

                if (Operators.is_assignment_operator(op)) {
                    handleAssignment(name, op);
                }
                else {
                    logger.debug("Unknown operator `" + op + "`", "Interpreter::interpretAssignment");
                }
            } 
            else {
                logger.debug("Unimplemented token `" + current().info() + "`", "Interpreter::interpretAssignment");
            }
        } 
        else {
            logger.debug("Unimplemented token `" + current().info() + "`", "Interpreter::interpretAssignment");
        }
    }

    // WIP: expression interpreter
    std::variant<int, double, bool, std::string> interpretExpression() {
        std::variant<int, double, bool, std::string> result = interpretTerm();

        while (current().type == TokenType::OPERATOR) {
            std::string op = current().toString();
            next();

            std::variant<int, double, bool, std::string> nextTerm = interpretTerm();

            if (op == Operators.Add) {
                result = std::visit(AddVisitor(), result, nextTerm);
            }
            else if (op == Operators.Subtract) {
                result = std::visit(SubtractVisitor(), result, nextTerm);
            }
            else if (op == Operators.Multiply) {
                result = std::visit(MultiplyVisitor(), result, nextTerm);
            }
            else if (op == Operators.Divide) {
                result = std::visit(DivideVisitor(), result, nextTerm);
            }
            else if (op == Operators.Exponent) {
                result = std::visit(PowerVisitor(), result, nextTerm);
            }
            else {
                logger.debug("Unimplemented operator `" + op + "`", "Interpreter::interpretExpression");
            }
        }

        return result;
    }

    std::variant<int, double, bool, std::string> interpretTerm() {
        if (current().text == Symbols.DeclVar) {
            next();
        }

        if (current().type == TokenType::OPEN_PAREN) {
            next(); // Skip the '('
            std::variant<int, double, bool, std::string> result = interpretExpression();
            next(); // Skip the ')'
            return result;
        }
        else if (current().type == TokenType::IDENTIFIER) {
            std::string variableName = current().toString();
            if (variables.find(variableName) != variables.end()) {
                next();
                return variables[variableName];
            }
        }
        else if (current().value_type == ValueType::Double) {
            double doubleValue = current().toDouble();
            return doubleValue;
        } 
        else if (current().value_type == ValueType::Integer) {
            int intValue = current().toInteger();
            return intValue;
        } 
        else if (current().value_type == ValueType::String) {
            std::string stringValue = evaluateString();
            return stringValue;
        }

        logger.debug("Unimplemented value type `" + current().info() + "`", "Interpreter::interpretTerm");

        return 0; // Placeholder for unsupported types
    }
};

#endif
#ifndef INTERP_H
#define INTERP_H

#include <map>
#include <variant>
#include "errors/error.h"
#include "logging/logger.h"
#include "typing/value_type.h"
#include "math/math_visitor.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "parsing/keywords.h"

class Interpreter {
    // WIP: this is the new interpreter.
public:
    Interpreter(Logger &logger) : logger(logger), variables() {}

    int interpret(Lexer &lexer) {
        _tokens = lexer.getAllTokens();
        _lines = lexer.getLines();
        _position = 0;
        _end = _tokens.size();

        if (_end == 0) {
            return 0;
        }

        while (_position < _end) {
            if (_position + 1 == _end) {
                break;
            }

            try {
                // Don't parse comments.
                if (current().getType() == TokenType::COMMENT) {
                    next();
                    continue;
                }

                if (current().getType() == TokenType::KEYWORD && current().getText() == Symbols.DeclVar) {
                    interpretAssignment();
                    continue;
                }
                
                if (current().getType() == TokenType::IDENTIFIER && (current().getText() == Keywords.PrintLn || current().getText() == Keywords.Print)) {
                    interpretPrint(current().getText() == Keywords.PrintLn);
                }
                else if (current().getType() == TokenType::CONDITIONAL) {
                    interpretConditional();
                }
                else {
                    logger.debug("Unhandled token " + current().info(), "Interpreter::interpret");
                }

                next();   
            }
            catch (const UslangError &e) {
                return handleError(e);
            }
            catch (const std::exception &e) {
                throw;
            }
        }

        return 0;
    }

private:
    Logger &logger;
    std::map<std::string, std::variant<int, double, bool, std::string>> variables;
    std::vector<Token> _tokens;
    std::vector<std::string> _lines;
    int _position;
    int _end;
    
    int handleError(const UslangError &e) {
        const Token& token = e.getToken();
        std::string message = e.getMessage();
        std::string tokenText = token.getText();
        int lineNumber = token.getLineNumber();
        int linePosition = token.getLinePosition();

        std::string line = _lines.at(lineNumber);
        int length = line.length();
        
        std::cerr << "Exception: " << e.getError() << std::endl;
        if (!message.empty()) {
            std::cerr << "Message:   " << e.getMessage() << std::endl;
        }
        std::cerr << "Location:  Line " << 1 + lineNumber << ", Column " << linePosition << "." << std::endl;
        std::cerr << "Code: " << std::endl 
            << "```" << std::endl 
            << line << std::endl;

        for (int i = 0; i < length; ++i) {
            std::cerr << (i == linePosition - 1 ? "^" : " ");
        }
        std::cerr << std::endl << "```" << std::endl << std::endl;

        return 1;
    }

    Token current() { return _tokens.at(_position); }

    Token next() {
        if (_position + 1 < _end) {
            ++_position;
        }

        return _tokens.at(_position);
    }

    Token peek() {
        if (_position + 1 < _end) {
            return _tokens.at(_position + 1);
        }

        return _tokens.at(_position);
    }

    void interpretAssignment() {
        next(); // Skip the "@"

        if (current().getType() == TokenType::IDENTIFIER && current().getValueType() == ValueType::String) {
            std::string name = current().toString();
            next();

            if (current().getType() == TokenType::OPERATOR && current().getValueType() == ValueType::String) {
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

    void interpretConditional() {
        if (current().getText() != Keywords.If) {
            throw SyntaxError(current());
        }
    }

    void interpretPrint(bool printNewLine = false) {
        next(); // skip the "print"

        if (current().getType() == TokenType::STRING && current().getValueType() == ValueType::String) {
            std::cout << evaluateString();
        }
        else if (current().getType() == TokenType::KEYWORD && current().getText() == Symbols.DeclVar) {
            next();
            std::string name = current().getText();

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
                    throw UslangError(current(), "Unhandled value type: " + current().info());
                }
            }
            else {
                throw UslangError(current(), "Unknown term `" + name + "`");
            }
        }
        else {
            std::ostringstream error;
            error << "Not implemented `" << Keywords.Print << "`|`"
                  << Keywords.PrintLn << "` for type `"
                  << get_value_type_string(current().getValueType()) << std::endl
                  << "Value: `" << current().getText() << "`";
            throw UslangError(current(), error.str());
        }

        if (printNewLine) {
            std::cout << std::endl;
        }
    }

    // WIP: expression interpreter
    std::variant<int, double, bool, std::string> interpretExpression() {
        Token tokenTerm = current();
        std::variant<int, double, bool, std::string> result = interpretTerm(tokenTerm);

        while (current().getType() == TokenType::OPERATOR) {
            std::string op = current().toString();
            next();

            std::variant<int, double, bool, std::string> nextTerm = interpretTerm(tokenTerm);

            if (op == Operators.Add) {
                result = std::visit(AddVisitor(tokenTerm), result, nextTerm);
            }
            else if (op == Operators.Subtract) {
                result = std::visit(SubtractVisitor(tokenTerm), result, nextTerm);
            }
            else if (op == Operators.Multiply) {
                result = std::visit(MultiplyVisitor(tokenTerm), result, nextTerm);
            }
            else if (op == Operators.Divide) {
                result = std::visit(DivideVisitor(tokenTerm), result, nextTerm);
            }
            else if (op == Operators.Exponent) {
                result = std::visit(PowerVisitor(tokenTerm), result, nextTerm);
            }
        }

        return result;
    }

    std::variant<int, double, bool, std::string> interpretTerm(Token& termToken) {
        if (current().getText() == Symbols.DeclVar) {
            next();
        }
        
        termToken = current();

        if (current().getType() == TokenType::OPEN_PAREN) {
            next(); // Skip the '('
            std::variant<int, double, bool, std::string> result = interpretExpression();
            next(); // Skip the ')'
            return result;
        }
        else if (current().getType() == TokenType::IDENTIFIER) {
            std::string variableName = current().toString();
            if (variables.find(variableName) != variables.end()) {
                next();
                return variables[variableName];
            }
        }
        else if (current().getValueType() == ValueType::Double) {
            double doubleValue = current().toDouble();
            return doubleValue;
        } 
        else if (current().getValueType() == ValueType::Integer) {
            int intValue = current().toInteger();
            return intValue;
        } 
        else if (current().getValueType() == ValueType::String) {
            std::string stringValue = evaluateString();
            return stringValue;
        }

        return 0; // Placeholder for unsupported types
    }

    std::string evaluateString() {
        std::string output;

        // TODO: an error should occur here.
        if (current().getValueType() != ValueType::String) {
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
            Token token = eval.at(position);

            if (token.getText() == Symbols.Interpolate && position + 1 < end && eval.at(position + 1).getText() == Symbols.OpenCurlyBrace) {
                evaluateStringInterpolation(position, token, eval, string);
                continue;
            } 
            else if (token.getType() == TokenType::ESCAPED) {
                string << token.getText();
                continue;
            }

            string << token.getText();
        }

        output = string.str();

        return output;
    }

    void evaluateStringInterpolation(
        int &position, Token &token, std::vector<Token> &eval,
        std::ostringstream &string) {
        position += 2; // Skip "${"

        token = eval.at(position);
        // loop to the end
        while (token.getText() != Symbols.CloseCurlyBrace) {
            if (token.getText() == Symbols.DeclVar) {
                token = eval.at(++position);

                if (variables.find(token.getText()) != variables.end()) {
                    auto v  = variables[token.getText()];
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
                } 
                else {
                    logger.debug("Variable undefined: " + token.info(), "Interpreter::evaluateStringInterpolation");
                }
            } 
            else {
                logger.debug("Unhandled token: " + token.info(), "Interpreter::evaluateStringInterpolation");
            }

            token = eval.at(++position);
        }
    }

    void handleAssignment(std::string& name, std::string& op) {
        std::variant<int, double, bool, std::string> value = interpretExpression();

        if (op == Operators.Assign) {
            variables[name] = value;
            return;
        }

        if (variables.find(name) == variables.end()) {
            throw VariableUndefinedError(current(), name);
        }

        std::variant<int, double, bool, std::string> currentValue = variables[name];

        if (op == Operators.AddAssign) {
            currentValue = std::visit(AddVisitor(current()), currentValue, value);
        }
        else if (op == Operators.SubtractAssign) {
            currentValue = std::visit(SubtractVisitor(current()), currentValue, value);
        }
        else if (op == Operators.MultiplyAssign) {
            currentValue = std::visit(MultiplyVisitor(current()), currentValue, value);
        }
        else if (op == Operators.DivideAssign) {
            currentValue = std::visit(DivideVisitor(current()), currentValue, value);
        }
        else if (op == Operators.ExponentAssign) {
            currentValue = std::visit(PowerVisitor(current()), currentValue, value);
        }

        variables[name] = currentValue;
    }
};

#endif
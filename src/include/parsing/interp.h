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

        void interpret(std::vector<Token> &tokens) {
            for (auto &token : tokens) {
                if (token.type == TokenType::KEYWORD && token.toString() == "@") {
                    // interpretAssignment(tokens);
                }
                else if (token.type == TokenType::IDENTIFIER && token.toString() == "println") {
                    // interpretPrintln(tokens);
                }
            }
        }

    private:
        std::map<std::string, std::variant<std::string, int, double, bool>> variables;
};

class InterpSession {
    public:
        InterpSession() : interp(), scripts(), args() {}

        void registerScript(const std::string &scriptPath) {
            scripts.push_back(scriptPath);
        }

        void registerArg(const std::string &name, const std::string &value) {
            args[name] = value;
        }

        int start(bool replMode) {
            // TODO: parse scripts vector

            if (replMode) {
                // TODO: return repl()
            }

            return 0;
        }

    private:
        Interpreter interp;
        std::vector<std::string> scripts;
        std::map<std::string, std::string> args;
};

#endif
#ifndef INTERP_SESSION_H
#define INTERP_SESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "interp.h"

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
            // Load any scripts first.
            int ret = loadScripts();

            // Optionally, load REPL.
            if (replMode)
                return loadRepl();

            return ret;
        }

    private:
        Interpreter interp;
        std::vector<std::string> scripts;
        std::map<std::string, std::string> args;

        int loadRepl() {
            std::string input;

            while (true) {
                std::cout << "> ";
                std::getline(std::cin, input);

                if (input == Keywords.Exit)
                    break;

                Lexer lexer(input);
                interp.interpret(lexer.getAllTokens());
            }

            return 0;
        }

        int loadScripts() {
            try {
                for (const std::string &script : scripts) {
                    std::string content = readFile(script);
                    if (content.empty())
                        continue;

                    Lexer lexer(content);
                    interp.interpret(lexer.getAllTokens());
                }
            }
            catch (const std::exception &e) {
                print_error(e);
                return 1;
            }

            return 0;
        }

        std::string readFile(const std::string &filePath) {
            std::ifstream file(filePath);

            if (!file.is_open())
                throw std::ios_base::failure("Cannot open file: " + filePath);

            std::ostringstream string;
            string << file.rdbuf();
            
            std::string content = string.str();
            file.close();

            return content;
        }
};

#endif
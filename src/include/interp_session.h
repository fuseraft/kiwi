#ifndef INTERP_SESSION_H
#define INTERP_SESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "logging/logger.h"
#include "parsing/keywords.h"
#include "interp.h"

class InterpSession {
public:
    InterpSession(Logger &logger, Interpreter &interp)
        : logger(logger), interp(interp), scripts(), args() {}

    void registerScript(const std::string &scriptPath) {
        logger.debug(scriptPath, "InterpSession::registerScript");
        scripts.push_back(scriptPath);
    }

    void registerArg(const std::string &name, const std::string &value) {
        logger.debug(name + "=" + value, "InterpSession::registerArg");
        args[name] = value;
    }

    int start(bool replMode) {
        // Load any registered scripts first.
        int ret = loadScripts();

        if (replMode) {
            return loadRepl();
        }

        return ret;
    }

private:
    Logger &logger;
    Interpreter &interp;
    std::vector<std::string> scripts;
    std::map<std::string, std::string> args;

    int loadRepl() {
        std::string input;

        while (true) {
            try {
                std::cout << "> ";
                std::getline(std::cin, input);

                if (input == Keywords.Exit) {
                    break;
                }

                Lexer lexer(logger, input);
                interp.interpret(lexer);
            } 
            catch (const std::exception &e) {
                print_error(e);
                return 1;
            }
        }

        return 0;
    }

    int loadScripts() {
        int returnCode = 0;

        try {
            for (const std::string &script : scripts) {
                std::string content = readFile(script);
                if (content.empty()) {
                    continue;
                }

                Lexer lexer(logger, content);
                returnCode = interp.interpret(lexer);

                // If one script fails, we need to stop here.
                if (returnCode != 0) {
                    break;
                }
            }
        } 
        catch (const std::exception &e) {
            print_error(e);
            return 1;
        }

        return returnCode;
    }

    std::string readFile(const std::string &filePath) {
        logger.debug(filePath, "InterpSession::readFile");
        std::ifstream file(filePath);

        if (!file.is_open()) {
            logger.error("Cannot open file: " + filePath, "InterpSession::readFile");
            throw std::ios_base::failure("Cannot open file: " + filePath);
        }

        std::ostringstream string;
        string << file.rdbuf();

        return string.str();
    }
};

#endif
#ifndef INTERP_SESSION_H
#define INTERP_SESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "interp.h"
#include "../logging/logger.h"

class InterpSession {
  public:
    InterpSession(Logger &logger, Interpreter &interp)
        : logger(logger), interp(interp), scripts(), args() {}

    void registerScript(const std::string &scriptPath) {
        logger.debug("", "InterpSession::registerScript");
        scripts.push_back(scriptPath);
    }

    void registerArg(const std::string &name, const std::string &value) {
        logger.debug("", "InterpSession::registerArg");
        args[name] = value;
    }

    int start(bool replMode) {
        logger.debug("", "InterpSession::start");
        // Load any scripts first.
        int ret = loadScripts();

        // Optionally, load REPL.
        if (replMode)
            return loadRepl();

        return ret;
    }

  private:
    Logger &logger;
    Interpreter &interp;
    std::vector<std::string> scripts;
    std::map<std::string, std::string> args;

    int loadRepl() {
        logger.debug("", "InterpSession::loadRepl");
        std::string input;

        while (true) {
            try {
                std::cout << "> ";
                std::getline(std::cin, input);

                if (input == Keywords.Exit)
                    break;

                Lexer lexer(logger, input);
                interp.interpret(lexer.getAllTokens());
            } catch (const std::exception &e) {
                print_error(e);
                return 1;
            }
        }

        return 0;
    }

    int loadScripts() {
        logger.debug("", "InterpSession::loadScripts");
        try {
            for (const std::string &script : scripts) {
                std::string content = readFile(script);
                if (content.empty())
                    continue;

                Lexer lexer(logger, content);
                interp.interpret(lexer.getAllTokens());
            }
        } catch (const std::exception &e) {
            print_error(e);
            return 1;
        }

        return 0;
    }

    std::string readFile(const std::string &filePath) {
        logger.debug("", "InterpSession::readFile");
        std::ifstream file(filePath);

        if (!file.is_open())
            throw std::ios_base::failure("Cannot open file: " + filePath);

        std::ostringstream string;
        string << file.rdbuf();

        return string.str();
    }
};

#endif
#ifndef INTERP_SESSION_H
#define INTERP_SESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "errors/error.h"
#include "logging/logger.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "interp.h"

class InterpSession {
public:
    InterpSession(Logger &logger, Interpreter &interp)
        : logger(logger), interp(interp), scripts(), args() {}

    void registerScript(const std::string &scriptPath) {
        if (!FileIO::fileExists(scriptPath)) {
            throw FileNotFoundError(scriptPath);
        }

        std::string absolutePath = FileIO::getAbsolutePath(scriptPath);

        if (scripts.find(absolutePath) != scripts.end()) {
            return;
        }

        scripts.insert(absolutePath);
    }

    void registerArg(const std::string &name, const std::string &value) {
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
    std::unordered_set<std::string> scripts;
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
                ErrorHandler::printError(e);
                return 1;
            }
        }

        return 0;
    }

    int loadScripts() {
        int returnCode = 0;

        try {
            for (const std::string &script : scripts) {
                std::string content = FileIO::readFile(script);
                if (content.empty()) {
                    continue;
                }

                std::string parentPath = FileIO::getParentPath(script);

                Lexer lexer(logger, content);
                returnCode = interp.interpret(lexer, parentPath);

                // If one script fails, we need to stop here.
                if (returnCode != 0) {
                    break;
                }
            }
        } 
        catch (const std::exception &e) {
            ErrorHandler::printError(e);
            return 1;
        }

        return returnCode;
    }
};

#endif
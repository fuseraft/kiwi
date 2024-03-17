// globals.h
#ifndef KIWI_GLOBALS_H
#define KIWI_GLOBALS_H

#include <string>

extern bool DEBUG;
extern bool SILENCE;

extern const std::string kiwi_name = "The Kiwi Programming Language";
extern const std::string kiwi_version = "1.3.1";
extern const std::string kiwi_arg = "kiwi";

#include <unordered_map>
#include <stack>
#include <memory>
#include <string>
#include <mutex>
#include "logging/logger.h"
#include "concurrency/task.h"
#include "objects/method.h"
#include "objects/module.h"
#include "objects/class.h"
#include "stackframe.h"
#include "parsing/tokens.h"
#include "web/httplib.h"

extern Logger logger;
extern TaskManager task;
extern std::unordered_map<std::string, Method> methods;
extern std::unordered_map<std::string, Module> modules;
extern std::unordered_map<std::string, Class> classes;
extern std::unordered_map<std::string, std::string> kiwiArgs;
extern std::stack<std::shared_ptr<CallStackFrame>> callStack;
extern std::stack<std::shared_ptr<TokenStream>> streamStack;
extern std::stack<std::string> moduleStack;
extern httplib::Server kiwiWebServer;
extern std::unordered_map<int, Method> kiwiWebServerHooks;

extern std::string kiwiWebServerHost;
extern k_int kiwiWebServerPort;
extern std::mutex kiwiWebServerMutex;
extern std::mutex kiwiWebServerHooksMutex;
extern std::mutex kiwiWebServerHostMutex;
extern std::mutex kiwiWebServerPortMutex;

extern std::mutex methodsMutex;
extern std::mutex modulesMutex;
extern std::mutex classesMutex;
extern std::mutex kiwiArgsMutex;
extern std::mutex callStackMutex;
extern std::mutex streamStackMutex;
extern std::mutex moduleStackMutex;

#endif
// globals.h
#ifndef KIWI_GLOBALS_H
#define KIWI_GLOBALS_H

#include <string>

extern bool SILENCE;

extern const std::string kiwi_name = "Kiwi";
extern const std::string kiwi_version = "2.0.0";
extern const std::string kiwi_arg = "kiwi";

#ifdef _WIN64
extern const std::string kiwi_min_extension = ".min.kiwi";
extern const std::string kiwi_extension = ".kiwi";
#else
extern const std::string kiwi_min_extension = ".min.🥝";
extern const std::string kiwi_extension = ".🥝";
#endif

#include <unordered_map>
#include <stack>
#include <memory>
#include <string>
#include <mutex>
#include "logging/logger.h"
//#include "concurrency/task.h
#include "stackframe.h"
#include "parsing/tokens.h"
#include "web/httplib.h"

extern Logger logger;
//extern TaskManager task;

extern std::unordered_map<std::string, std::string> kiwiArgs;
extern std::stack<std::shared_ptr<CallStackFrame>> callStack;
extern std::stack<std::string> packageStack;

#endif

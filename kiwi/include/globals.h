// globals.h
#ifndef KIWI_GLOBALS_H
#define KIWI_GLOBALS_H

#include <string>

extern bool SAFEMODE;

extern const std::string kiwi_name = "Kiwi";
extern const std::string kiwi_version = "2.0.9";
extern const std::string kiwi_arg = "kiwi";
extern const std::string kiwi_min_extension = ".min.🥝";
extern const std::string kiwi_extension = ".🥝";

#include <unordered_map>
#include <stack>
#include <memory>
#include <string>
#include <mutex>
#include "logging/logger.h"
#include "stackframe.h"
#include "parsing/tokens.h"
#include "web/httplib.h"

extern Logger logger;
//extern TaskManager task;

extern std::unordered_map<std::string, std::string> kiwiArgs;

#endif

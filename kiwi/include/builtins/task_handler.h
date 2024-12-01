#ifndef KIWI_BUILTINS_TASKHANDLER_H
#define KIWI_BUILTINS_TASKHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "concurrency/task.h"

class TaskBuiltinHandler {
 public:
  static k_value execute(TaskManager& taskmgr, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Task_Busy:
        return executeBusy(taskmgr, token, args);

      case KName::Builtin_Task_List:
        return executeList(taskmgr, token, args);

      case KName::Builtin_Task_Result:
        return executeResult(taskmgr, token, args);

      case KName::Builtin_Task_Sleep:
        return executeSleep(token, args);

      case KName::Builtin_Task_Status:
        return executeStatus(taskmgr, token, args);

      default:
        break;
    }
    throw InvalidOperationError(token, "Come back later.");
  }

 private:
  static k_value executeBusy(TaskManager& taskmgr, const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskBusy);
    }

    return k_value(taskmgr.hasActiveTasks());
  }

  static k_value executeSleep(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskSleep);
    }

    auto ms = get_integer(token, args.at(0));

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));

    return {};
  }

  static k_value executeList(TaskManager& taskmgr, const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskList);
    }

    std::vector<k_value> activeTasks;

    for (const auto& task : taskmgr.getTasks()) {
      activeTasks.push_back(task.first);
    }

    return std::make_shared<List>(activeTasks);
  }

  static k_value executeResult(TaskManager& taskmgr, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskResult);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw TaskError(token, "Invalid task identifier: " +
                                 Serializer::serialize(args.at(0)));
    }

    auto taskId = std::get<k_int>(args.at(0));
    auto taskStatus = taskmgr.isTaskCompleted(token, taskId);

    if (std::holds_alternative<bool>(taskStatus) &&
        std::get<bool>(taskStatus)) {
      return taskmgr.getTaskResult(token, taskId);
    }

    return taskStatus;
  }

  static k_value executeStatus(TaskManager& taskmgr, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskStatus);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw TaskError(token, "Invalid task identifier: " +
                                 Serializer::serialize(args.at(0)));
    }

    k_int taskId = std::get<k_int>(args.at(0));

    return taskmgr.getTaskStatus(token, taskId);
  }
};

#endif
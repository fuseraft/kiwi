#ifndef KIWI_BUILTINS_TASKHANDLER_H
#define KIWI_BUILTINS_TASKHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "concurrency/task.h"

class TaskBuiltinHandler {
 public:
  static KValue execute(TaskManager& taskmgr, const Token& token,
                        const KName& builtin, const std::vector<KValue>& args) {
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
  static KValue executeBusy(TaskManager& taskmgr, const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskBusy);
    }

    return KValue::createBoolean(taskmgr.hasActiveTasks());
  }

  static KValue executeSleep(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskSleep);
    }

    auto ms = get_integer(token, args.at(0));

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));

    return {};
  }

  static KValue executeList(TaskManager& taskmgr, const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskList);
    }

    std::vector<KValue> activeTasks;

    for (const auto& task : taskmgr.getTasks()) {
      activeTasks.push_back(KValue::create(task.first));
    }

    return KValue::createList(std::make_shared<List>(activeTasks));
  }

  static KValue executeResult(TaskManager& taskmgr, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskResult);
    }

    if (!args.at(0).isInteger()) {
      throw TaskError(token, "Invalid task identifier: " +
                                 Serializer::serialize(args.at(0)));
    }

    auto taskId = args.at(0).getInteger();
    auto taskStatus = taskmgr.isTaskCompleted(token, taskId);

    if (taskStatus.isBoolean() && taskStatus.getBoolean()) {
      return taskmgr.getTaskResult(token, taskId);
    }

    return taskStatus;
  }

  static KValue executeStatus(TaskManager& taskmgr, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TaskBuiltins.TaskStatus);
    }

    if (!args.at(0).isInteger()) {
      throw TaskError(token, "Invalid task identifier: " +
                                 Serializer::serialize(args.at(0)));
    }

    k_int taskId = args.at(0).getInteger();

    return taskmgr.getTaskStatus(token, taskId);
  }
};

#endif
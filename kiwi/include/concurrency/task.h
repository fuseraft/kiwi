#ifndef KIWI_CONCURRENCY_TASK_H
#define KIWI_CONCURRENCY_TASK_H

#include <functional>
#include <future>
#include <unordered_map>
#include <atomic>
#include <string>
#include "parsing/tokens.h"
#include "typing/value.h"

class TaskManager {
 public:
  using TaskFunction = std::packaged_task<KValue()>;

 private:
  std::atomic<k_int> nextPromiseId;
  std::unordered_map<k_int, std::future<KValue>> tasks;

 public:
  TaskManager() : nextPromiseId(0) {}

  k_int addTask(TaskFunction func) {
    k_int id = nextPromiseId++;
    auto future = func.get_future();
    tasks[id] = std::move(future);
    std::thread(std::move(func)).detach();  // Run the task in a new thread
    return id;
  }

  std::unordered_map<k_int, std::future<KValue>>& getTasks() { return tasks; }

  bool hasTask(k_int id) { return tasks.find(id) != tasks.end(); }

  KValue getTaskStatus(const Token& token, const k_int& id) {
    const auto& statusKey = KValue::createString("status");
    auto taskStatus = std::make_shared<Hashmap>();
    taskStatus->add(statusKey, KValue::createString("unknown"));

    if (hasTask(id)) {
      const auto& taskCompleted = isTaskCompleted(token, id);
      if (taskCompleted.isBoolean() && taskCompleted.getBoolean()) {
        taskStatus->add(statusKey, KValue::createString("complete"));
      } else {
        taskStatus->add(statusKey, KValue::createString("running"));
      }
    }

    return KValue::createHashmap(taskStatus);
  }

  KValue getTaskResult(const Token& token, k_int id) {
    if (tasks.find(id) == tasks.end()) {
      return getTaskStatus(token, id);
    }

    auto& future = tasks.at(id);
    if (future.valid()) {
      return future.get();
    } else {
      auto status = std::make_shared<Hashmap>();
      status->add(KValue::createString("status"),
                  KValue::createString("running"));
      return KValue::createHashmap(status);
    }
  }

  KValue isTaskCompleted(const Token& token, k_int id) {
    if (!hasTask(id)) {
      return getTaskStatus(token, id);
    }

    return KValue::createBoolean(tasks.at(id).wait_for(std::chrono::seconds(
                                     0)) == std::future_status::ready);
  }

  bool hasActiveTasks() {
    for (auto& activeTask : tasks) {
      if (activeTask.second.valid() &&
          activeTask.second.wait_for(std::chrono::seconds(0)) !=
              std::future_status::ready) {
        return true;
      }
    }

    return false;
  }
};

#endif

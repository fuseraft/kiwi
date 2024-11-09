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
  using TaskFunction = std::packaged_task<k_value()>;

 private:
  std::atomic<k_int> nextPromiseId;
  std::unordered_map<k_int, std::future<k_value>> tasks;

 public:
  TaskManager() : nextPromiseId(0) {}

  k_int addTask(TaskFunction func) {
    k_int id = nextPromiseId++;
    auto future = func.get_future();
    tasks[id] = std::move(future);
    std::thread(std::move(func)).detach();  // Run the task in a new thread
    return id;
  }

  std::unordered_map<k_int, std::future<k_value>>& getTasks() { return tasks; }

  bool hasTask(k_int id) { return tasks.find(id) != tasks.end(); }

  k_value getTaskStatus(const Token& token, const k_int& id) {
    auto taskStatus = std::make_shared<Hash>();
    taskStatus->add("status", "unknown");

    if (hasTask(id)) {
      if (std::get<bool>(isTaskCompleted(token, id))) {
        taskStatus->add("status", "completed");
      } else {
        taskStatus->add("status", "running");
      }
    }

    return taskStatus;
  }

  k_value getTaskResult(const Token& token, k_int id) {
    if (tasks.find(id) == tasks.end()) {
      return getTaskStatus(token, id);
    }

    auto& future = tasks.at(id);
    if (future.valid()) {
      return future.get();
    } else {
      auto status = std::make_shared<Hash>();
      status->add("status", "running");
      return status;
    }
  }

  k_value isTaskCompleted(const Token& token, k_int id) {
    if (!hasTask(id)) {
      return getTaskStatus(token, id);
    }

    return tasks.at(id).wait_for(std::chrono::seconds(0)) ==
           std::future_status::ready;
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

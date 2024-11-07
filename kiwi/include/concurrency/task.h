#ifndef KIWI_CONCURRENCY_TASK_H
#define KIWI_CONCURRENCY_TASK_H

#include <functional>
#include <future>
#include <unordered_map>
#include <atomic>
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

  k_value getTaskResult(k_int id) {
    auto& future = tasks.at(id);
    if (future.valid()) {
      return future.get();
    } else {
      auto status = std::make_shared<Hash>();
      status->add("status", "running");
      return status;
    }
  }

  bool isTaskCompleted(k_int id) {
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

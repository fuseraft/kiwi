#ifndef KIWI_CONCURRENCY_TASK_H
#define KIWI_CONCURRENCY_TASK_H

#include <functional>
#include <future>
#include <unordered_map>
#include <atomic>
#include "typing/valuetype.h"

class TaskManager {
 public:
  using TaskFunction = std::function<Value()>;

 private:
  std::atomic<k_int> nextPromiseId;
  std::unordered_map<k_int, std::future<Value>> tasks;

 public:
  TaskManager() : nextPromiseId(0) {}

  k_int addTask(TaskFunction func) {
    k_int id = nextPromiseId++;
    tasks[id] = std::async(std::launch::async, func);
    return id;
  }

  Value getTaskResult(k_int id) {
    auto& task = tasks.at(id);
    return task.get();
  }

  bool isTaskCompleted(k_int id) {
    auto& task = tasks.at(id);
    return task.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }

  bool hasActiveTasks() {
    for (auto& task : tasks) {
      if (task.second.wait_for(std::chrono::seconds(0)) !=
          std::future_status::ready) {
        return true;
      }
    }

    return false;
  }
};

#endif
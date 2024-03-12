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
  std::atomic<int> nextPromiseId;
  std::unordered_map<int, std::future<Value>> tasks;

 public:
  TaskManager() : nextPromiseId(0) {}

  int addTask(TaskFunction func) {
    int id = nextPromiseId++;
    tasks[id] = std::async(std::launch::async, func);
    return id;
  }

  Value getTaskResult(int id) {
    auto& task = tasks.at(id);
    return task.get();
  }

  bool isTaskCompleted(int id) {
    auto& task = tasks.at(id);
    return task.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }
};

int task_example() {
  TaskManager manager;

  auto taskFunc = []() -> Value {
    return Value(42);
  };

  int promiseId = manager.addTask(taskFunc);

  auto result = manager.getTaskResult(promiseId);

  return 0;
}

#endif

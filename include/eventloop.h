#ifndef KIWI_EVENTLOOP_H
#define KIWI_EVENTLOOP_H

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class EventLoop {
 public:
  using Task = std::function<void()>;

  EventLoop() : stop(false) {}

  ~EventLoop() { stopLoop(); }

  void enqueueTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex);
    tasks.push(task);
    condition.notify_one();
  }

  void startLoop() {
    loopThread = std::thread([this]() {
      while (true) {
        Task task;
        {
          std::unique_lock<std::mutex> lock(mutex);
          condition.wait(lock, [this]() { return stop || !tasks.empty(); });
          if (stop && tasks.empty())
            break;
          task = tasks.front();
          tasks.pop();
        }
        task();
      }
    });
  }

  void stopLoop() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      stop = true;
      condition.notify_all();
    }

    if (loopThread.joinable()) {
      loopThread.join();
    }
  }

 private:
  std::queue<Task> tasks;
  std::mutex mutex;
  std::condition_variable condition;
  std::thread loopThread;
  bool stop;
};

#endif
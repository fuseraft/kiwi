#ifndef KIWI_CONCURRENCY_THREAD_H
#define KIWI_CONCURRENCY_THREAD_H

#include <iostream>
#include <thread>
#include <string>

void threadFunction(std::string message) {
  // Simulate work
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Thread message: " << message << std::endl;
}

int thread_example() {
  std::string msg = "Hello from thread!";
  std::thread t([msg]() { threadFunction(msg); });

  // Detach the thread and let it run independently
  t.detach();

  std::cout << "Main thread continues..." << std::endl;
  // Ensure main thread does enough work or waits before exiting,
  // otherwise, the detached thread might be abruptly terminated.
  std::this_thread::sleep_for(std::chrono::seconds(2));

  return 0;
}

#endif
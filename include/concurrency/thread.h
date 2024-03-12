#ifndef KIWI_CONCURRENCY_THREAD_H
#define KIWI_CONCURRENCY_THREAD_H

#include <thread>
#include <string>

void threadFunction(std::string message) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

int thread_example() {
  std::string msg = "";
  std::thread t([msg]() { threadFunction(msg); });

  t.detach();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  return 0;
}

#endif
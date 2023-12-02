#include <iostream>
#include <thread>
#include <chrono>

// TODO: add to this.

class Thread {
public:
    static void sleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};
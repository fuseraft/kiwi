#include <iostream>
#include <cstdlib>

// Function to create a dynamically allocated integer
extern "C" int* create_integer(int value) {
    int* ptr = new int(value);
    std::cout << "Created integer with value: " << *ptr << std::endl;
    return ptr;
}

// Function to modify the value of the integer
extern "C" void modify_integer(int* ptr, int new_value) {
    if (ptr) {
        *ptr = new_value;
        std::cout << "Modified integer to value: " << *ptr << std::endl;
    } else {
        std::cerr << "Pointer is null. Cannot modify value." << std::endl;
    }
}

// Function to read the value of the integer
extern "C" int read_integer(const int* ptr) {
    if (ptr) {
        return *ptr;
    } else {
        std::cerr << "Pointer is null. Returning default value 0." << std::endl;
        return 0;
    }
}

// Function to free the allocated integer
extern "C" void free_integer(int* ptr) {
    if (ptr) {
        delete ptr;
        std::cout << "Freed allocated integer." << std::endl;
    } else {
        std::cerr << "Pointer is null. Nothing to free." << std::endl;
    }
}

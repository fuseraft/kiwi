#include <iostream>
#include <cstdlib>

// Function that just returns a string.
extern "C" const char* get_version() {
  return "libexample-0.0.1";
}

extern "C" bool is_even(int* ptr) {
  if (!ptr) {
    std::cerr << "Pointer is null. Cannot check evenness." << std::endl;
    return false;
  }

  return *ptr % 2 == 0;
}

extern "C" int boolean_to_int(bool b) {
  return b ? 1 : 0;
}

// Function to create a dynamically allocated integer
extern "C" int* create_integer(int value) {
  int* ptr = new int(value);
  return ptr;
}

// Function to modify the value of the integer
extern "C" void modify_integer(int* ptr, int new_value) {
  if (ptr) {
    *ptr = new_value;
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
  } else {
    std::cerr << "Pointer is null. Nothing to free." << std::endl;
  }
}

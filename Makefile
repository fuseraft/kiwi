CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
BIN_DIR := bin
TEST_DIR := tests

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDE_FILES := $(wildcard $(INCLUDE_DIR)/*.h)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

EXECUTABLE := $(BIN_DIR)/astral

.PHONY: all clean test play install profile

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

format:
	find . -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i --style=file

test: $(EXECUTABLE)
	@echo "================================"
	$(EXECUTABLE) test

play: $(EXECUTABLE)
	@echo "================================"
	$(EXECUTABLE) play

install: $(EXECUTABLE)
	@echo "================================"
	./$(EXECUTABLE) install

profile: CXXFLAGS += -pg
profile: clean $(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

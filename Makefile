CXX := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -pedantic -g

SRC_DIR := kiwi/src
INCLUDE_DIR := kiwi/include
BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDE_FILES := $(wildcard $(INCLUDE_DIR)/*.h)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

EXECUTABLE := $(BIN_DIR)/kiwi

# Define linker flags and libraries
LDFLAGS :=
LDLIBS := -lffi -ldl

.PHONY: all clean test play install profile rpm

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

format:
	find . -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i --style=file

test: $(EXECUTABLE)
	@echo "================================"
	$(EXECUTABLE) ./test

play: $(EXECUTABLE)
	@echo "================================"
	$(EXECUTABLE) play

install: $(EXECUTABLE)
	@echo "================================"
	./$(EXECUTABLE) configure install

profile: CXXFLAGS += -pg
profile: clean $(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

rpm: all
	# Create source tarball
	tar czf kiwi-2.0.10.tar.gz --transform 's,^,kiwi-2.0.10/,' bin/ lib/ kiwi.spec
	# Build the RPM package
	rpmbuild -ta kiwi-2.0.10.tar.gz

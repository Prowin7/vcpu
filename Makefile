CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2

BUILD_DIR := build
HEADERS := $(wildcard src/cpu/*.hpp)

.PHONY: test build clean

test: $(BUILD_DIR)/registers_test $(BUILD_DIR)/cpu_test
	./$(BUILD_DIR)/registers_test
	./$(BUILD_DIR)/cpu_test

build: $(BUILD_DIR)/vcpu

$(BUILD_DIR)/registers_test: tests/registers_test.cpp $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ tests/registers_test.cpp

$(BUILD_DIR)/cpu_test: tests/cpu_test.cpp $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ tests/cpu_test.cpp

$(BUILD_DIR)/vcpu: src/main.cpp $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

clean:
	rm -rf $(BUILD_DIR)

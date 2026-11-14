CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2

BUILD_DIR := build
HEADERS := $(wildcard src/cpu/*.hpp)

.PHONY: test clean

test: $(BUILD_DIR)/registers_test
	./$(BUILD_DIR)/registers_test

$(BUILD_DIR)/registers_test: tests/registers_test.cpp $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ tests/registers_test.cpp

clean:
	rm -rf $(BUILD_DIR)

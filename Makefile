CXX ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD := build

.PHONY: all test queue_tests demo clean

all: $(BUILD)/demo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/demo: src/main.cpp src/device.cpp include/device.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iinclude src/main.cpp src/device.cpp -o $@

$(BUILD)/tests: tests/device_test.cpp tests/check.hpp src/device.cpp include/device.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iinclude tests/device_test.cpp src/device.cpp -o $@

$(BUILD)/queue_tests: tests/queue_test.cpp tests/check.hpp src/queue.cpp src/device.cpp include/queue.hpp include/device.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iinclude tests/queue_test.cpp src/queue.cpp src/device.cpp -o $@

test: $(BUILD)/tests $(BUILD)/queue_tests
	./$(BUILD)/tests
	./$(BUILD)/queue_tests

queue_tests: $(BUILD)/queue_tests
	./$(BUILD)/queue_tests

demo: $(BUILD)/demo
	./$(BUILD)/demo

clean:
	rm -rf $(BUILD)

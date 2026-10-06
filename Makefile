CXX ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD := build

.PHONY: all test demo clean
all: $(BUILD)/demo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/demo: src/main.cpp src/device.cpp include/device.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iinclude src/main.cpp src/device.cpp -o $@

$(BUILD)/tests: tests/device_test.cpp src/device.cpp include/device.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iinclude tests/device_test.cpp src/device.cpp -o $@

test: $(BUILD)/tests
	./$(BUILD)/tests
	$(CXX) $(CXXFLAGS) -Iinclude tests/queue_test.cpp src/queue.cpp src/device.cpp -o $(BUILD)/queue_tests
	./$(BUILD)/queue_tests

demo: $(BUILD)/demo
	./$(BUILD)/demo

clean:
	rm -rf $(BUILD)

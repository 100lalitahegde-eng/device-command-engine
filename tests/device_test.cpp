#include "device.hpp"
#include "check.hpp"
#include <iostream>

int main() {
    using namespace device;
    SimulatedDevice registers(3);
    Controller controller(registers);
    CHECK(controller.execute(0, 3).outcome == Outcome::invalid_command);
    CHECK(controller.execute(7, 2).outcome == Outcome::timeout);
    CHECK(controller.execute(8, 3).outcome == Outcome::device_busy);
    CHECK(controller.execute(9, 3).outcome == Outcome::device_busy);
    controller.reset();
    const auto completed = controller.execute(7, 3);
    CHECK(completed.outcome == Outcome::success && completed.value == 14);
    CHECK(controller.execute(8, 3).outcome == Outcome::device_busy);
    controller.reset();
    registers.fail_next_command();
    CHECK(controller.execute(4, 3).outcome == Outcome::device_fault);
    CHECK(controller.execute(5, 3).outcome == Outcome::device_fault);
    controller.reset();
    CHECK(controller.execute(5, 3).outcome == Outcome::success);
    SimulatedDevice immediate(0);
    Controller other(immediate);
    CHECK(other.execute(1, 0).outcome == Outcome::timeout);
    CHECK(other.execute(1, 1).outcome == Outcome::device_busy);
    other.reset();
    CHECK(other.execute(1, 1).outcome == Outcome::success);
    std::cout << "device tests passed\n";
}

#include "device.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace device;
    SimulatedDevice registers(3);
    Controller controller(registers);
    assert(controller.execute(0, 3).outcome == Outcome::invalid_command);
    assert(controller.execute(7, 2).outcome == Outcome::timeout);
    assert(controller.execute(8, 3).outcome == Outcome::device_busy);
    assert(controller.execute(9, 3).outcome == Outcome::device_busy);
    controller.reset();
    const auto completed = controller.execute(7, 3);
    assert(completed.outcome == Outcome::success && completed.value == 14);
    assert(controller.execute(8, 3).outcome == Outcome::device_busy);
    controller.reset();
    registers.fail_next_command();
    assert(controller.execute(4, 3).outcome == Outcome::device_fault);
    assert(controller.execute(5, 3).outcome == Outcome::device_fault);
    controller.reset();
    assert(controller.execute(5, 3).outcome == Outcome::success);
    SimulatedDevice immediate(0);
    Controller other(immediate);
    assert(other.execute(1, 0).outcome == Outcome::timeout);
    assert(other.execute(1, 1).outcome == Outcome::device_busy);
    other.reset();
    assert(other.execute(1, 1).outcome == Outcome::success);
    std::cout << "device tests passed\n";
}

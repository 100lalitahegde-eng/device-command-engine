#include "device.hpp"
#include <iostream>

int main() {
    device::SimulatedDevice registers(2);
    device::Controller controller(registers);
    const auto result = controller.execute(21, 2);
    if (result.outcome != device::Outcome::success) return 1;
    std::cout << "command 21 -> result " << result.value << '\n';
}

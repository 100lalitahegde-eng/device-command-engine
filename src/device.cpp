#include "device.hpp"
#include <stdexcept>

namespace device {
Result Controller::execute(std::uint32_t command, std::uint32_t poll_budget) {
    if (command == 0) return {Outcome::invalid_command, 0};
    const auto initial = registers_.read(status_reg);
    if (initial & fault) return {Outcome::device_fault, 0};
    if (initial & busy) return {Outcome::device_busy, 0};
    // A completed command must be acknowledged with reset before reuse.
    if (initial & done) return {Outcome::device_busy, 0};
    registers_.write(command_reg, command);
    for (std::uint32_t i = 0; i < poll_budget; ++i) {
        const auto status = registers_.read(status_reg);
        if (status & fault) return {Outcome::device_fault, 0};
        if (status & done) return {Outcome::success, registers_.read(result_reg)};
    }
    return {Outcome::timeout, 0};
}

void Controller::reset() { registers_.write(reset_reg, 1); }

SimulatedDevice::SimulatedDevice(std::uint32_t polls_until_completion)
    : remaining_(polls_until_completion), configured_delay_(polls_until_completion) {}

std::uint32_t SimulatedDevice::read(std::uint32_t offset) {
    switch (offset) {
    case status_reg:
        if (status_ & busy) {
            if (remaining_ > 0) --remaining_;
            if (remaining_ == 0) {
                status_ = pending_fault_ ? fault : done;
                if (!pending_fault_) result_ = command_ * 2u; // defined unsigned wrap
            }
        }
        return status_;
    case result_reg: return result_;
    default: throw std::invalid_argument("register is not readable");
    }
}

void SimulatedDevice::write(std::uint32_t offset, std::uint32_t value) {
    switch (offset) {
    case command_reg:
        if (value == 0 || status_ != 0) throw std::logic_error("device is not idle");
        command_ = value;
        remaining_ = configured_delay_;
        pending_fault_ = fail_next_;
        fail_next_ = false;
        status_ = busy;
        break;
    case reset_reg:
        if (value != 1) throw std::invalid_argument("reset requires 1");
        status_ = result_ = command_ = 0;
        remaining_ = configured_delay_;
        pending_fault_ = false;
        break;
    default: throw std::invalid_argument("register is not writable");
    }
}

void SimulatedDevice::fail_next_command() { fail_next_ = true; }
} // namespace device

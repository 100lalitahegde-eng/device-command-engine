#pragma once

#include <cstdint>

namespace device {
constexpr std::uint32_t command_reg = 0x00;
constexpr std::uint32_t status_reg = 0x04;
constexpr std::uint32_t result_reg = 0x08;
constexpr std::uint32_t reset_reg = 0x0c;
constexpr std::uint32_t busy = 1u << 0;
constexpr std::uint32_t done = 1u << 1;
constexpr std::uint32_t fault = 1u << 2;

struct Registers {
    virtual ~Registers() = default;
    virtual std::uint32_t read(std::uint32_t offset) = 0;
    virtual void write(std::uint32_t offset, std::uint32_t value) = 0;
};

enum class Outcome { success, invalid_command, device_busy, device_fault, timeout };
struct Result { Outcome outcome; std::uint32_t value; };

class Controller {
public:
    explicit Controller(Registers& registers) : registers_(registers) {}
    Result execute(std::uint32_t command, std::uint32_t poll_budget);
    void reset();
private:
    Registers& registers_;
};

// An invented, deterministic register bank; never performs physical I/O.
class SimulatedDevice final : public Registers {
public:
    explicit SimulatedDevice(std::uint32_t polls_until_completion = 1);
    std::uint32_t read(std::uint32_t offset) override;
    void write(std::uint32_t offset, std::uint32_t value) override;
    void fail_next_command();
private:
    std::uint32_t remaining_;
    std::uint32_t configured_delay_;
    std::uint32_t status_ = 0;
    std::uint32_t result_ = 0;
    std::uint32_t command_ = 0;
    bool fail_next_ = false;
    bool pending_fault_ = false;
};
} // namespace device

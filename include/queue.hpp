#pragma once

#include "device.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>

namespace device {
using Ticket = std::uint64_t;
using Owner = std::uint64_t;
enum class JobState { queued, active, success, device_fault, timeout, cancelled };
struct Job { Owner owner; std::uint32_t command; JobState state; std::uint32_t value = 0; };

// Single-threaded host scheduler. Capacity counts waiting jobs, excluding the active job.
class SubmissionQueue {
public:
    SubmissionQueue(Registers& registers, std::size_t capacity, std::uint32_t poll_budget);
    std::optional<Ticket> submit(Owner owner, std::uint32_t command);
    std::optional<Job> inspect(Owner owner, Ticket ticket) const;
    bool cancel(Owner owner, Ticket ticket); // waiting jobs only
    void advance(); // at most one device status poll; starts the next job if idle
    void reset(); // abandons outstanding jobs and resets the device
    std::size_t waiting() const { return waiting_.size(); }
private:
    Registers& registers_;
    std::size_t capacity_;
    std::uint32_t poll_budget_;
    std::uint32_t polls_ = 0;
    Ticket next_ = 1;
    std::deque<Ticket> waiting_;
    std::optional<Ticket> active_;
    std::map<Ticket, Job> jobs_;
};
} // namespace device

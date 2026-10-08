#include "queue.hpp"
#include <algorithm>
#include <stdexcept>

namespace device {
SubmissionQueue::SubmissionQueue(Registers& registers, std::size_t capacity, std::uint32_t poll_budget)
    : registers_(registers), capacity_(capacity), poll_budget_(poll_budget) {
    if (capacity == 0) throw std::invalid_argument("queue capacity must be positive");
}

std::optional<Ticket> SubmissionQueue::submit(Owner owner, std::uint32_t command) {
    if (command == 0 || waiting_.size() == capacity_) return std::nullopt;
    if (next_ == 0) throw std::overflow_error("ticket space exhausted");
    const Ticket ticket = next_++;
    jobs_.emplace(ticket, Job{owner, command, JobState::queued});
    waiting_.push_back(ticket);
    return ticket;
}

std::optional<Job> SubmissionQueue::inspect(Owner owner, Ticket ticket) const {
    const auto it = jobs_.find(ticket);
    if (it == jobs_.end() || it->second.owner != owner) return std::nullopt;
    return it->second;
}

bool SubmissionQueue::cancel(Owner owner, Ticket ticket) {
    const auto it = jobs_.find(ticket);
    if (it == jobs_.end() || it->second.owner != owner || it->second.state != JobState::queued)
        return false;
    const auto pos = std::find(waiting_.begin(), waiting_.end(), ticket);
    waiting_.erase(pos);
    it->second.state = JobState::cancelled;
    return true;
}

void SubmissionQueue::advance() {
    if (!active_) {
        if (waiting_.empty()) return;
        // Never overwrite an unresolved, timed-out, completed or faulted device.
        if (registers_.read(status_reg) != 0) return;
        active_ = waiting_.front();
        waiting_.pop_front();
        auto& job = jobs_.at(*active_);
        registers_.write(command_reg, job.command);
        job.state = JobState::active;
        polls_ = 0;
    }
    auto& job = jobs_.at(*active_);
    if (poll_budget_ == 0) {
        job.state = JobState::timeout;
    } else {
        const auto status = registers_.read(status_reg);
        ++polls_;
        if (status & fault) job.state = JobState::device_fault;
        else if (status & done) {
            job.state = JobState::success;
            job.value = registers_.read(result_reg);
            registers_.write(reset_reg, 1); // acknowledge completion for the next queued job
        } else if (polls_ == poll_budget_) job.state = JobState::timeout;
    }
    if (job.state != JobState::active) active_.reset();
}

void SubmissionQueue::reset() {
    registers_.write(reset_reg, 1);
    if (active_) jobs_.at(*active_).state = JobState::cancelled;
    for (const auto ticket : waiting_) jobs_.at(ticket).state = JobState::cancelled;
    active_.reset();
    waiting_.clear();
    polls_ = 0;
}
} // namespace device

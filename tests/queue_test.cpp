#include "queue.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace device;
    SimulatedDevice registers(2);
    SubmissionQueue queue(registers, 2, 2);
    assert(!queue.submit(1, 0));
    auto a = *queue.submit(1, 3);
    auto b = *queue.submit(2, 4);
    assert(!queue.submit(1, 5));
    assert(!queue.inspect(2, a));
    assert(!queue.cancel(1, b));
    queue.advance();
    assert(queue.inspect(1, a)->state == JobState::active);
    assert(!queue.cancel(1, a));
    assert(queue.cancel(2, b));
    assert(queue.waiting() == 0);
    assert(queue.submit(1, 5));
    queue.advance();
    assert(queue.inspect(1, a)->state == JobState::success);
    assert(queue.inspect(1, a)->value == 6);
    queue.advance();
    queue.advance();
    assert(queue.inspect(2, b)->state == JobState::cancelled);

    SimulatedDevice slow(3);
    SubmissionQueue timed(slow, 1, 2);
    auto t = *timed.submit(1, 7);
    timed.advance();
    auto pending = *timed.submit(1, 8);
    timed.advance();
    assert(timed.inspect(1, t)->state == JobState::timeout);
    timed.advance(); // busy device cannot start pending command
    assert(timed.inspect(1, pending)->state == JobState::queued);
    timed.reset();
    assert(timed.inspect(1, t)->state == JobState::timeout);
    assert(timed.inspect(1, pending)->state == JobState::cancelled);
    auto recovered = *timed.submit(1, 9);
    timed.advance();
    assert(timed.inspect(1, recovered)->state == JobState::active);

    SimulatedDevice faulty(1);
    SubmissionQueue faults(faulty, 2, 1);
    faulty.fail_next_command();
    auto f = *faults.submit(1, 2);
    auto later = *faults.submit(1, 3);
    faults.advance();
    assert(faults.inspect(1, f)->state == JobState::device_fault);
    faults.advance();
    assert(faults.inspect(1, later)->state == JobState::queued);
    faults.reset();
    assert(faults.inspect(1, later)->state == JobState::cancelled);

    SimulatedDevice held(5);
    SubmissionQueue abandoned(held, 1, 5);
    auto in_flight = *abandoned.submit(4, 11);
    abandoned.advance();
    abandoned.reset();
    assert(abandoned.inspect(4, in_flight)->state == JobState::cancelled);
    assert(!abandoned.inspect(5, in_flight));
    std::cout << "queue tests passed\n";
}

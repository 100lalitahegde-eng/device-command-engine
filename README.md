# Device Command Engine

A small C++20 exercise in host-to-device control: submit a command, poll status, handle a device fault or timeout, and reset. A deterministic simulated register bank makes the hardware boundary testable without a board.

**Status:** learning project. The device and register map are invented. This is not a real driver, firmware, DMA implementation, or performance result.

## Register contract

| Offset | Register | Meaning |
| --- | --- | --- |
| `0x00` | COMMAND | Host writes a nonzero command identifier when idle |
| `0x04` | STATUS | Bit 0 busy; bit 1 done; bit 2 fault |
| `0x08` | RESULT | Device-produced result when done |
| `0x0c` | RESET | Host writes 1 to clear state |

The controller checks that the device is idle, submits once, polls at most `poll_budget` times, and returns a typed outcome. A timeout leaves the device state intact; the caller may explicitly reset it. This design uses an abstract register interface, so it makes no claims about actual MMIO ordering, cacheability, interrupts, or platform barriers.

## Bounded submission queue

`SubmissionQueue` is an optional, single-threaded scheduler over the same abstract registers. Construct it with a positive waiting capacity and a per-command poll budget; do not use it and `Controller` on the same register bank concurrently. `submit(owner, command)` rejects zero commands and a full waiting queue, and returns an opaque ticket otherwise. Capacity counts *waiting* jobs; one additional command may be active. `advance()` starts the oldest waiting command when the device is idle and performs at most one status poll. A successful result is read before the scheduler resets the simulated device to admit the next command.

The owner supplied at submission is required to inspect or cancel a ticket. Cancellation only succeeds while a command waits; an active command cannot be recalled from the device. Tickets remain queryable by their owner, including terminal results, for the scheduler's lifetime. The caller should discard the scheduler when that history is no longer needed.

After a fault or exhausted poll budget, the device retains its status and later jobs wait. `reset()` explicitly resets the device and marks any active or waiting jobs cancelled, while preserving earlier terminal outcomes (including timeout and fault). Reset does not undo effects of an already submitted command; `cancelled` on an active job means its result is abandoned. This is deterministic host-side bookkeeping, not a device cancellation guarantee.

## Run

```sh
make test
make demo
```

Requires a C++20 compiler and `make`. `make test` compiles with warnings and runs deterministic tests. No device or external library is required.

## Learning path before interviews

1. Trace each read/write in `Controller::execute`, including idle, done, fault, and timeout paths. Explain why a timeout does not imply cancellation.
2. Change the simulator to complete after a configurable number of polls; add a boundary test for exactly `poll_budget` polls.
3. Add an explicit recovery policy and test whether a fault requires reset before resubmission.
4. Study a real device's published register specification. Only then design a separate platform adapter with the required memory ordering, access widths, mapping lifetime, and error handling. Do not cast a pointer to this simulator's interface and call it a driver.

## Scope and next milestone

The simulator and queue are single threaded and model invented command/control registers only. They provide no real-driver, MMIO, interrupt, or hardware evidence. Real hardware integration would require a specific device specification and platform API.

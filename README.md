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

The simulator is single threaded and models command/control registers only. A useful next milestone is a bounded submission queue with explicit ownership and cancellation semantics. Real hardware integration would require a specific device specification and platform API.

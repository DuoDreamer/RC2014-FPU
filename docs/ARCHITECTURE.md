# RC2014 FPU firmware architecture

This document outlines the baseline firmware scaffolding for the Raspberry Pi Pico 2 floating-point coprocessor. The design emphasizes modularity so that arithmetic units, opcode execution, and I/O glue can evolve independently.

## Hardware interface

- **Data pins:** 8-bit parallel bus for register payloads and command tokens.
- **Control pins:**
  - `WR` / `RD`: Selects bus direction.
  - `EN` / `VALID`: Latches transfers.
  - `BUSY`: Active-low indicator that the coprocessor is executing or servicing a transaction.
- **Timing model:** A polling bus interface currently stands in for the GPIO implementation; the final version should wrap PIO/IRQ-driven handlers that translate pin states into `BusTransaction` objects.

## Memory map

- **256 × 32-bit registers:** Indexed by an 8-bit address.
- **256 × 64-bit registers:** Indexed by an 8-bit address (address space reuse is intentional; the command layer selects width).
- **Opcode RAM:** 1024-entry microcode area for queued instructions or microprograms.

## Firmware subsystems

| Component | Responsibility | Notes |
| --- | --- | --- |
| `RegisterFile` | Owns 32-bit and 64-bit register banks with bounds checking. | Backed by `std::array`; supports zeroization. |
| `OperationDispatcher` | Executes arithmetic/trigonometry operations against the register banks. | Uses IEEE-754 bit reinterprets to preserve payload fidelity. |
| `OpcodeRam` | Stores micro-ops for sequenced execution. | Fixed-size circular buffer can be layered on later. |
| `BusInterface` | Abstracts the parallel I/O signals and payload framing. | Replace `MockBusInterface` with a GPIO/PIO implementation. |
| `SystemContext` | Aggregates subsystems and exposes them to the main loop. | Ensures dispatcher always references the live `RegisterFile`. |

## Control flow

1. The bus layer parses incoming transactions (register writes, opcode payloads, and execution triggers).
2. Register writes load 32-bit or 64-bit payloads based on the command framing.
3. Execution commands resolve to `OperationRequest` instances and are executed by the dispatcher.
4. Results are written back to the register file; `BUSY` is asserted/deasserted around the transaction.

## Extensibility hooks

- **Opcode decoding:** Map opcode words to `OperationRequest` for single-issue execution or queue them for batch runs.
- **PIO offload:** Use RP2040 PIO state machines to service the parallel bus with minimal CPU intervention.
- **Testing harness:** Add host-side simulations that drive `BusInterface` and validate register contents after operations.

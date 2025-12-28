# RC2014-FPU

Floating-point coprocessor firmware for RC2014-compatible systems built on a Raspberry Pi Pico 2. The firmware exposes a parallel 8-bit interface for register access and orchestrates a programmable opcode engine for math workloads.

## Project layout

```
├── CMakeLists.txt        # Pico SDK build entry point
├── cmake/                # SDK import helper
├── include/fpu/          # Public headers for core subsystems
├── src/                  # Firmware implementation
└── docs/                 # Design and bring-up notes
```

### Core subsystems

- **RegisterFile** (`include/fpu/registers.hpp`): 256 × 32-bit and 256 × 64-bit IEEE-754 storage with bounds-checked accessors.
- **OperationDispatcher** (`include/fpu/operations.hpp`): Executes arithmetic/trigonometry requests against the register file.
- **OpcodeRam** (`include/fpu/opcode_ram.hpp`): 1 KiB programmable store for microcode/opcode sequences.
- **BusInterface** (`include/fpu/bus_interface.hpp`): Abstraction for the 8-bit parallel interface; a polling mock is used until GPIO glue is added.
- **SystemContext** (`include/fpu/system_context.hpp`): Wiring of the subsystems for the main control loop.

## Building

1. Install the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` to its location.
2. Configure and build:
   ```bash
   mkdir -p build && cd build
   cmake .. -DPICO_BOARD=pico2
   cmake --build .
   ```
3. The UF2, ELF, BIN, and map artifacts will be generated in the build tree for flashing to the Pico 2.

## Next steps

- Implement a GPIO-backed `BusInterface` to drive the physical enable/write/busy pins and 8-bit data lines.
- Add decoding of opcode sequences and scheduling to the dispatcher.
- Integrate a host-side harness for regression tests of arithmetic instructions and opcode flows.

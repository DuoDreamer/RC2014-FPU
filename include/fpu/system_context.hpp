#pragma once

#include "fpu/bus_interface.hpp"
#include "fpu/opcode_ram.hpp"
#include "fpu/operations.hpp"
#include "fpu/registers.hpp"

namespace fpu {

struct SystemContext {
    RegisterFile registers{};
    OpcodeRam opcode_ram{};
    OperationDispatcher dispatcher{registers};
    BusInterface* bus{nullptr};
};

}  // namespace fpu

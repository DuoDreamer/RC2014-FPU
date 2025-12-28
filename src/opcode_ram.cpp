#include "fpu/opcode_ram.hpp"

namespace fpu {

bool OpcodeRam::write(std::size_t index, Opcode opcode) {
    if (index >= storage_.size()) {
        return false;
    }
    storage_[index] = opcode;
    return true;
}

std::optional<Opcode> OpcodeRam::read(std::size_t index) const {
    if (index >= storage_.size()) {
        return std::nullopt;
    }
    return storage_[index];
}

void OpcodeRam::clear() {
    storage_.fill(Opcode{});
}

}  // namespace fpu

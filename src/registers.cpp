#include "fpu/registers.hpp"

namespace fpu {

std::optional<RegisterFile::Word32> RegisterFile::read32(std::size_t index) const {
    if (index >= kRegisterCount32) {
        return std::nullopt;
    }
    return reg32_[index];
}

std::optional<RegisterFile::Word64> RegisterFile::read64(std::size_t index) const {
    if (index >= kRegisterCount64) {
        return std::nullopt;
    }
    return reg64_[index];
}

bool RegisterFile::write32(std::size_t index, RegisterFile::Word32 value) {
    if (index >= kRegisterCount32) {
        return false;
    }
    reg32_[index] = value;
    return true;
}

bool RegisterFile::write64(std::size_t index, RegisterFile::Word64 value) {
    if (index >= kRegisterCount64) {
        return false;
    }
    reg64_[index] = value;
    return true;
}

void RegisterFile::clear() {
    reg32_.fill(0);
    reg64_.fill(0);
}

}  // namespace fpu

#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace fpu {

constexpr std::size_t kRegisterCount32 = 256;
constexpr std::size_t kRegisterCount64 = 256;

class RegisterFile {
public:
    using Word32 = uint32_t;
    using Word64 = uint64_t;

    std::optional<Word32> read32(std::size_t index) const;
    std::optional<Word64> read64(std::size_t index) const;

    bool write32(std::size_t index, Word32 value);
    bool write64(std::size_t index, Word64 value);

    void clear();

private:
    std::array<Word32, kRegisterCount32> reg32_{};
    std::array<Word64, kRegisterCount64> reg64_{};
};

}  // namespace fpu

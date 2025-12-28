#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace fpu {

constexpr std::size_t kOpcodeRamSize = 1024;

enum class OpcodeKind : uint8_t {
    Nop = 0x00,
    Load32 = 0x10,
    Load64 = 0x11,
    Store32 = 0x12,
    Store64 = 0x13,
    ExecuteOp = 0x20,
};

struct Opcode {
    OpcodeKind kind{OpcodeKind::Nop};
    uint8_t operand0{0};
    uint8_t operand1{0};
    uint8_t operand2{0};
};

class OpcodeRam {
public:
    bool write(std::size_t index, Opcode opcode);
    std::optional<Opcode> read(std::size_t index) const;
    void clear();
    std::size_t capacity() const { return storage_.size(); }

private:
    std::array<Opcode, kOpcodeRamSize> storage_{};
};

}  // namespace fpu

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "fpu/registers.hpp"

namespace fpu {

enum class OperationId : uint8_t {
    Add32,
    Sub32,
    Mul32,
    Div32,
    Add64,
    Sub64,
    Mul64,
    Div64,
    Sine64,
    Cosine64,
    Tangent64,
    SquareRoot64,
};

struct OperationRequest {
    OperationId id;
    uint8_t destination;
    uint8_t source0;
    uint8_t source1;
};

struct OperationResult {
    bool ok{false};
    std::optional<std::string_view> error;
};

class OperationDispatcher {
public:
    explicit OperationDispatcher(RegisterFile& registers) : registers_(registers) {}
    OperationResult execute(const OperationRequest& request);

private:
    RegisterFile& registers_;
};

}  // namespace fpu

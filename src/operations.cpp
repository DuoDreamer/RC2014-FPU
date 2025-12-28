#include "fpu/operations.hpp"

#include <bit>
#include <cmath>

namespace fpu {

namespace {

std::optional<float> load_f32(const RegisterFile& registers, uint8_t index) {
    auto raw = registers.read32(index);
    if (!raw.has_value()) {
        return std::nullopt;
    }
    return std::bit_cast<float>(*raw);
}

std::optional<double> load_f64(const RegisterFile& registers, uint8_t index) {
    auto raw = registers.read64(index);
    if (!raw.has_value()) {
        return std::nullopt;
    }
    return std::bit_cast<double>(*raw);
}

OperationResult write_f32(RegisterFile& registers, uint8_t index, float value) {
    auto raw = std::bit_cast<RegisterFile::Word32>(value);
    if (!registers.write32(index, raw)) {
        return {false, "destination register out of range"};
    }
    return {true, std::nullopt};
}

OperationResult write_f64(RegisterFile& registers, uint8_t index, double value) {
    auto raw = std::bit_cast<RegisterFile::Word64>(value);
    if (!registers.write64(index, raw)) {
        return {false, "destination register out of range"};
    }
    return {true, std::nullopt};
}

}  // namespace

OperationResult OperationDispatcher::execute(const OperationRequest& request) {
    switch (request.id) {
        case OperationId::Add32: {
            auto a = load_f32(registers_, request.source0);
            auto b = load_f32(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f32(registers_, request.destination, *a + *b);
        }
        case OperationId::Sub32: {
            auto a = load_f32(registers_, request.source0);
            auto b = load_f32(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f32(registers_, request.destination, *a - *b);
        }
        case OperationId::Mul32: {
            auto a = load_f32(registers_, request.source0);
            auto b = load_f32(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f32(registers_, request.destination, *a * *b);
        }
        case OperationId::Div32: {
            auto a = load_f32(registers_, request.source0);
            auto b = load_f32(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f32(registers_, request.destination, *a / *b);
        }
        case OperationId::Add64: {
            auto a = load_f64(registers_, request.source0);
            auto b = load_f64(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, *a + *b);
        }
        case OperationId::Sub64: {
            auto a = load_f64(registers_, request.source0);
            auto b = load_f64(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, *a - *b);
        }
        case OperationId::Mul64: {
            auto a = load_f64(registers_, request.source0);
            auto b = load_f64(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, *a * *b);
        }
        case OperationId::Div64: {
            auto a = load_f64(registers_, request.source0);
            auto b = load_f64(registers_, request.source1);
            if (!a || !b) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, *a / *b);
        }
        case OperationId::Sine64: {
            auto a = load_f64(registers_, request.source0);
            if (!a) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, std::sin(*a));
        }
        case OperationId::Cosine64: {
            auto a = load_f64(registers_, request.source0);
            if (!a) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, std::cos(*a));
        }
        case OperationId::Tangent64: {
            auto a = load_f64(registers_, request.source0);
            if (!a) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, std::tan(*a));
        }
        case OperationId::SquareRoot64: {
            auto a = load_f64(registers_, request.source0);
            if (!a) {
                return {false, "source register out of range"};
            }
            return write_f64(registers_, request.destination, std::sqrt(*a));
        }
        default:
            return {false, "unsupported operation"};
    }
}

}  // namespace fpu

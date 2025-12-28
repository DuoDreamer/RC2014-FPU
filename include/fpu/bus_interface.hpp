#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "fpu/registers.hpp"

namespace fpu {

enum class BusDirection { Read, Write };
enum class BusWidth { Byte = 1 };

struct BusSignals {
    bool enable{false};
    bool write{false};
    bool busy{false};  // Active low on physical pin; logical true means busy.
};

struct BusTransaction {
    BusDirection direction{BusDirection::Read};
    uint8_t address{0};
    std::vector<uint8_t> payload;
};

class BusInterface {
public:
    virtual ~BusInterface() = default;
    virtual std::optional<BusTransaction> poll() = 0;
    virtual void respond(const BusTransaction& transaction, const std::vector<uint8_t>& data) = 0;
    virtual void set_busy(bool busy) = 0;
};

class MockBusInterface : public BusInterface {
public:
    std::optional<BusTransaction> poll() override { return std::nullopt; }
    void respond(const BusTransaction&, const std::vector<uint8_t>&) override {}
    void set_busy(bool) override {}
};

}  // namespace fpu

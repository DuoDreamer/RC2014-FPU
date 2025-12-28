#include <array>
#include <vector>

#include "pico/stdlib.h"

#include "fpu/bus_interface.hpp"
#include "fpu/opcode_ram.hpp"
#include "fpu/operations.hpp"
#include "fpu/registers.hpp"
#include "fpu/system_context.hpp"

namespace {

using namespace fpu;

class PollingBus : public MockBusInterface {
public:
    void set_busy(bool busy) override { signals_.busy = busy; }

private:
    BusSignals signals_{};
};

bool handle_transaction(SystemContext& context, const BusTransaction& tx) {
    if (tx.direction == BusDirection::Write) {
        if (tx.payload.size() == sizeof(RegisterFile::Word32)) {
            RegisterFile::Word32 word{};
            for (std::size_t i = 0; i < sizeof(word); ++i) {
                word |= static_cast<RegisterFile::Word32>(tx.payload[i]) << (8 * i);
            }
            return context.registers.write32(tx.address, word);
        }
        if (tx.payload.size() == sizeof(RegisterFile::Word64)) {
            RegisterFile::Word64 word{};
            for (std::size_t i = 0; i < sizeof(word); ++i) {
                word |= static_cast<RegisterFile::Word64>(tx.payload[i]) << (8 * i);
            }
            return context.registers.write64(tx.address, word);
        }
    } else {
        if (tx.payload.empty()) {
            auto value32 = context.registers.read32(tx.address);
            if (value32.has_value()) {
                std::vector<uint8_t> data(sizeof(RegisterFile::Word32));
                auto word = *value32;
                for (std::size_t i = 0; i < data.size(); ++i) {
                    data[i] = static_cast<uint8_t>((word >> (8 * i)) & 0xFF);
                }
                context.bus->respond(tx, data);
                return true;
            }
        }
    }
    return false;
}

void run_bus_loop(SystemContext& context) {
    while (true) {
        auto transaction = context.bus->poll();
        if (transaction.has_value()) {
            context.bus->set_busy(true);
            handle_transaction(context, *transaction);
            context.bus->set_busy(false);
        }
        sleep_ms(1);
    }
}

}  // namespace

int main() {
    stdio_init_all();

    PollingBus bus{};
    SystemContext context{};
    context.bus = &bus;
    context.registers.clear();
    context.opcode_ram.clear();

    run_bus_loop(context);
    return 0;
}

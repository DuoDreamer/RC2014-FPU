#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace fpu {

class DebugInterface {
public:
    void init();
    void poll();

    void log(std::string_view message);
    bool enabled() const { return debug_enabled_; }

private:
    void send_line(std::string_view line);
    void handle_command(std::string_view command);
    void set_debug_enabled(bool enabled);

    std::array<char, 128> rx_buffer_{};
    std::size_t rx_size_{0};
    bool debug_enabled_{true};
};

}  // namespace fpu

#include "fpu/debug_interface.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

namespace fpu {
namespace {

constexpr uint32_t kBaudRate = 115200;
constexpr uart_inst_t* kDebugUart = uart0;
constexpr uint kTxPin = PICO_DEFAULT_UART_TX_PIN;
constexpr uint kRxPin = PICO_DEFAULT_UART_RX_PIN;

std::string normalize_command(std::string_view command) {
    auto start = command.find_first_not_of(" \t");
    auto end = command.find_last_not_of(" \t");
    if (start == std::string_view::npos || end == std::string_view::npos) {
        return {};
    }

    std::string normalized(command.substr(start, end - start + 1));
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return normalized;
}

}  // namespace

void DebugInterface::init() {
    uart_init(kDebugUart, kBaudRate);
    gpio_set_function(kTxPin, GPIO_FUNC_UART);
    gpio_set_function(kRxPin, GPIO_FUNC_UART);
    uart_set_hw_flow(kDebugUart, false, false);
    uart_set_format(kDebugUart, 8, 1, UART_PARITY_NONE);

    // Publish binary info for detection utilities.
    bi_decl_if_func_used(bi_program_feature("UART debug console"));

    send_line("Debug UART ready. Commands: DEBUG ON|OFF, DEBUG?, HELP.");
}

void DebugInterface::poll() {
    while (uart_is_readable(kDebugUart)) {
        char c = static_cast<char>(uart_getc(kDebugUart));
        if (c == '\r' || c == '\n') {
            if (rx_size_ > 0) {
                handle_command(std::string_view(rx_buffer_.data(), rx_size_));
                rx_size_ = 0;
            }
        } else if (rx_size_ + 1 < rx_buffer_.size()) {
            rx_buffer_[rx_size_++] = c;
        } else {
            rx_size_ = 0;
            send_line("Command too long; cleared buffer.");
        }
    }
}

void DebugInterface::log(std::string_view message) {
    if (!debug_enabled_) {
        return;
    }
    send_line(message);
}

void DebugInterface::send_line(std::string_view line) {
    for (char c : line) {
        uart_putc_raw(kDebugUart, c);
    }
    uart_putc_raw(kDebugUart, '\r');
    uart_putc_raw(kDebugUart, '\n');
}

void DebugInterface::handle_command(std::string_view command) {
    auto normalized = normalize_command(command);
    if (normalized.empty()) {
        return;
    }

    if (normalized == "DEBUG ON" || normalized == "DEBUG 1") {
        set_debug_enabled(true);
        send_line("Debug messages enabled.");
    } else if (normalized == "DEBUG OFF" || normalized == "DEBUG 0") {
        set_debug_enabled(false);
        send_line("Debug messages disabled.");
    } else if (normalized == "DEBUG?" || normalized == "STATUS") {
        send_line(debug_enabled_ ? "Debug messages are ON." : "Debug messages are OFF.");
    } else if (normalized == "HELP") {
        send_line("Commands: DEBUG ON|OFF|0|1, DEBUG?, STATUS, HELP");
    } else {
        send_line("Unrecognized command: " + normalized);
    }
}

void DebugInterface::set_debug_enabled(bool enabled) { debug_enabled_ = enabled; }

}  // namespace fpu

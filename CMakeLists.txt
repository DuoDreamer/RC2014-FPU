cmake_minimum_required(VERSION 3.21)

include(cmake/pico_sdk_import.cmake)

project(RC2014_FPU_Firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

add_executable(rc2014_fpu
    src/main.cpp
    src/registers.cpp
    src/operations.cpp
    src/opcode_ram.cpp
    src/debug_interface.cpp
)

target_include_directories(rc2014_fpu
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_definitions(rc2014_fpu PRIVATE
    PICO_HEAP_SIZE=65536
    PICO_CORE1_STACK_SIZE=16384
)

target_link_libraries(rc2014_fpu
    pico_stdlib
)

pico_enable_stdio_usb(rc2014_fpu 1)
pico_enable_stdio_uart(rc2014_fpu 0)

pico_add_extra_outputs(rc2014_fpu)

cmake_minimum_required(VERSION 3.13)

# This file mirrors the official Raspberry Pi Pico SDK import pattern:
# https://github.com/raspberrypi/pico-sdk
if (DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif ()

if (NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Please set it to the location of the pico-sdk checkout.")
endif ()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)

if (NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(FATAL_ERROR "Unable to find ${PICO_SDK_INIT_CMAKE_FILE}")
endif ()

include(${PICO_SDK_INIT_CMAKE_FILE})

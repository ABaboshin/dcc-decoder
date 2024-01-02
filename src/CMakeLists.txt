cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(dcc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_DECODER_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

add_executable(
        dcc
        main.cpp
        DccDecoder.cpp
        DccDecoderState.cpp
        DccDevice.cpp
        RP2040DccDecoder.cpp
        )

target_compile_definitions(
        dcc PUBLIC
        # https://forums.raspberrypi.com/viewtopic.php?t=317631
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)

# Add pico_multicore which is required for multicore functionality
target_link_libraries(
        dcc
        pico_stdlib
        hardware_pwm
        hardware_adc
        hardware_flash
        pico_multicore
)

pico_enable_stdio_usb(dcc 1)
pico_enable_stdio_uart(dcc 0)

pico_add_extra_outputs(dcc)

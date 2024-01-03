cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(dcc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(common)
add_subdirectory(/picomp3lib/src /picomp3lib/src/build)
include_directories(common fs /picomp3lib/src /picomp3lib /picomp3lib/interface)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

# add_custom_command(
#         OUTPUT res.o
#         COMMAND ${CMAKE_LINKER} -r -b binary -o res.o output.mp3
#         DEPENDS output.mp3)

function(embed_resource resource_file_name source_file_name variable_name)

        file(READ ${resource_file_name} hex_content HEX)

        string(REPEAT "[0-9a-f]" 32 column_pattern)
        string(REGEX REPLACE "(${column_pattern})" "\\1\n" content "${hex_content}")

        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " content "${content}")

        string(REGEX REPLACE ", $" "" content "${content}")

        set(array_definition "static const unsigned char ${variable_name}[] =\n{\n${content}\n};")

        set(source "// Auto generated file.\n${array_definition}\n")

        file(WRITE "${source_file_name}" "${source}")

endfunction()

embed_resource(output.mp3 resource.cpp outputmp3)

add_executable(
        dcc
        main.cpp
        RP2040DccDecoder.cpp
        Train.cpp
        /picomp3lib/interface/music_file.c
        MP3Player.cpp
        fs/ff.c
        
        )

# add_link_options(
#         --format=binary Train.cpp --format=default
# )

target_compile_definitions(
        dcc PUBLIC
        # https://forums.raspberrypi.com/viewtopic.php?t=317631
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)

target_link_libraries(
        dcc
        pico_stdlib
        hardware_pwm
        hardware_dma
        dcc-lib
        picomp3lib
)

pico_enable_stdio_usb(dcc 1)
pico_enable_stdio_uart(dcc 0)

pico_add_extra_outputs(dcc)

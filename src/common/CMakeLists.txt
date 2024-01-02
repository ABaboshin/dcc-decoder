cmake_minimum_required(VERSION 3.13)

project(dcc-lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_DECODER_PATH ${PROJECT_SOURCE_DIR})

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

add_library(
        dcc-lib STATIC
        DccDecoder.cpp
        DccDecoderState.cpp
        DccDevice.cpp
        )

# Add pico_multicore which is required for multicore functionality
target_link_libraries(
        dcc-lib
)

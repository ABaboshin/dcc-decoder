cmake_minimum_required(VERSION 3.13)

project(fs C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

add_library(
        fs STATIC
        ff.c
        )

# Add pico_multicore which is required for multicore functionality
target_link_libraries(
        fs
)

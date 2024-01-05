// Maintain configuration data
#pragma once

#include <stdbool.h>

typedef enum sound_state    // Describes the supported music and noise states
{
    off = 0,
    start = off + 1,
    brown = start,
    file_1 = brown + 1,
    file_2 = file_1 + 1,
    file_3 = file_2 + 1,
    white = file_3 + 1,
    pink = white + 1,
    end = pink + 1
} sound_state;

extern bool getSampleValues(uint sample_rate, uint* shift, uint* wrap, uint* mid_point, float* fraction);
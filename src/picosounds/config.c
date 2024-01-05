#include <stdio.h>
#include "config.h"
#include "ff.h"

// Determine configuration data, based on sample rate
bool getSampleValues(uint sample_rate, uint* shift, uint* wrap, uint* mid_point, float* fraction)
{
    bool ret = true;

    switch (sample_rate)
    {
        case 11000:
            *shift = 2;
            *wrap = 4091;
            *fraction = 1.0f;
        break;

        case 22000:
            *shift = 1;
            *wrap = 4091;
            *fraction = 1.0f;
        break;

        case 11025:
            *shift = 2;
            *wrap = 4082;
            *fraction = 1.0f;
        break;

        case 22050:
            *shift = 1;
            *wrap = 4082;
            *fraction = 1.0f;
        break;

        case 44000:
            *shift = 0;
            *wrap = 4091;
            *fraction = 1.0f;
        break;

        case 44100:
            *shift = 0;
            *wrap = 4082;
            *fraction = 1.0f;
        break;

        case 8000:
            *shift = 2;
            *wrap = 4091;
            *fraction = 1.375f;
        break;

        case 16000:
            *shift = 1;
            *wrap = 4091;
            *fraction = 1.375f;
        break;

        case 32000:
            *shift = 0;
            *wrap = 4091;
            *fraction = 1.375f;
        break;

        case 12000:
            *shift = 2;
            *wrap = 3750;
            *fraction = 1.0f;
        break;

        case 24000:
            *shift = 1;
            *wrap = 3750;
            *fraction = 1.0f;
        break;

        case 48000:
            *shift = 0;
            *wrap = 3750;
            *fraction = 1.0f;
        break;

        default:
            // Not a supported rate
            *wrap = 0;
            ret = false;
        break;
    }

    // mid point is half of wrap value
    *mid_point = *wrap >> 1;

    return ret;
}


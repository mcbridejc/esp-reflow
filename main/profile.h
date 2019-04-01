#pragma once

#include <stdint.h>

typedef struct __profile_point_t {
    uint16_t temp;
    uint16_t duration;
    uint8_t ramp;
} profile_point_t;


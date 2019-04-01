#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef struct {
    int gpio;
    int event_bit;
} button_t;

#define BUTTON_A (39)
#define BUTTON_B (38)
#define BUTTON_C (37)

int button_init(button_t *button, int gpio);
EventGroupHandle_t button_event_group();
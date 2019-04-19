#include "PowerControl.h"

#include "driver/gpio.h"


PowerControl::PowerControl(int gpio) : mGpio(gpio) 
{
    mTimerHandle = NULL;
    gpio_set_level((gpio_num_t)mGpio, 0);
    gpio_set_direction((gpio_num_t)mGpio, GPIO_MODE_OUTPUT);
}

void PowerControl::StaticCallback(void *ctx) {
    PowerControl *self = (PowerControl*)ctx;
    self->Callback();
}

void PowerControl::Callback() {
    if(mState) {
        mState = 0;
        gpio_set_level((gpio_num_t)mGpio, 0);
        esp_timer_start_once(mTimerHandle, 1000000 * (100 - mOutput) / 100);
    } else {
        mState = 1;
        if(mOutput > 0) {
            gpio_set_level((gpio_num_t)mGpio, 1);
        }
        esp_timer_start_once(mTimerHandle, 1000000 * mOutput / 100);
    }
}

void PowerControl::setOutput(uint8_t output) {
    initIfNeeded();
    mOutput = output;
}

void PowerControl::initIfNeeded() {
    esp_timer_create_args_t args;

    if(mTimerHandle == NULL) {
        args.callback = PowerControl::StaticCallback;
        args.arg = this;
        args.dispatch_method = ESP_TIMER_TASK;
        args.name = "PowerControl";
        esp_timer_create(&args, &mTimerHandle);
        esp_timer_start_once(mTimerHandle, 1000000);
    }
}
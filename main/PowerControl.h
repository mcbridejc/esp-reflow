#pragma once

#include "IOutput.h"

#include "esp_timer.h"

class PowerControl : public IOutput {
public:
    PowerControl(int pwm_gpio);

    virtual void setOutput(uint8_t output);
private:
    esp_timer_handle_t mTimerHandle;
    int mGpio;
    uint8_t mOutput;
    uint8_t mState;

    void initIfNeeded();

    void Callback();
    static void StaticCallback(void *arg);
};
#pragma once

#include <stdint.h>

class IOutput {
public:
    virtual void setOutput(uint8_t power);
};

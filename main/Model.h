#pragma once

#include "IOutput.h"
#include "ISensor.h"

// Simulated model of the oven for testing
class Model : public IOutput, public ISensor {
public:
    Model();

    virtual void setOutput(uint8_t power);
    uint8_t getOutput();
    virtual float read();

private:
    float mTemp;
    uint8_t mOutput;
    uint64_t mLastTimestamp;
};
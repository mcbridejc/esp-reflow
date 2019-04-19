#pragma once

#include <stdint.h>
#include <stdexcept>

class ReflowLog {
public:
    struct Entry {
        uint32_t timestamp_ms;
        float integratorSum;
        uint16_t measuredTemp;
        uint16_t targetTemp;
        uint8_t output;
    } __attribute__((__packed__));

    ReflowLog(uint16_t bufferSize);

    void reset();
    void log(
        float integratorSum,
        uint16_t measuredTemp,
        uint16_t targetTemp,
        uint8_t output);

    Entry &operator[](int index);
    
    uint32_t size() { return mSize; }

private:
    Entry *mData;
    uint32_t mAllocSize;
    uint32_t mSize;
    uint32_t mRotate;
};
#include "ReflowLog.h"

#include "esp_timer.h"

#include <stdio.h>
#include <stdlib.h>

ReflowLog::ReflowLog(uint16_t bufferSize) {
    mData = (Entry *)malloc(sizeof(Entry) * bufferSize);
    if(mData == NULL) {
        printf("Could not allocate memory for log storage");
        mAllocSize = 0;
        return;
    }

    mAllocSize = bufferSize;
    reset();
}

void ReflowLog::reset() {
    mSize = 0;
    mRotate = 0;
}

ReflowLog::Entry& ReflowLog::operator[](int index) {
    return mData[(mRotate + index) % mAllocSize];
}

void ReflowLog::log(
        float integratorSum,
        uint16_t measuredTemp,
        uint16_t targetTemp,
        uint8_t output) 
{
    uint32_t timestamp = esp_timer_get_time() / 1000;
    Entry e = {timestamp, integratorSum, measuredTemp, targetTemp, output};
    if(mSize < mAllocSize) {
        mData[mSize++] = e; 
    } else {
        mData[mRotate] = e;
        mRotate = (mRotate + 1) % mAllocSize;
    }
}


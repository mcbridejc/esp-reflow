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
    mSize = 0;
}

void ReflowLog::reset() {
    mSize = 0;
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
        printf("Log overflow");
    }
}


#include "Model.h"

#include "esp_timer.h"

#define MODEL_KHEAT (1.0/30.)
#define MODEL_KRADIATE (1.0/160.0)

Model::Model() : mTemp(20.0) {

}

float Model::read() {
    return mTemp;
}

void Model::setOutput(uint8_t output) {
    mOutput = output;
    uint64_t timestamp = esp_timer_get_time();
    float dT = (timestamp - mLastTimestamp) / 1e6;
    mLastTimestamp = timestamp;
    
    mTemp += (MODEL_KHEAT * output - MODEL_KRADIATE * mTemp) * dT;
}

uint8_t Model::getOutput() {
    return mOutput;
}
#pragma once

#include "OLEDDisplay.h"
#include "profile.h"

#ifdef __cplusplus
extern "C" {
#endif 

class Display {
public:
    Display(OLEDDisplay *display) : 
        mDisplay(display),
        mProfile(),
        mStatus(NULL)
    { }

    void setTempReading(uint16_t temp_c);
    
    void setTempTarget(uint16_t temp_c);

    void setOutput(uint8_t output);

    void setProfile(Profile &profile, uint16_t stage, uint16_t sec_into_stage);

    void setStatus(const char *status);

    void update();
private:
    OLEDDisplay *mDisplay;
    Profile mProfile;
    const char *mStatus;
    uint16_t mProfileStage;
    uint16_t mProfileTime;
    uint16_t mTempReading;
    uint16_t mTempTarget;
    uint8_t mOutput;

    void renderProfile(uint16_t left, uint16_t top, uint16_t width, uint16_t height);
};

#ifdef __cplusplus
}
#endif
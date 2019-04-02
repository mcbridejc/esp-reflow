#pragma once

#include "ISensor.h"
#include "IOutput.h"
#include "profile.h"

class Control {
public:
    Control(ISensor *sensor, IOutput *output);

    void setProfile(const profile_point_t *profile);

    /** Gets the current target temperature computed from the profile */
    uint16_t targetTemp() { return mTargetTemp; }

    /** Get the currently active step in the profile sequence */
    uint16_t profileStage() { return mProfileStage; }

    /** Get the current elapsed time for the active stage
     * 
     * Note that for stages where ramp=0, this time does not begin incrementing
     * until the target temperature is reached. Prior to reaching the target, 
     * this will be 0. */
    uint16_t profileElapsedTime() { return mProfileElapsed; }

    /** Must be called periodically to update the controller
     * 
     * It does not have to be a particular period, but around 1Hz is reasonable
     */
    void run();

    float integrationValue() { return mIntegration; }

private:
    ISensor *mSensor;
    IOutput *mOutput;
    const profile_point_t *mProfile;
    uint8_t mProfileStage;
    float mProfileElapsed;
    float mTargetTemp;
    uint64_t mLastRunTime;
    float mKp;
    float mKi;
    float mKf;
    float mIntegration;

    void advanceProfile(float curTemp, float dT);
};

#pragma once

#include "ISensor.h"
#include "IOutput.h"
#include "profile.h"

class Control {
public:

    enum State_e {
        Idle,
        RunningProfile,
        RunningTempHold
    };

    Control(ISensor *sensor, IOutput *output);

    void setProfile(const Profile &profile);

    /** Gets the current target temperature computed from the profile */
    uint16_t targetTemp() { return mTargetTemp; }

    uint16_t latestTemp() { return mLatestTemp; }

    /** Get the currently active step in the profile sequence */
    uint16_t profileStage() { return mProfileStage; }

    /** Get the current elapsed time for the active stage
     * 
     * Note that for stages where ramp=0, this time does not begin incrementing
     * until the target temperature is reached. Prior to reaching the target, 
     * this will be 0. */
    uint16_t profileElapsedTime() { return mProfileElapsed; }

    uint8_t output() { return mLastOutput; }

    State_e currentState() { return mState; }

    /** Begin running the profile */
    void startProfile();

    /** Enter Temp Hold mode and maintain the given temperature */
    void holdTemp(float temp);

    /** Go to idle, and turn off all output power */
    void stop();

    /** Must be called periodically to update the controller
     * 
     * It does not have to be a particular period, but around 1Hz is reasonable
     */
    void run();

    float integrationValue() { return mIntegration; }

private:
    ISensor *mSensor;
    IOutput *mOutput;
    Profile mProfile;
    uint8_t mProfileStage;
    uint8_t mLastOutput;
    float mProfileElapsed;
    float mTargetTemp;
    float mLatestTemp;
    uint64_t mLastRunTime;
    float mKp;
    float mKi;
    float mKf;
    float mIntegration;
    State_e mState;

    void advanceProfile(float curTemp, float dT);
};

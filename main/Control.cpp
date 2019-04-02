#include "Control.h"

#include "esp_timer.h"

#include <math.h>

// Within how many degrees must we measure to consider the target temp reached
#define TEMP_MARGIN 3

// Default controller gains
#define DEFAULT_KP (3.5)
#define DEFAULT_KI (0.05)
#define DEFAULT_KF (0.15)

Control::Control(ISensor *sensor, IOutput *output) :
    mSensor(sensor),
    mOutput(output),
    mProfile(NULL),
    mProfileStage(0),
    mProfileElapsed(0.0),
    mLastRunTime(0),
    mKp(DEFAULT_KP),
    mKi(DEFAULT_KI),
    mKf(DEFAULT_KF)
{

}

void Control::setProfile(const profile_point_t *profile) {
    mProfile = profile;
}

void Control::run() {
    uint64_t curTime_us = esp_timer_get_time();
    float delta = (curTime_us - mLastRunTime) / 1e6;
    mLastRunTime = curTime_us;
    float curTemp = mSensor->read();

    advanceProfile(curTemp, delta);


    // Run a PI controller with a feedforward term
    // effort = kp * e + ki * sum(e*dT) + kf * targetTemp
    float e = mTargetTemp - curTemp;

    float effort = (mKp * e + mKi * mIntegration + mKf * mTargetTemp);

    // Don't integrate error if our output has hit a rail
    if(effort > 0 && effort < 100) {
        mIntegration += e * delta;
    }

    if(effort < 0) {
        effort = 0;
    }
    if(effort > 100) {
        effort = 100;
    }

    mOutput->setOutput((uint8_t)effort);
}

void Control::advanceProfile(float curTemp, float dT) {
    const profile_point_t *p = mProfile + mProfileStage;

    if(p->duration == 0) {
        // Reached the end of the list
        mTargetTemp = 0;
    }
    
    if(p->ramp) {
        mProfileElapsed += dT;
        float startTemp = 25;
        if(mProfileStage > 0) {
            startTemp = mProfile[mProfileStage-1].temp;
        }
        
        mTargetTemp = startTemp + (p->temp - startTemp) * mProfileElapsed / p->duration;
    } else {
        mTargetTemp = p->temp;
        // We don't start counting until we reach the target temperature
        // Once the target temperature is reached, we continue counting even if
        // we go out of range
        if(mProfileElapsed > 0.0 || fabs(curTemp - mTargetTemp) < TEMP_MARGIN) {
            mProfileElapsed += dT;
        }
    }
    if(mProfileElapsed > p->duration) {
        mProfileStage++;
        mProfileElapsed = 0;
    }
}
#include "Display.h"

#include "profile.h"

#include <stdio.h>


#define TEMP_Y (40)

void Display::setTempReading(uint16_t temp_c) {
    mTempReading = temp_c;
}

void Display::setTempTarget(uint16_t temp_c) {
    mTempTarget = temp_c;
}

void Display::setOutput(uint8_t output) {
    mOutput = output;
}

void Display::setProfile(Profile &profile, uint16_t stage, uint16_t sec_into_stage) {
    mProfile = profile;
    mProfileStage = stage;
    mProfileTime = sec_into_stage;
}

void Display::setStatus(const char *status) {
    mStatus = status;
}

#define TEMP2Y(t) (top + height - ((t-minTemp) * height / (maxTemp - minTemp)))
void Display::renderProfile(uint16_t left, uint16_t top, uint16_t width, uint16_t height) {

    uint16_t totalTime = 0;
    uint16_t maxTemp = 0;
    uint16_t minTemp = 100;
    uint16_t x = left;
    uint8_t stageCount = 0;
    uint16_t prevTemp;

    for(int i=0; i<mProfile.size(); i++) {
        const ProfileStep *p = &mProfile[i];
        totalTime += p->duration;
        if(p->temp > maxTemp) {
            maxTemp = p->temp;
        }
        if(p->temp < minTemp) {
            minTemp = p->temp;
        }
        p++;
    }
    prevTemp = minTemp;

    for(int i=0; i<mProfile.size(); i++) {
        const ProfileStep *p = &mProfile[i];
        uint16_t start_x = x;
        uint16_t end_x = x + (width * p->duration) / totalTime;
        x = end_x + 1;
        if(p->ramp) {
            mDisplay->drawLine(start_x, TEMP2Y(prevTemp), end_x, TEMP2Y(p->temp));
        } else {
            mDisplay->drawLine(start_x, TEMP2Y(prevTemp), start_x, TEMP2Y(p->temp));
            mDisplay->drawLine(start_x, TEMP2Y(p->temp), end_x, TEMP2Y(p->temp));
        }
        prevTemp = p->temp;

        if(stageCount == mProfileStage) {
            // This is the active stage
            uint16_t xMark = start_x + (mProfileTime * (end_x - start_x)) / p->duration;
            mDisplay->drawLine(xMark, top, xMark, top+height);
        }

        stageCount++;
    }
    mDisplay->drawLine(left, top, left, top+height);
    mDisplay->drawLine(left, top+height, left+width, top+height);
    mDisplay->drawLine(left+width, top, left+width, top+height);
}

void Display::update() {
    char buf[12];

    mDisplay->clear();
    mDisplay->setFont(ArialMT_Plain_10);

    if(mStatus) {
        mDisplay->drawString(0, 0, mStatus);
    }
    snprintf(buf, sizeof(buf), "Stp %d %ds", mProfileStage, mProfileTime);
    mDisplay->drawString(0, 12, buf);
    mDisplay->drawString(0, 30, "Meas");
    mDisplay->drawString(48, 30, "Targ");
    mDisplay->setFont(ArialMT_Plain_24);
    snprintf(buf, 5, "%03d", mTempReading);
    mDisplay->drawString(0, TEMP_Y, buf);
    snprintf(buf, 5, "%03d", mTempTarget);
    mDisplay->drawString(48, TEMP_Y, buf);

    // Draw power bar ticks
    mDisplay->setPixel(125, 0);
    mDisplay->setPixel(125, 15);
    mDisplay->setPixel(125, 31);
    mDisplay->setPixel(125, 47);
    mDisplay->setPixel(124, 0);
    mDisplay->setPixel(124, 15);
    mDisplay->setPixel(124, 31);
    mDisplay->setPixel(124, 47);
    // Draw vertical power bar
    mDisplay->drawLine(127, 63 - (mOutput * 64) / 100, 127, 63);
    mDisplay->drawLine(126, 63 - (mOutput * 64) / 100, 126, 63);

    renderProfile(63, 0, 58, 28);
    
    mDisplay->display();
}
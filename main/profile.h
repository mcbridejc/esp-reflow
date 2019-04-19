#pragma once

#include <stdint.h>
#include <string>
#include <vector>

struct ProfileStep {
    uint16_t temp;
    uint16_t duration;
    uint8_t ramp;
};

class Profile {
public:
    Profile() {
    }

    Profile(const char *name, const ProfileStep* points=NULL) {
        mName = name;
        while(points != NULL && points->duration != 0) {
            mPoints.push_back(*points);
            points++;
        }
    }

    const char *name() { return mName.c_str(); };
    //ProfileStep *points() { return &mPoints[0]; };

    void addStep(ProfileStep &point) {
        mPoints.push_back(point);
    }

    uint32_t size() { 
        return mPoints.size();
    }

    bool empty() {
        return mName.empty() || mPoints.size() == 0;
    }

    ProfileStep &operator[](int index) {
        return mPoints[index];
    }
    
private:
    std::string mName;
    std::vector<ProfileStep> mPoints;
};


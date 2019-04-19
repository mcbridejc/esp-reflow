#include "ProfileManager.h"

#include "json/json.h"

#include "esp_log.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


static const char *TAG = "ProfileManager"; // Logging tag

ProfileManager::ProfileManager(const char *config_path) : mActiveProfile("empty") {
  mConfigPath = config_path;
}

void ProfileManager::init() {
  readProfiles();
}

void ProfileManager::save() {
  saveProfiles();
}

bool ProfileManager::setActiveProfile(const char *name) {
  int idx = findIndexByName(name);
  if(idx >= 0) {
    mActiveProfile = mProfiles[idx];
    return true;
  }
  return false;
}

Profile& ProfileManager::getActiveProfile() {
  return mActiveProfile;
}

std::vector<Profile> ProfileManager::getAllProfiles() {
  return mProfiles;
}

bool ProfileManager::createProfile(Profile &new_profile) {
  if(strlen(new_profile.name()) < 1) {
    return false; // Don't allow empty name
  }

  // Banned names
  if(strcmp(new_profile.name(), "active") == 0) {
    return false;
  }

  int idx = findIndexByName(new_profile.name());
  if(idx >= 0) {
    return false; // Don't allow duplicate names
  }
  
  mProfiles.push_back(new_profile);
  return true;
}

bool ProfileManager::updateProfile(const char *name, Profile &new_profile) {
  // Banned names
  if(strcmp(new_profile.name(), "active") == 0) {
    return false;
  }

  int idx = findIndexByName(name);
  if(idx < 0) {
    return false; // Profile to be updated doesn't exist
  }
  mProfiles[idx] = new_profile;
  return true;
}

bool ProfileManager::deleteProfile(const char *name) {
  int idx = findIndexByName(name);
  if(idx < 0) {
    return false;
  }
  mProfiles.erase(mProfiles.begin() + idx);
  return true;
}

int ProfileManager::findIndexByName(const char *name) {
  for(int i=0; i<mProfiles.size(); i++) {
    if(strcmp(mProfiles[i].name(), name) == 0) {
      return i;
    }
  }
  return -1; // None found
}

void ProfileManager::readProfiles() {
  Json::Value root;
  Json::Reader jsonReader;
  struct stat finfo;
  char *buf;

  FILE *f = fopen(mConfigPath.c_str(), "r");
  if(f == NULL) {
    ESP_LOGW(TAG, "Failed to open %s, will use and save default profiles", mConfigPath.c_str());
    loadDefaultProfiles();
    saveProfiles();
    return;
  }

  fstat(fileno(f), &finfo);

  buf = (char *)malloc(finfo.st_size);
  if(buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocation buffer of size %d for reading profiles. No profiles loaded.", (int)finfo.st_size);
    return;
  }
  fread(buf, 1, finfo.st_size, f);
  fclose(f);
  bool success = jsonReader.parse(buf, buf+finfo.st_size, root, false);
  free(buf);

  if(!success) {
    ESP_LOGW(TAG, "Failed to parse %s, will use and save default profiles", mConfigPath.c_str());
    loadDefaultProfiles();
    saveProfiles();
    return;
  }

  // Root object is a dict; keys are the names of the profiles, values are an array of 
  // step objects like [{"temp": 0, "duration": 50, "ramp": 0},...]
  try {
    std::vector<std::string> names = root["profiles"].getMemberNames();
    for(auto it=names.begin(); it != names.end(); it++) {
      Profile newProfile((*it).c_str());
      Json::Value profileNode = root[*it];
      for(int i=0; i < profileNode.size(); i++) {
        ProfileStep point = {
          .temp =  (uint16_t)profileNode[i]["temp"].asInt(),
          .duration = (uint16_t)profileNode[i]["duration"].asInt(),
          .ramp = (uint8_t)profileNode[i]["ramp"].asInt()
        };
        newProfile.addStep(point);
      }
      mProfiles.push_back(newProfile);
    }
    std::string activeProfileName = root["activeProfile"].asString();
    int activeIdx = findIndexByName(activeProfileName.c_str());
    if(activeIdx < 0 && mProfiles.size() > 0) {
      activeIdx = 0;
    }
    if(activeIdx >= 0) {
      mActiveProfile = mProfiles[activeIdx];
    }
  } catch (Json::LogicError &ex) {
    ESP_LOGE(TAG, "Error parsing profiles JSON: %s", ex.what());
  }

}

void ProfileManager::saveProfiles() {
  Json::Value root;
  Json::FastWriter jsonWriter;
  for(auto profile=mProfiles.begin(); profile!=mProfiles.end(); profile++) {
    Json::Value stepsNode = Json::arrayValue;
    for(int stepIndex = 0; stepIndex < (*profile).size(); stepIndex++) {
      Json::Value stepObj;
      stepObj["temp"] = (*profile)[stepIndex].temp;
      stepObj["duration"] = (*profile)[stepIndex].duration;
      stepObj["ramp"] = (*profile)[stepIndex].ramp;
      stepsNode[stepIndex] =stepObj;
    }
    root["profiles"][(*profile).name()] = stepsNode;
  }
  root["activeProfile"] = mActiveProfile.name();

  FILE *f = fopen(mConfigPath.c_str(), "w");
  if(f == NULL) {
    ESP_LOGE(TAG, "Error opening %s for write", mConfigPath.c_str());
    return;
  }
  std::string data = jsonWriter.write(root);
  fwrite(&data[0], 1, data.size(), f);
  fclose(f);
}

static const ProfileStep DEFAULT_PROFILE[] = {
    {150, 60, 0}, // Preheat
    {180, 120, 1}, // Soak
    {245, 40, 0}, // Reflow
    {150, 25, 1}, // Cooldown
    {0, 0, 0}, // Terminator
};

void ProfileManager::loadDefaultProfiles() {
  Profile profile("default", DEFAULT_PROFILE);
  mProfiles.push_back(profile);
}


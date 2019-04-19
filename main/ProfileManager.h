#pragma once

#include "Control.h"
#include "profile.h"

#include <vector>

class ProfileManager {
public:
  ProfileManager(const char *config_path);

  /** Must be called on boot, after the filesystem is ready, to initialize the
   * object and read the profiles stored on the SPIFFS */
  void init();

  /** Select the active profile
   * 
   * Returns false if the profile does not exist 
   */
  bool setActiveProfile(const char *name);

  /** Get a copy of the active profile 
   * 
   * This object includes its own allocation, so that it will outlive the 
   * possibility of being deleted while active. 
   */
  Profile& getActiveProfile();

  std::vector<Profile> getAllProfiles();

  bool createProfile(Profile &new_profile);

  bool updateProfile(const char *name, Profile &new_profile);

  bool deleteProfile(const char *name);

  void save();

private:
  //Control *mControl;
  std::string mConfigPath;
  std::vector<Profile> mProfiles;
  Profile mActiveProfile;
  
  void readProfiles();
  void saveProfiles();
  void loadDefaultProfiles();
  int findIndexByName(const char *name);
};

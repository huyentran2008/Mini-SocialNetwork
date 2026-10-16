#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include "UserManager.h"
#include "Profile.h"
#include <string>
#include <vector>

class ProfileManager {
private:
    UserManager* userManager;

public:
    ProfileManager(UserManager* um);

    // Required methods
    void viewProfile(int userId) const;
    bool editProfile(int userId, const std::string& fullName, const std::string& city,
                     const std::string& hometown, const std::string& school,
                     const std::string& workplace, const std::vector<std::string>& interests);

    // Helpers
    void printProfile(const Profile& profile) const;
};

#endif

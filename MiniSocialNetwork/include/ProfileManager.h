#ifndef PROFILE_MANAGER_H
#define PROFILE_MANAGER_H

#include "UserManager.h"
#include "Profile.h"
#include "AttributeIndex.h"

class ProfileManager {
private:
    UserManager* userManager;
    AttributeIndex* attributeIndex;

public:
    ProfileManager(UserManager* um, AttributeIndex* ai);

    // Lay ho so cua mot nguoi dung theo ID (tra ve ban sao)
    Profile viewProfile(int userId);

    // Sua ho so cua mot nguoi dung theo ID
    bool editProfile(int userId, const Profile& newProfile);
};

#endif

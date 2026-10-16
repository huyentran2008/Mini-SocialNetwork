#include "../include/ProfileManager.h"

ProfileManager::ProfileManager(UserManager* um, AttributeIndex* ai)
    : userManager(um), attributeIndex(ai) {}

Profile ProfileManager::viewProfile(int userId) {
    User* user = userManager->getUserById(userId);
    if (user == nullptr) {
        return Profile(); // tra ve profile rong neu khong tim thay
    }
    return user->getProfile();
}

bool ProfileManager::editProfile(int userId, const Profile& newProfile) {
    User* user = userManager->getUserById(userId);
    if (user == nullptr) return false;
    if (!newProfile.validateProfile()) return false;

    Profile oldProfile = user->getProfile();
    user->setProfile(newProfile);

    // Cap nhat AttributeIndex
    if (attributeIndex != nullptr) {
        attributeIndex->updateUserInIndex(userId, oldProfile, newProfile);
    }

    return true;
}

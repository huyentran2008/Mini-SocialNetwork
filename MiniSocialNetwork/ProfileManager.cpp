#include "ProfileManager.h"
#include <iostream>
#include <iomanip>

ProfileManager::ProfileManager(UserManager* um) : userManager(um) {}

void ProfileManager::printProfile(const Profile& profile) const {
    std::cout << "----------------------------------------\n";
    std::cout << profile.getProfileSummary() << "\n";
    std::cout << "----------------------------------------\n";
}

void ProfileManager::viewProfile(int userId) const {
    User* user = userManager->getUserById(userId);
    if (!user) {
        std::cout << "Khong tim thay nguoi dung!\n";
        return;
    }
    std::cout << "\n=== HO SO NGUOI DUNG ===\n";
    std::cout << "ID: " << user->getId() << "\n";
    std::cout << "Username: " << user->getUsername() << "\n";
    std::cout << "So ban be: " << user->getFriendCount() << "\n";
    printProfile(user->getProfile());
}

bool ProfileManager::editProfile(int userId, const std::string& fullName, const std::string& city,
                                 const std::string& hometown, const std::string& school,
                                 const std::string& workplace, const std::vector<std::string>& interests) {
    User* user = userManager->getUserById(userId);
    if (!user) return false;

    Profile& profile = user->getProfile();
    profile.updateProfile(fullName, city, hometown, school, workplace, interests);
    return profile.validateProfile();
}

#include "User.h"
#include <sstream>
#include <vector>

User::User() : id(0), isAdmin(false), friendCount(0) {}

User::User(int id, const std::string& username, const std::string& password, bool isAdmin)
    : id(id), username(username), password(password), isAdmin(isAdmin), friendCount(0) {}

int User::getId() const { return id; }
std::string User::getUsername() const { return username; }
Profile& User::getProfile() { return profile; }
const Profile& User::getProfile() const { return profile; }
void User::setProfile(const Profile& profile) { this->profile = profile; }
int User::getFriendCount() const { return friendCount; }

std::string User::getPassword() const { return password; }
bool User::getIsAdmin() const { return isAdmin; }
void User::setFriendCount(int count) { friendCount = count; }
void User::incrementFriendCount() { ++friendCount; }
void User::decrementFriendCount() { if (friendCount > 0) --friendCount; }

std::string User::serialize() const {
    std::ostringstream oss;
    oss << id << "|" << username << "|" << password << "|"
        << (isAdmin ? "1" : "0") << "|" << friendCount << "|"
        << profile.serialize();
    return oss.str();
}

void User::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    std::vector<std::string> parts;
    while (std::getline(iss, token, '|')) {
        parts.push_back(token);
    }
    if (parts.size() >= 6) {
        id = std::stoi(parts[0]);
        username = parts[1];
        password = parts[2];
        isAdmin = (parts[3] == "1");
        friendCount = std::stoi(parts[4]);
        // Reconstruct profile from remaining parts
        std::string profileData;
        for (size_t i = 5; i < parts.size(); ++i) {
            if (i > 5) profileData += "|";
            profileData += parts[i];
        }
        profile.deserialize(profileData);
    }
}

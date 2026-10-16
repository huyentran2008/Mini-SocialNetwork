#include "../include/User.h"

User::User() : id(-1), friendCount(0) {}

User::User(int id, const std::string& username)
    : id(id), username(username), friendCount(0) {}

int User::getId() const { return id; }

std::string User::getUsername() const { return username; }

Profile User::getProfile() const { return profile; }

void User::setProfile(const Profile& profile) {
    this->profile = profile;
}

int User::getFriendCount() const { return friendCount; }

void User::setFriendCount(int count) {
    friendCount = count;
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "User[ID=" << user.id
       << ", Username=" << user.username
       << ", Friends=" << user.friendCount << "]\n"
       << user.profile;
    return os;
}

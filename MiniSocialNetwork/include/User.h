#ifndef USER_H
#define USER_H

#include "Profile.h"
#include <string>
#include <iostream>

class User {
private:
    int id;
    std::string username;
    Profile profile;
    int friendCount;   // duoc cap nhat tu Graph (bac cua dinh)

public:
    User();
    User(int id, const std::string& username);

    int getId() const;
    std::string getUsername() const;
    Profile getProfile() const;
    void setProfile(const Profile& profile);
    int getFriendCount() const;
    void setFriendCount(int count);

    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

#endif

#ifndef USER_H
#define USER_H

#include <string>
#include "Profile.h"

class User {
private:
    int id;
    std::string username;
    std::string password;
    bool isAdmin;
    Profile profile;
    int friendCount;

public:
    User();
    User(int id, const std::string& username, const std::string& password, bool isAdmin = false);

    // Required methods
    int getId() const;
    std::string getUsername() const;
    Profile& getProfile();
    const Profile& getProfile() const;
    void setProfile(const Profile& profile);
    int getFriendCount() const;

    // Additional necessary methods
    std::string getPassword() const;
    bool getIsAdmin() const;
    void setFriendCount(int count);
    void incrementFriendCount();
    void decrementFriendCount();
    std::string serialize() const;
    void deserialize(const std::string& data);
};

#endif

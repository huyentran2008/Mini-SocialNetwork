#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "UserManager.h"
#include <string>

class FileManager {
private:
    std::string dataDir;
    std::string usersFile;
    std::string friendsFile;
    std::string requestsFile;

    bool ensureDataDir() const;

public:
    FileManager(const std::string& dir = "data");

    bool saveAll(const UserManager& um) const;
    bool loadAll(UserManager& um) const;

    bool saveUsers(const UserManager& um) const;
    bool loadUsers(UserManager& um) const;
    bool saveFriends(const UserManager& um) const;
    bool loadFriends(UserManager& um) const;
    bool saveRequests(const UserManager& um) const;
    bool loadRequests(UserManager& um) const;
};

#endif

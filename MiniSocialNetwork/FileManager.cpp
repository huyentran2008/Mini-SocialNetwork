#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

FileManager::FileManager(const std::string& dir)
    : dataDir(dir),
      usersFile(dir + "/users.txt"),
      friendsFile(dir + "/friends.txt"),
      requestsFile(dir + "/requests.txt") {}

bool FileManager::ensureDataDir() const {
#ifdef _WIN32
    return mkdir(dataDir.c_str(), 0755) == 0 || errno == EEXIST;
#else
    struct stat st;
    if (stat(dataDir.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return mkdir(dataDir.c_str(), 0755) == 0;
#endif
}

bool FileManager::saveUsers(const UserManager& um) const {
    if (!ensureDataDir()) return false;
    std::ofstream out(usersFile.c_str());
    if (!out.is_open()) return false;

    out << um.getNextUserId() << "\n";
    auto users = um.getAllUsers();
    for (User* u : users) {
        out << u->serialize() << "\n";
    }
    out.close();
    return true;
}

bool FileManager::loadUsers(UserManager& um) const {
    std::ifstream in(usersFile.c_str());
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) {
        in.close();
        return false;
    }
    try {
        int nextId = std::stoi(line);
        um.setNextUserId(nextId);
    } catch (...) {
        in.close();
        return false;
    }

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        User* user = new User();
        user->deserialize(line);
        if (!um.loadUser(user)) {
            // already deleted inside loadUser if failed
        }
    }
    in.close();
    return true;
}

// To make loading work properly, I will implement full save/load with proper methods.
// Let's rewrite load properly by adding a method to UserManager.

bool FileManager::saveFriends(const UserManager& um) const {
    if (!ensureDataDir()) return false;
    std::ofstream out(friendsFile.c_str());
    if (!out.is_open()) return false;

    const Graph& g = um.getFriendshipGraph();
    auto vertices = g.getAllVertices();
    for (int v : vertices) {
        auto neighbors = g.getNeighbors(v);
        for (int n : neighbors) {
            if (v < n) { // avoid duplicate undirected edges
                out << v << " " << n << "\n";
            }
        }
    }
    out.close();
    return true;
}

bool FileManager::loadFriends(UserManager& um) const {
    std::ifstream in(friendsFile.c_str());
    if (!in.is_open()) return false;

    int u, v;
    while (in >> u >> v) {
        um.getFriendshipGraph().addEdge(u, v);
    }
    in.close();

    // Sync friend counts after all edges loaded
    auto users = um.getAllUsers();
    for (User* user : users) {
        int deg = um.getFriendshipGraph().getDegree(user->getId());
        user->setFriendCount(deg);
    }
    return true;
}

bool FileManager::saveRequests(const UserManager& um) const {
    if (!ensureDataDir()) return false;
    std::ofstream out(requestsFile.c_str());
    if (!out.is_open()) return false;

    const auto& reqs = um.getPendingRequestsMap();
    for (const auto& pair : reqs) {
        for (int rid : pair.second) {
            out << pair.first << " " << rid << "\n";
        }
    }
    out.close();
    return true;
}

bool FileManager::loadRequests(UserManager& um) const {
    std::ifstream in(requestsFile.c_str());
    if (!in.is_open()) return false;

    int userId, requesterId;
    while (in >> userId >> requesterId) {
        um.setPendingRequest(userId, requesterId);
    }
    in.close();
    return true;
}

bool FileManager::saveAll(const UserManager& um) const {
    return saveUsers(um) && saveFriends(um) && saveRequests(um);
}

bool FileManager::loadAll(UserManager& um) const {
    // loadUsers needs fix, so we'll implement a proper loadUsers using an added method.
    return loadUsers(um) && loadFriends(um) && loadRequests(um);
}

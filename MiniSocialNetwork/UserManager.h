#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <vector>
#include <map>
#include "User.h"
#include "HashTable.h"
#include "Graph.h"

class UserManager {
private:
    HashTable usersByUsername; // key = username
    std::map<int, User*> usersById;
    Graph friendshipGraph;
    int nextUserId;
    User* currentUser;

    // Pending friend requests: userId -> list of requesterIds
    std::map<int, std::vector<int>> pendingRequests;

public:
    UserManager();
    ~UserManager();

    // Required methods
    User* registerUser(const std::string& username, const std::string& password, bool isAdmin = false);
    bool deleteUser(int userId);
    User* getUserById(int userId) const;
    User* getUserByUsername(const std::string& username) const;
    bool isUserExists(int userId) const;
    bool isUsernameTaken(const std::string& username) const;
    int generateUserId();
    std::vector<User*> getAllUsers() const;
    int getUserCount() const;

    // Additional necessary
    User* login(const std::string& username, const std::string& password);
    void logout();
    User* getCurrentUser() const;
    bool isLoggedIn() const;

    // Friendship
    bool sendFriendRequest(int fromId, int toId);
    bool acceptFriendRequest(int userId, int requesterId);
    bool rejectFriendRequest(int userId, int requesterId);
    bool removeFriend(int userId1, int userId2);
    std::vector<User*> getFriends(int userId) const;
    std::vector<User*> getPendingRequests(int userId) const;
    bool areFriends(int userId1, int userId2) const;
    Graph& getFriendshipGraph();
    const Graph& getFriendshipGraph() const;

    // For file I/O
    void setNextUserId(int id);
    int getNextUserId() const;
    const std::map<int, std::vector<int>>& getPendingRequestsMap() const;
    void setPendingRequest(int userId, int requesterId);
    void clearAll();
    bool loadUser(User* user); // Takes ownership of the pointer
};

#endif

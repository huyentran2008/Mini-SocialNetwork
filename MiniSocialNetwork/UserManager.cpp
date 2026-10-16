#include "UserManager.h"
#include <algorithm>

UserManager::UserManager() : nextUserId(1), currentUser(nullptr) {}

UserManager::~UserManager() {
    clearAll();
}

void UserManager::clearAll() {
    auto all = usersByUsername.getAllValues();
    for (User* u : all) {
        delete u;
    }
    usersByUsername.clear();
    usersById.clear();
    friendshipGraph.clear();
    pendingRequests.clear();
    currentUser = nullptr;
    nextUserId = 1;
}

User* UserManager::registerUser(const std::string& username, const std::string& password, bool isAdmin) {
    if (isUsernameTaken(username)) {
        return nullptr;
    }
    int id = generateUserId();
    User* user = new User(id, username, password, isAdmin);
    usersByUsername.insert(username, user);
    usersById[id] = user;
    friendshipGraph.addVertex(id);
    return user;
}

bool UserManager::deleteUser(int userId) {
    User* user = getUserById(userId);
    if (!user) return false;

    // Remove all friendships
    auto neighbors = friendshipGraph.getNeighbors(userId);
    for (int friendId : neighbors) {
        friendshipGraph.removeEdge(userId, friendId);
        User* friendUser = getUserById(friendId);
        if (friendUser) {
            friendUser->decrementFriendCount();
        }
    }
    friendshipGraph.removeVertex(userId);

    // Remove pending requests involving this user
    pendingRequests.erase(userId);
    for (auto& pair : pendingRequests) {
        auto& reqs = pair.second;
        reqs.erase(std::remove(reqs.begin(), reqs.end(), userId), reqs.end());
    }

    std::string username = user->getUsername();
    usersByUsername.remove(username);
    usersById.erase(userId);
    if (currentUser == user) {
        currentUser = nullptr;
    }
    delete user;
    return true;
}

User* UserManager::getUserById(int userId) const {
    auto it = usersById.find(userId);
    if (it != usersById.end()) {
        return it->second;
    }
    return nullptr;
}

User* UserManager::getUserByUsername(const std::string& username) const {
    return usersByUsername.search(username);
}

bool UserManager::isUserExists(int userId) const {
    return getUserById(userId) != nullptr;
}

bool UserManager::isUsernameTaken(const std::string& username) const {
    return usersByUsername.contains(username);
}

int UserManager::generateUserId() {
    return nextUserId++;
}

std::vector<User*> UserManager::getAllUsers() const {
    return usersByUsername.getAllValues();
}

int UserManager::getUserCount() const {
    return usersByUsername.getSize();
}

User* UserManager::login(const std::string& username, const std::string& password) {
    User* user = getUserByUsername(username);
    if (user && user->getPassword() == password) {
        currentUser = user;
        return user;
    }
    return nullptr;
}

void UserManager::logout() {
    currentUser = nullptr;
}

User* UserManager::getCurrentUser() const {
    return currentUser;
}

bool UserManager::isLoggedIn() const {
    return currentUser != nullptr;
}

bool UserManager::sendFriendRequest(int fromId, int toId) {
    if (fromId == toId) return false;
    if (!isUserExists(fromId) || !isUserExists(toId)) return false;
    if (areFriends(fromId, toId)) return false;

    // Check if already pending
    auto& reqs = pendingRequests[toId];
    if (std::find(reqs.begin(), reqs.end(), fromId) != reqs.end()) {
        return false;
    }
    // Also check reverse
    auto& reverseReqs = pendingRequests[fromId];
    if (std::find(reverseReqs.begin(), reverseReqs.end(), toId) != reverseReqs.end()) {
        return false;
    }

    pendingRequests[toId].push_back(fromId);
    return true;
}

bool UserManager::acceptFriendRequest(int userId, int requesterId) {
    auto it = pendingRequests.find(userId);
    if (it == pendingRequests.end()) return false;

    auto& reqs = it->second;
    auto pos = std::find(reqs.begin(), reqs.end(), requesterId);
    if (pos == reqs.end()) return false;

    reqs.erase(pos);
    friendshipGraph.addEdge(userId, requesterId);

    User* u1 = getUserById(userId);
    User* u2 = getUserById(requesterId);
    if (u1) u1->incrementFriendCount();
    if (u2) u2->incrementFriendCount();
    return true;
}

bool UserManager::rejectFriendRequest(int userId, int requesterId) {
    auto it = pendingRequests.find(userId);
    if (it == pendingRequests.end()) return false;

    auto& reqs = it->second;
    auto pos = std::find(reqs.begin(), reqs.end(), requesterId);
    if (pos == reqs.end()) return false;

    reqs.erase(pos);
    return true;
}

bool UserManager::removeFriend(int userId1, int userId2) {
    if (!areFriends(userId1, userId2)) return false;
    friendshipGraph.removeEdge(userId1, userId2);
    User* u1 = getUserById(userId1);
    User* u2 = getUserById(userId2);
    if (u1) u1->decrementFriendCount();
    if (u2) u2->decrementFriendCount();
    return true;
}

std::vector<User*> UserManager::getFriends(int userId) const {
    std::vector<User*> result;
    auto neighbors = friendshipGraph.getNeighbors(userId);
    for (int fid : neighbors) {
        User* u = getUserById(fid);
        if (u) result.push_back(u);
    }
    return result;
}

std::vector<User*> UserManager::getPendingRequests(int userId) const {
    std::vector<User*> result;
    auto it = pendingRequests.find(userId);
    if (it != pendingRequests.end()) {
        for (int rid : it->second) {
            User* u = getUserById(rid);
            if (u) result.push_back(u);
        }
    }
    return result;
}

bool UserManager::areFriends(int userId1, int userId2) const {
    auto neighbors = friendshipGraph.getNeighbors(userId1);
    return std::find(neighbors.begin(), neighbors.end(), userId2) != neighbors.end();
}

Graph& UserManager::getFriendshipGraph() {
    return friendshipGraph;
}

const Graph& UserManager::getFriendshipGraph() const {
    return friendshipGraph;
}

void UserManager::setNextUserId(int id) {
    nextUserId = id;
}

int UserManager::getNextUserId() const {
    return nextUserId;
}

const std::map<int, std::vector<int>>& UserManager::getPendingRequestsMap() const {
    return pendingRequests;
}

void UserManager::setPendingRequest(int userId, int requesterId) {
    pendingRequests[userId].push_back(requesterId);
}

bool UserManager::loadUser(User* user) {
    if (!user) return false;
    if (isUsernameTaken(user->getUsername()) || isUserExists(user->getId())) {
        delete user;
        return false;
    }
    usersByUsername.insert(user->getUsername(), user);
    usersById[user->getId()] = user;
    friendshipGraph.addVertex(user->getId());
    if (user->getId() >= nextUserId) {
        nextUserId = user->getId() + 1;
    }
    return true;
}

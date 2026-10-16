#include "../include/UserManager.h"

UserManager::UserManager(Graph* graph, AttributeIndex* attrIndex)
    : friendshipGraph(graph), attributeIndex(attrIndex), nextUserId(1) {}

UserManager::~UserManager() {
    // Giai phong tat ca User*
    std::vector<User*> all = usersById.getAllValues();
    for (size_t i = 0; i < all.size(); ++i) {
        delete all[i];
    }
    // HashTable se tu clear trong destructor
}

int UserManager::generateUserId() {
    return nextUserId++;
}

bool UserManager::isUsernameTaken(const std::string& username) {
    return usersByUsername.contains(username);
}

bool UserManager::isUserExists(int userId) {
    return usersById.contains(userId);
}

User* UserManager::registerUser(const std::string& username, const Profile& profile) {
    if (isUsernameTaken(username)) {
        return nullptr; // username da ton tai
    }
    if (!profile.validateProfile()) {
        return nullptr; // ho so khong hop le
    }

    int id = generateUserId();
    User* newUser = new User(id, username);
    newUser->setProfile(profile);

    usersById.insert(id, newUser);
    usersByUsername.insert(username, newUser);

    // Them dinh vao do thi
    if (friendshipGraph != nullptr) {
        friendshipGraph->addVertex(id);
    }

    // Them vao AttributeIndex
    if (attributeIndex != nullptr) {
        attributeIndex->addUserToIndex(id, profile);
    }

    return newUser;
}

bool UserManager::deleteUser(int userId) {
    User** pUser = usersById.search(userId);
    if (pUser == nullptr || *pUser == nullptr) {
        return false;
    }

    User* user = *pUser;
    std::string username = user->getUsername();
    Profile profile = user->getProfile();

    // Xoa khoi AttributeIndex
    if (attributeIndex != nullptr) {
        attributeIndex->removeUserFromIndex(userId, profile);
    }

    // Xoa dinh khoi Graph
    if (friendshipGraph != nullptr) {
        friendshipGraph->removeVertex(userId);
    }

    // Xoa khoi 2 bang bam
    usersById.remove(userId);
    usersByUsername.remove(username);

    delete user;
    return true;
}

User* UserManager::getUserById(int userId) {
    User** pUser = usersById.search(userId);
    if (pUser == nullptr) return nullptr;
    return *pUser;
}

User* UserManager::getUserByUsername(const std::string& username) {
    User** pUser = usersByUsername.search(username);
    if (pUser == nullptr) return nullptr;
    return *pUser;
}

std::vector<User*> UserManager::getAllUsers() {
    return usersById.getAllValues();
}

int UserManager::getUserCount() const {
    return usersById.getSize();
}

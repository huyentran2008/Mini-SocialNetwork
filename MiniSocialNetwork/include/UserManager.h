#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "User.h"
#include "HashTable.h"
#include "Graph.h"
#include "AttributeIndex.h"
#include <string>
#include <vector>

class UserManager {
private:
    HashTable<int, User*> usersById;              // ID -> User*
    HashTable<std::string, User*> usersByUsername; // username -> User*
    Graph* friendshipGraph;                       // quan he ban be
    AttributeIndex* attributeIndex;
    int nextUserId;

public:
    UserManager(Graph* graph, AttributeIndex* attrIndex);
    ~UserManager();

    // Dang ky nguoi dung moi, them vao HashTable va Graph
    User* registerUser(const std::string& username, const Profile& profile);

    // Xoa nguoi dung khoi HashTable va Graph
    bool deleteUser(int userId);

    // Lay nguoi dung theo ID
    User* getUserById(int userId);

    // Lay nguoi dung theo username
    User* getUserByUsername(const std::string& username);

    // Kiem tra nguoi dung co ton tai khong
    bool isUserExists(int userId);

    // Kiem tra username da bi dung chua
    bool isUsernameTaken(const std::string& username);

    // Sinh ID tu tang, khong trung
    int generateUserId();

    // Lay danh sach toan bo nguoi dung
    std::vector<User*> getAllUsers();

    // Dem tong so nguoi dung
    int getUserCount() const;
};

#endif

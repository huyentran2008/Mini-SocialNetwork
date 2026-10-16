#ifndef ATTRIBUTE_INDEX_H
#define ATTRIBUTE_INDEX_H

#include "HashTable.h"
#include "Profile.h"
#include <vector>
#include <string>

class AttributeIndex {
private:
    // Moi thuoc tinh -> danh sach userId
    HashTable<std::string, std::vector<int>> cityIndex;
    HashTable<std::string, std::vector<int>> hometownIndex;
    HashTable<std::string, std::vector<int>> schoolIndex;
    HashTable<std::string, std::vector<int>> workplaceIndex;
    HashTable<std::string, std::vector<int>> interestIndex;

    void addToIndex(HashTable<std::string, std::vector<int>>& index,
                    const std::string& key, int userId);

    void removeFromIndex(HashTable<std::string, std::vector<int>>& index,
                         const std::string& key, int userId);

public:
    AttributeIndex();

    // Them nguoi dung vao chi muc theo cac thuoc tinh trong ho so
    void addUserToIndex(int userId, const Profile& profile);

    // Xoa nguoi dung khoi chi muc (khi xoa tai khoan)
    void removeUserFromIndex(int userId, const Profile& profile);

    // Cap nhat chi muc khi nguoi dung sua ho so
    void updateUserInIndex(int userId, const Profile& oldProfile, const Profile& newProfile);

    // Lay danh sach nguoi cung noi o
    std::vector<int> getUsersByCity(const std::string& city);

    // Lay danh sach nguoi cung que quan
    std::vector<int> getUsersByHometown(const std::string& hometown);

    // Lay danh sach nguoi cung truong
    std::vector<int> getUsersBySchool(const std::string& school);

    // Lay danh sach nguoi cung noi lam viec
    std::vector<int> getUsersByWorkplace(const std::string& workplace);

    // Lay danh sach nguoi cung so thich
    std::vector<int> getUsersByInterest(const std::string& interest);
};

#endif

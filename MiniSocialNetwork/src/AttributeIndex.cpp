#include "../include/AttributeIndex.h"
#include <algorithm>

AttributeIndex::AttributeIndex() {}

void AttributeIndex::addToIndex(HashTable<std::string, std::vector<int>>& index,
                                const std::string& key, int userId) {
    if (key.empty()) return;

    std::vector<int>* list = index.search(key);
    if (list == nullptr) {
        std::vector<int> newList;
        newList.push_back(userId);
        index.insert(key, newList);
    } else {
        // Tranh trung
        if (std::find(list->begin(), list->end(), userId) == list->end()) {
            list->push_back(userId);
        }
    }
}

void AttributeIndex::removeFromIndex(HashTable<std::string, std::vector<int>>& index,
                                     const std::string& key, int userId) {
    if (key.empty()) return;

    std::vector<int>* list = index.search(key);
    if (list != nullptr) {
        list->erase(std::remove(list->begin(), list->end(), userId), list->end());
    }
}

void AttributeIndex::addUserToIndex(int userId, const Profile& profile) {
    addToIndex(cityIndex, profile.getCity(), userId);
    addToIndex(hometownIndex, profile.getHometown(), userId);
    addToIndex(schoolIndex, profile.getSchool(), userId);
    addToIndex(workplaceIndex, profile.getWorkplace(), userId);

    std::vector<std::string> interests = profile.getInterests();
    for (size_t i = 0; i < interests.size(); ++i) {
        addToIndex(interestIndex, interests[i], userId);
    }
}

void AttributeIndex::removeUserFromIndex(int userId, const Profile& profile) {
    removeFromIndex(cityIndex, profile.getCity(), userId);
    removeFromIndex(hometownIndex, profile.getHometown(), userId);
    removeFromIndex(schoolIndex, profile.getSchool(), userId);
    removeFromIndex(workplaceIndex, profile.getWorkplace(), userId);

    std::vector<std::string> interests = profile.getInterests();
    for (size_t i = 0; i < interests.size(); ++i) {
        removeFromIndex(interestIndex, interests[i], userId);
    }
}

void AttributeIndex::updateUserInIndex(int userId, const Profile& oldProfile, const Profile& newProfile) {
    removeUserFromIndex(userId, oldProfile);
    addUserToIndex(userId, newProfile);
}

std::vector<int> AttributeIndex::getUsersByCity(const std::string& city) {
    std::vector<int>* list = cityIndex.search(city);
    if (list != nullptr) return *list;
    return std::vector<int>();
}

std::vector<int> AttributeIndex::getUsersByHometown(const std::string& hometown) {
    std::vector<int>* list = hometownIndex.search(hometown);
    if (list != nullptr) return *list;
    return std::vector<int>();
}

std::vector<int> AttributeIndex::getUsersBySchool(const std::string& school) {
    std::vector<int>* list = schoolIndex.search(school);
    if (list != nullptr) return *list;
    return std::vector<int>();
}

std::vector<int> AttributeIndex::getUsersByWorkplace(const std::string& workplace) {
    std::vector<int>* list = workplaceIndex.search(workplace);
    if (list != nullptr) return *list;
    return std::vector<int>();
}

std::vector<int> AttributeIndex::getUsersByInterest(const std::string& interest) {
    std::vector<int>* list = interestIndex.search(interest);
    if (list != nullptr) return *list;
    return std::vector<int>();
}

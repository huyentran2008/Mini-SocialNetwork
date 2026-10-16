#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include "User.h"

struct HashNode {
    std::string key; // username
    User* value;
    HashNode* next;
    HashNode(const std::string& k, User* v) : key(k), value(v), next(nullptr) {}
};

class HashTable {
private:
    HashNode** table;
    int capacity;
    int size;
    static const int INITIAL_CAPACITY = 16;
    static const float LOAD_FACTOR_THRESHOLD;

    int hashFunction(const std::string& key) const;
    void handleCollision(int index, const std::string& key, User* value);
    void rehash();

public:
    HashTable();
    ~HashTable();

    // Required methods
    int hashFunction(const std::string& key); // non-const version for interface
    void insert(const std::string& key, User* value);
    User* search(const std::string& key) const;
    bool remove(const std::string& key);
    bool contains(const std::string& key) const;
    float getLoadFactor() const;
    int getSize() const;
    std::vector<User*> getAllValues() const;
    void clear();

    // Index related (for profile attributes if needed)
    void addUserToIndex(User* user);
    void removeUserFromIndex(User* user);
    void updateUserIndex(User* user);

    // Search by profile attributes
    std::vector<User*> searchByCity(const std::string& city) const;
    std::vector<User*> searchByHometown(const std::string& hometown) const;
    std::vector<User*> searchBySchool(const std::string& school) const;
    std::vector<User*> searchByWorkplace(const std::string& workplace) const;
    std::vector<User*> searchByInterest(const std::string& interest) const;
};

#endif

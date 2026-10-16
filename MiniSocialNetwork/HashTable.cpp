#include "HashTable.h"
#include <functional>
#include <algorithm>

const float HashTable::LOAD_FACTOR_THRESHOLD = 0.75f;

HashTable::HashTable() : capacity(INITIAL_CAPACITY), size(0) {
    table = new HashNode*[capacity];
    for (int i = 0; i < capacity; ++i) {
        table[i] = nullptr;
    }
}

HashTable::~HashTable() {
    clear();
    delete[] table;
}

int HashTable::hashFunction(const std::string& key) const {
    // Simple polynomial hash
    unsigned long hash = 0;
    for (char c : key) {
        hash = hash * 31 + static_cast<unsigned char>(c);
    }
    return static_cast<int>(hash % capacity);
}

int HashTable::hashFunction(const std::string& key) {
    return static_cast<const HashTable*>(this)->hashFunction(key);
}

void HashTable::handleCollision(int index, const std::string& key, User* value) {
    HashNode* newNode = new HashNode(key, value);
    newNode->next = table[index];
    table[index] = newNode;
}

void HashTable::rehash() {
    int oldCapacity = capacity;
    HashNode** oldTable = table;
    capacity *= 2;
    table = new HashNode*[capacity];
    for (int i = 0; i < capacity; ++i) {
        table[i] = nullptr;
    }
    size = 0;

    for (int i = 0; i < oldCapacity; ++i) {
        HashNode* current = oldTable[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            insert(current->key, current->value);
            delete current;
            current = next;
        }
    }
    delete[] oldTable;
}

void HashTable::insert(const std::string& key, User* value) {
    if (contains(key)) {
        // Update existing
        int index = hashFunction(key);
        HashNode* current = table[index];
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }
    }

    if (getLoadFactor() >= LOAD_FACTOR_THRESHOLD) {
        rehash();
    }

    int index = hashFunction(key);
    handleCollision(index, key, value);
    ++size;
}

User* HashTable::search(const std::string& key) const {
    int index = hashFunction(key);
    HashNode* current = table[index];
    while (current != nullptr) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return nullptr;
}

bool HashTable::remove(const std::string& key) {
    int index = hashFunction(key);
    HashNode* current = table[index];
    HashNode* prev = nullptr;
    while (current != nullptr) {
        if (current->key == key) {
            if (prev == nullptr) {
                table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            --size;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool HashTable::contains(const std::string& key) const {
    return search(key) != nullptr;
}

float HashTable::getLoadFactor() const {
    return static_cast<float>(size) / capacity;
}

int HashTable::getSize() const {
    return size;
}

std::vector<User*> HashTable::getAllValues() const {
    std::vector<User*> result;
    for (int i = 0; i < capacity; ++i) {
        HashNode* current = table[i];
        while (current != nullptr) {
            result.push_back(current->value);
            current = current->next;
        }
    }
    return result;
}

void HashTable::clear() {
    for (int i = 0; i < capacity; ++i) {
        HashNode* current = table[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            delete current;
            current = next;
        }
        table[i] = nullptr;
    }
    size = 0;
}

void HashTable::addUserToIndex(User* user) {
    // Placeholder: main index is by username. Additional indexes can be extended.
    (void)user;
}

void HashTable::removeUserFromIndex(User* user) {
    (void)user;
}

void HashTable::updateUserIndex(User* user) {
    (void)user;
}

std::vector<User*> HashTable::searchByCity(const std::string& city) const {
    std::vector<User*> result;
    auto all = getAllValues();
    for (User* u : all) {
        if (u->getProfile().getCity() == city) {
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User*> HashTable::searchByHometown(const std::string& hometown) const {
    std::vector<User*> result;
    auto all = getAllValues();
    for (User* u : all) {
        if (u->getProfile().getHometown() == hometown) {
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User*> HashTable::searchBySchool(const std::string& school) const {
    std::vector<User*> result;
    auto all = getAllValues();
    for (User* u : all) {
        if (u->getProfile().getSchool() == school) {
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User*> HashTable::searchByWorkplace(const std::string& workplace) const {
    std::vector<User*> result;
    auto all = getAllValues();
    for (User* u : all) {
        if (u->getProfile().getWorkplace() == workplace) {
            result.push_back(u);
        }
    }
    return result;
}

std::vector<User*> HashTable::searchByInterest(const std::string& interest) const {
    std::vector<User*> result;
    auto all = getAllValues();
    for (User* u : all) {
        auto interests = u->getProfile().getInterests();
        if (std::find(interests.begin(), interests.end(), interest) != interests.end()) {
            result.push_back(u);
        }
    }
    return result;
}

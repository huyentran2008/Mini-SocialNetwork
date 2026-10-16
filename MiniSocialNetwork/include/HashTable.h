#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <iostream>

// Node cho chaining
template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    HashNode* next;

    HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
};

template <typename K, typename V>
class HashTable {
private:
    HashNode<K, V>** table;
    int capacity;
    int size;
    static const double MAX_LOAD_FACTOR;

    // Hash function cho int
    int hashFunction(int key) const {
        return (key % capacity + capacity) % capacity;
    }

    // Hash function cho string (ho tro tieng Viet UTF-8)
    int hashFunction(const std::string& key) const {
        unsigned long hash = 5381;
        for (size_t i = 0; i < key.length(); ++i) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(key[i]);
        }
        return static_cast<int>(hash % capacity);
    }

    // Xu ly dung do bang chaining
    void handleCollision(int index, HashNode<K, V>* newNode) {
        HashNode<K, V>* current = table[index];
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }

    void rehash() {
        int oldCapacity = capacity;
        HashNode<K, V>** oldTable = table;

        capacity *= 2;
        table = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; ++i) {
            table[i] = nullptr;
        }
        size = 0;

        for (int i = 0; i < oldCapacity; ++i) {
            HashNode<K, V>* current = oldTable[i];
            while (current != nullptr) {
                insert(current->key, current->value);
                HashNode<K, V>* temp = current;
                current = current->next;
                delete temp;
            }
        }
        delete[] oldTable;
    }

public:
    HashTable(int initialCapacity = 16) : capacity(initialCapacity), size(0) {
        table = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; ++i) {
            table[i] = nullptr;
        }
    }

    ~HashTable() {
        clear();
        delete[] table;
    }

    // Cam copy de tranh double free
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const K& key, const V& value) {
        if (getLoadFactor() >= MAX_LOAD_FACTOR) {
            rehash();
        }

        int index = hashFunction(key);
        HashNode<K, V>* current = table[index];

        // Neu key da ton tai -> cap nhat value
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Them moi
        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            handleCollision(index, newNode);
        }
        ++size;
    }

    V* search(const K& key) {
        int index = hashFunction(key);
        HashNode<K, V>* current = table[index];
        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }

    bool remove(const K& key) {
        int index = hashFunction(key);
        HashNode<K, V>* current = table[index];
        HashNode<K, V>* prev = nullptr;

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

    bool contains(const K& key) {
        return search(key) != nullptr;
    }

    double getLoadFactor() const {
        if (capacity == 0) return 0.0;
        return static_cast<double>(size) / capacity;
    }

    int getSize() const {
        return size;
    }

    std::vector<V> getAllValues() const {
        std::vector<V> result;
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                result.push_back(current->value);
                current = current->next;
            }
        }
        return result;
    }

    void clear() {
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                HashNode<K, V>* temp = current;
                current = current->next;
                delete temp;
            }
            table[i] = nullptr;
        }
        size = 0;
    }
};

template <typename K, typename V>
const double HashTable<K, V>::MAX_LOAD_FACTOR = 0.75;

#endif

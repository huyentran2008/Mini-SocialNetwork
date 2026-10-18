// TIM KIEM: theo ID, username, ten, thuoc tinh, loc nhieu dieu kien, ban chung - khai bao
#ifndef SEARCH_SERVICE_H
#define SEARCH_SERVICE_H

#include <string>
#include <vector>
#include "core.h"

struct FilterCriteria {
    std::string city, hometown, school, workplace, hobby;   // rong = bo qua
    bool empty() const;
};

class SearchService {
    const SocialNetwork& net;

    static bool userMatchesAttr(const User& u, Attr a, const std::string& qNorm);
public:
    explicit SearchService(const SocialNetwork& n) : net(n) {}

    // --- tim theo ID / username (HashTable) ---
    const User* searchById(int id) const { return net.getUser(id); }

    const User* searchByUsername(const std::string& username) const;

    // --- tim theo tu khoa trong ho ten (duyet tuyen tinh, khong phan biet hoa thuong) ---
    std::vector<int> searchByName(const std::vector<int>& base, const std::string& keyword) const;

    // --- tim theo MOT thuoc tinh (dung AttributeIndex), gioi han trong "base" neu co ---
    std::vector<int> searchByAttribute(Attr a, const std::string& value, const std::vector<int>* base = NULL) const;

    // --- loc nhieu dieu kien cung luc tren mot tap nguoi dung ---
    std::vector<int> filterUsers(const std::vector<int>& base, const FilterCriteria& c) const;

    // --- ban chung giua hai nguoi, co the loc theo ten ---
    std::vector<int> searchMutualFriends(int a, int b, const std::string& nameKeyword = "") const;

    // --- tap nguoi dung theo pham vi ---
    std::vector<int> friendsOf(int uid) const;
    std::vector<int> membersOf(int gid) const;

};

#endif // SEARCH_SERVICE_H

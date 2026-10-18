// CORE: chuoi, mo hinh, do thi, doc file, nhap lieu - khai bao
#ifndef CORE_H
#define CORE_H

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

// ---- Tien ich chuoi (ASCII) ----
namespace TextUtil {

inline bool isSpaceChar(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
std::string trim(const std::string& s);
// Gop nhieu khoang trang thanh mot, bo khoang trang dau/cuoi
std::string collapseSpaces(const std::string& s);
// Chuan hoa de so sanh: chu thuong + gop khoang trang. "Nguyen  VAN An" -> "nguyen van an"
std::string normalize(const std::string& s);
// 0 = hop le (chi chu cai a-z + khoang trang), 1 = co chu so, 2 = co ky tu khong hop le
int checkText(const std::string& s);
bool isAllDigits(const std::string& s);
// hay co chua needle khong (ca hai da chuan hoa). "ca hat" chua "hat" -> true
bool contains(const std::string& hayNorm, const std::string& needleNorm);
std::vector<std::string> split(const std::string& s, char sep);
std::string join(const std::vector<std::string>& v, const std::string& sep);
std::string lastWord(const std::string& norm);

} // namespace TextUtil

// ---- Mo hinh du lieu ----
enum Attr { A_CITY = 0, A_HOMETOWN, A_SCHOOL, A_WORKPLACE, A_HOBBY, A_COUNT };

const char* attrLabel(Attr a);

struct User {
    int id;
    std::string username;
    std::string name;
    std::string nameNorm;
    int age;
    std::string city, hometown, school, workplace;
    std::vector<std::string> hobbies;
    std::set<int> friends;

    User() : id(0), age(0) {}

    std::vector<std::string> values(Attr a) const;
};

struct Group {
    int id;
    std::string name;
    std::set<int> members;
    Group() : id(0) {}
};

// ---- Chi muc thuoc tinh ----
class AttributeIndex {
    std::unordered_map<std::string, std::vector<int> > idx[A_COUNT];
public:
    void addUser(const User& u);
    // Khop chinh xac (key da chuan hoa)
    std::vector<int> findExact(Attr a, const std::string& keyNorm) const;
    // Gia tri thuoc tinh CHUA chuoi can tim (da chuan hoa): "hat" khop "ca hat"
    std::vector<int> findContaining(Attr a, const std::string& qNorm) const;
};

// ---- Do thi ban be + HashTable + nhom ----
class SocialNetwork {
    std::unordered_map<int, User> users;            // HashTable theo ID
    std::unordered_map<std::string, int> byUsername;     // HashTable theo username
    std::vector<int> order;                         // thu tu them vao
    int nextId;                                // ID tu dong tang, khong bao gio trung
    AttributeIndex index;
    std::vector<Group> groups;
    int nextGroupId;
public:
    SocialNetwork() : nextId(1), nextGroupId(1) {}

    // ID luon la nextId++ => khong trung. Ho ten trung nhau van hop le,
    // username duoc them hau to ID neu bi trung (nguyen.van.an, nguyen.van.an26)
    int addUser(const std::string& name, int age, const std::string& city, const std::string& hometown,
                const std::string& school, const std::string& workplace, const std::vector<std::string>& hobbies);

    const User* getUser(int id) const;
    const User* findByUsername(const std::string& uname) const;
    bool exists(int id) const { return users.count(id) > 0; }
    const std::vector<int>& allIds() const { return order; }
    int userCount() const { return (int)order.size(); }
    const AttributeIndex& attributeIndex() const { return index; }

    bool areFriends(int a, int b) const;
    bool addFriend(int a, int b);
    std::vector<int> mutualFriends(int a, int b) const;

    // ---- Nhom ----
    int createGroup(const std::string& name);
    Group* getGroup(int gid);
    const Group* getGroup(int gid) const;
    bool joinGroup(int gid, int uid);
    const std::vector<Group>& allGroups() const { return groups; }
};

// ---- Doc du lieu tu file ----
class DataLoader {
    static bool okText(const std::string& s, bool allowEmpty);
    static void warn(int line, const std::string& msg);
public:
    static bool load(const std::string& path, SocialNetwork& net);
};

// ---- Nhap lieu an toan ----
std::string readLine(const std::string& prompt);
int readInt(const std::string& prompt, int lo, int hi);          // so nguyen khong am trong [lo, hi]
std::string readText(const std::string& prompt, bool allowEmpty = false);   // chu khong dau, khong chua so

#endif // CORE_H

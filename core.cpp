// CORE: chuoi, mo hinh, do thi, doc file, nhap lieu - cai dat
#include "core.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <set>
#include <cstdlib>
#include <cctype>

using namespace std;

// ---- Tien ich chuoi ----
namespace TextUtil {

string trim(const string& s) {
    size_t b = 0, e = s.size();
    while (b < e && isSpaceChar(s[b])) ++b;
    while (e > b && isSpaceChar(s[e - 1])) --e;
    return s.substr(b, e - b);
}

string collapseSpaces(const string& s) {
    string out;
    bool prevSpace = true;
    for (size_t i = 0; i < s.size(); ++i) {
        if (isSpaceChar(s[i])) { if (!prevSpace) out += ' '; prevSpace = true; }
        else { out += s[i]; prevSpace = false; }
    }
    if (!out.empty() && out[out.size() - 1] == ' ') out.erase(out.size() - 1);
    return out;
}

string normalize(const string& s) {
    string t;
    for (size_t i = 0; i < s.size(); ++i) t += (char)tolower((unsigned char)s[i]);
    return collapseSpaces(t);
}

int checkText(const string& s) {
    bool hasDigit = false, hasBad = false;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (isalpha(c) && c < 128) continue;
        if (c == ' ' || c == '\t') continue;
        if (c < 128 && isdigit(c)) hasDigit = true; else hasBad = true;
    }
    if (hasDigit) return 1;
    if (hasBad) return 2;
    return 0;
}

bool isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

bool contains(const string& hayNorm, const string& needleNorm) {
    return hayNorm.find(needleNorm) != string::npos;
}

vector<string> split(const string& s, char sep) {
    vector<string> out;
    string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == sep) { out.push_back(cur); cur.clear(); }
        else cur += s[i];
    }
    out.push_back(cur);
    return out;
}

string join(const vector<string>& v, const string& sep) {
    string out;
    for (size_t i = 0; i < v.size(); ++i) { if (i) out += sep; out += v[i]; }
    return out;
}

string lastWord(const string& norm) {
    size_t p = norm.find_last_of(' ');
    return p == string::npos ? norm : norm.substr(p + 1);
}

} // namespace TextUtil

// ---- Mo hinh du lieu ----
const char* attrLabel(Attr a) {
    switch (a) {
        case A_CITY:      return "Noi o";
        case A_HOMETOWN:  return "Que quan";
        case A_SCHOOL:    return "Truong";
        case A_WORKPLACE: return "Noi lam viec";
        case A_HOBBY:     return "So thich";
        default:          return "";
    }
}

vector<string> User::values(Attr a) const {
    vector<string> v;
    switch (a) {
        case A_CITY:      if (!city.empty()) v.push_back(city); break;
        case A_HOMETOWN:  if (!hometown.empty()) v.push_back(hometown); break;
        case A_SCHOOL:    if (!school.empty()) v.push_back(school); break;
        case A_WORKPLACE: if (!workplace.empty()) v.push_back(workplace); break;
        case A_HOBBY:     v = hobbies; break;
        default: break;
    }
    return v;
}

// ---- Chi muc thuoc tinh ----
void AttributeIndex::addUser(const User& u) {
    for (int a = 0; a < A_COUNT; ++a) {
        vector<string> vals = u.values((Attr)a);
        for (size_t i = 0; i < vals.size(); ++i) {
            string key = TextUtil::normalize(vals[i]);
            if (key.empty()) continue;
            vector<int>& vec = idx[a][key];
            if (find(vec.begin(), vec.end(), u.id) == vec.end()) vec.push_back(u.id);
        }
    }
}

vector<int> AttributeIndex::findExact(Attr a, const string& keyNorm) const {
    unordered_map<string, vector<int> >::const_iterator it = idx[a].find(keyNorm);
    if (it == idx[a].end()) return vector<int>();
    return it->second;
}

vector<int> AttributeIndex::findContaining(Attr a, const string& qNorm) const {
    set<int> res;
    if (qNorm.empty()) return vector<int>();
    for (unordered_map<string, vector<int> >::const_iterator it = idx[a].begin(); it != idx[a].end(); ++it) {
        if (TextUtil::contains(it->first, qNorm)) res.insert(it->second.begin(), it->second.end());
    }
    return vector<int>(res.begin(), res.end());
}

// ---- Do thi ban be ----
int SocialNetwork::addUser(const string& name, int age, const string& city, const string& hometown,
            const string& school, const string& workplace, const vector<string>& hobbies) {
    User u;
    u.id = nextId++;
    u.name = name;
    u.nameNorm = TextUtil::normalize(name);
    u.age = age;
    u.city = city; u.hometown = hometown; u.school = school; u.workplace = workplace;
    u.hobbies = hobbies;
    string base = u.nameNorm;
    for (size_t i = 0; i < base.size(); ++i) if (base[i] == ' ') base[i] = '.';
    if (base.empty()) base = "user";
    string uname = base;
    if (byUsername.count(uname)) { ostringstream o; o << base << u.id; uname = o.str(); }
    u.username = uname;
    users[u.id] = u;
    byUsername[uname] = u.id;
    order.push_back(u.id);
    index.addUser(users[u.id]);
    return u.id;
}

const User* SocialNetwork::getUser(int id) const {
    unordered_map<int, User>::const_iterator it = users.find(id);
    return it == users.end() ? NULL : &it->second;
}

const User* SocialNetwork::findByUsername(const string& uname) const {
    unordered_map<string, int>::const_iterator it = byUsername.find(uname);
    return it == byUsername.end() ? NULL : getUser(it->second);
}

bool SocialNetwork::areFriends(int a, int b) const {
    const User* u = getUser(a);
    return u && u->friends.count(b) > 0;
}

bool SocialNetwork::addFriend(int a, int b) {
    if (a == b || !exists(a) || !exists(b) || areFriends(a, b)) return false;
    users[a].friends.insert(b);
    users[b].friends.insert(a);
    return true;
}

vector<int> SocialNetwork::mutualFriends(int a, int b) const {
    vector<int> res;
    const User* ua = getUser(a);
    const User* ub = getUser(b);
    if (!ua || !ub) return res;
    set_intersection(ua->friends.begin(), ua->friends.end(),
                     ub->friends.begin(), ub->friends.end(), back_inserter(res));
    return res;
}

int SocialNetwork::createGroup(const string& name) {
    Group g; g.id = nextGroupId++; g.name = name;
    groups.push_back(g);
    return g.id;
}

Group* SocialNetwork::getGroup(int gid) {
    for (size_t i = 0; i < groups.size(); ++i) if (groups[i].id == gid) return &groups[i];
    return NULL;
}

const Group* SocialNetwork::getGroup(int gid) const {
    for (size_t i = 0; i < groups.size(); ++i) if (groups[i].id == gid) return &groups[i];
    return NULL;
}

bool SocialNetwork::joinGroup(int gid, int uid) {
    Group* g = getGroup(gid);
    if (!g || !exists(uid)) return false;
    return g->members.insert(uid).second;
}

// ---- Doc du lieu tu file ----
bool DataLoader::okText(const string& s, bool allowEmpty) {
    if (s.empty()) return allowEmpty;
    return TextUtil::checkText(s) == 0;
}

void DataLoader::warn(int line, const string& msg) {
    cout << "  [Canh bao] dong " << line << ": " << msg << " (bo qua)\n";
}

bool DataLoader::load(const string& path, SocialNetwork& net) {
    ifstream f(path.c_str());
    if (!f) { cout << "  ! Khong mo duoc file: " << path << "\n"; return false; }
    string line;
    int ln = 0, bad = 0, edges = 0, groups = 0;
    while (getline(f, line)) {
        ++ln;
        line = TextUtil::trim(line);
        if (line.empty() || line[0] == '#') continue;
        vector<string> p = TextUtil::split(line, '|');
        for (size_t i = 0; i < p.size(); ++i) p[i] = TextUtil::collapseSpaces(p[i]);
        string type = TextUtil::normalize(p[0]);

        if (type == "user") {
            if (p.size() != 8) { warn(ln, "USER can du 8 truong"); ++bad; continue; }
            if (!okText(p[1], false) || !okText(p[3], false) || !okText(p[4], false) ||
                !okText(p[5], true) || !okText(p[6], true)) {
                warn(ln, "ho ten / noi o / que quan / truong / noi lam viec chi duoc chua chu cai khong dau");
                ++bad; continue;
            }
            if (!TextUtil::isAllDigits(p[2]) || p[2].size() > 3 || atoi(p[2].c_str()) < 1 || atoi(p[2].c_str()) > 120) {
                warn(ln, "tuoi phai la so nguyen 1-120"); ++bad; continue;
            }
            vector<string> hb, raw = TextUtil::split(p[7], ',');
            bool ok = true;
            for (size_t i = 0; i < raw.size(); ++i) {
                string h = TextUtil::collapseSpaces(raw[i]);
                if (h.empty()) continue;
                if (TextUtil::checkText(h) != 0) { ok = false; break; }
                hb.push_back(h);
            }
            if (!ok) { warn(ln, "so thich chi duoc chua chu cai khong dau"); ++bad; continue; }
            net.addUser(p[1], atoi(p[2].c_str()), p[3], p[4], p[5], p[6], hb);
        } else if (type == "friend") {
            if (p.size() != 3 || !TextUtil::isAllDigits(p[1]) || !TextUtil::isAllDigits(p[2]) ||
                p[1].size() > 9 || p[2].size() > 9) {
                warn(ln, "FRIEND can dang FRIEND|id1|id2"); ++bad; continue;
            }
            int a = atoi(p[1].c_str()), b = atoi(p[2].c_str());
            if (a == b || !net.exists(a) || !net.exists(b)) { warn(ln, "ID khong ton tai hoac trung nhau"); ++bad; continue; }
            if (net.addFriend(a, b)) ++edges; else { warn(ln, "hai nguoi da la ban"); ++bad; }
        } else if (type == "group") {
            if (p.size() != 3 || !okText(p[1], false)) { warn(ln, "GROUP can dang GROUP|ten nhom|id1,id2"); ++bad; continue; }
            vector<string> ids = TextUtil::split(p[2], ',');
            vector<int> members;
            bool ok = true;
            for (size_t i = 0; i < ids.size(); ++i) {
                string t = TextUtil::trim(ids[i]);
                if (t.empty()) continue;
                if (!TextUtil::isAllDigits(t) || t.size() > 9 || !net.exists(atoi(t.c_str()))) { ok = false; break; }
                members.push_back(atoi(t.c_str()));
            }
            if (!ok) { warn(ln, "danh sach thanh vien co ID khong hop le"); ++bad; continue; }
            int g = net.createGroup(p[1]);
            for (size_t i = 0; i < members.size(); ++i) net.joinGroup(g, members[i]);
            ++groups;
        } else {
            warn(ln, "loai dong khong hop le (chi USER / FRIEND / GROUP)"); ++bad;
        }
    }
    if (net.userCount() == 0) { cout << "  ! File khong co nguoi dung nao hop le.\n"; return false; }
    cout << "Da nap " << net.userCount() << " nguoi dung, " << edges << " quan he ban be, "
         << groups << " nhom tu file \"" << path << "\"";
    if (bad) cout << " (" << bad << " dong loi)";
    cout << ".\n";
    return true;
}

// ---- Nhap lieu an toan ----
string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nDa het du lieu nhap. Thoat chuong trinh.\n";
        exit(0);
    }
    return TextUtil::trim(line);
}

// So nguyen khong am trong [lo, hi]
int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        if (s.empty()) { cout << "  ! Khong duoc de trong.\n"; continue; }
        if (s[0] == '-' && TextUtil::isAllDigits(s.substr(1))) { cout << "  ! Khong duoc nhap so am.\n"; continue; }
        if (!TextUtil::isAllDigits(s)) { cout << "  ! Chi duoc nhap so nguyen khong am.\n"; continue; }
        if (s.size() > 9) { cout << "  ! So qua lon.\n"; continue; }
        int v = atoi(s.c_str());
        if (v < lo || v > hi) { cout << "  ! Gia tri phai trong khoang " << lo << " - " << hi << ".\n"; continue; }
        return v;
    }
}

// Chu khong dau (hoa/thuong deu duoc), KHONG duoc chua so
string readText(const string& prompt, bool allowEmpty) {
    while (true) {
        string s = TextUtil::collapseSpaces(readLine(prompt));
        if (s.empty()) {
            if (allowEmpty) return "";
            cout << "  ! Khong duoc de trong.\n";
            continue;
        }
        int c = TextUtil::checkText(s);
        if (c == 1) { cout << "  ! Khong duoc nhap so trong truong chu.\n"; continue; }
        if (c == 2) { cout << "  ! Chi nhap chu cai khong dau (a-z) va khoang trang.\n"; continue; }
        return s;
    }
}

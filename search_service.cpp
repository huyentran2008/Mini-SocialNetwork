// TIM KIEM: theo ID, username, ten, thuoc tinh, loc nhieu dieu kien, ban chung - cai dat
#include "search_service.h"
#include <set>
#include <algorithm>

using namespace std;

bool FilterCriteria::empty() const {
    return city.empty() && hometown.empty() && school.empty() && workplace.empty() && hobby.empty();
}

bool SearchService::userMatchesAttr(const User& u, Attr a, const string& qNorm) {
    vector<string> vals = u.values(a);
    for (size_t i = 0; i < vals.size(); ++i)
        if (TextUtil::contains(TextUtil::normalize(vals[i]), qNorm)) return true;
    return false;
}

const User* SearchService::searchByUsername(const string& username) const {
    string key = TextUtil::normalize(username);
    if (!key.empty() && key[0] == '@') key.erase(0, 1);
    return net.findByUsername(key);
}

vector<int> SearchService::searchByName(const vector<int>& base, const string& keyword) const {
    vector<int> res;
    string q = TextUtil::normalize(keyword);
    for (size_t i = 0; i < base.size(); ++i) {
        const User* u = net.getUser(base[i]);
        if (u && TextUtil::contains(u->nameNorm, q)) res.push_back(base[i]);
    }
    return res;
}

vector<int> SearchService::searchByAttribute(Attr a, const string& value, const vector<int>* base) const {
    vector<int> found = net.attributeIndex().findContaining(a, TextUtil::normalize(value));
    if (!base) return found;
    set<int> allowed(base->begin(), base->end());
    vector<int> res;
    for (size_t i = 0; i < found.size(); ++i) if (allowed.count(found[i])) res.push_back(found[i]);
    return res;
}

vector<int> SearchService::filterUsers(const vector<int>& base, const FilterCriteria& c) const {
    vector<int> res;
    string qc = TextUtil::normalize(c.city), qh = TextUtil::normalize(c.hometown);
    string qs = TextUtil::normalize(c.school), qw = TextUtil::normalize(c.workplace);
    string qb = TextUtil::normalize(c.hobby);
    for (size_t i = 0; i < base.size(); ++i) {
        const User* u = net.getUser(base[i]);
        if (!u) continue;
        if (!qc.empty() && !userMatchesAttr(*u, A_CITY, qc)) continue;
        if (!qh.empty() && !userMatchesAttr(*u, A_HOMETOWN, qh)) continue;
        if (!qs.empty() && !userMatchesAttr(*u, A_SCHOOL, qs)) continue;
        if (!qw.empty() && !userMatchesAttr(*u, A_WORKPLACE, qw)) continue;
        if (!qb.empty() && !userMatchesAttr(*u, A_HOBBY, qb)) continue;
        res.push_back(base[i]);
    }
    return res;
}

vector<int> SearchService::searchMutualFriends(int a, int b, const string& nameKeyword) const {
    vector<int> mutual = net.mutualFriends(a, b);
    if (nameKeyword.empty()) return mutual;
    return searchByName(mutual, nameKeyword);
}

vector<int> SearchService::friendsOf(int uid) const {
    const User* u = net.getUser(uid);
    if (!u) return vector<int>();
    return vector<int>(u->friends.begin(), u->friends.end());
}

vector<int> SearchService::membersOf(int gid) const {
    const Group* g = net.getGroup(gid);
    if (!g) return vector<int>();
    return vector<int>(g->members.begin(), g->members.end());
}

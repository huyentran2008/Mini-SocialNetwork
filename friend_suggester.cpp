// GOI Y: ket ban theo ban chung va thuoc tinh - cai dat
#include "friend_suggester.h"
#include <set>
#include <sstream>
#include "sort_service.h"

using namespace std;

bool FriendSuggester::sameValue(const string& a, const string& b) {
    string na = TextUtil::normalize(a), nb = TextUtil::normalize(b);
    return !na.empty() && na == nb;
}

vector<string> FriendSuggester::commonHobbies(const User& a, const User& b) const {
    set<string> hb;
    for (size_t i = 0; i < b.hobbies.size(); ++i) hb.insert(TextUtil::normalize(b.hobbies[i]));
    vector<string> res;
    for (size_t i = 0; i < a.hobbies.size(); ++i)
        if (hb.count(TextUtil::normalize(a.hobbies[i]))) res.push_back(a.hobbies[i]);
    return res;
}

vector<string> FriendSuggester::matchedValues(const User& u, Attr a, const vector<string>& keys) {
    vector<string> res;
    vector<string> vals = u.values(a);
    for (size_t i = 0; i < vals.size(); ++i) {
        string nv = TextUtil::normalize(vals[i]);
        for (size_t k = 0; k < keys.size(); ++k)
            if (TextUtil::contains(nv, keys[k])) { res.push_back(vals[i]); break; }
    }
    return res;
}

vector<Suggestion> FriendSuggester::buildSuggestions(int uid, const vector<int>& ids, int attr,
                                    const vector<string>& keys) const {
    vector<Suggestion> res;
    for (size_t i = 0; i < ids.size(); ++i) {
        int c = ids[i];
        if (c == uid || !net.exists(c) || net.areFriends(uid, c)) continue;
        Suggestion s;
        s.userId = c;
        s.mutual = countMutualFriends(uid, c);
        s.score = calculateScore(uid, c);
        s.reasons = getReasons(uid, c);
        if (attr >= 0) {
            vector<string> m = matchedValues(*net.getUser(c), (Attr)attr, keys);
            if (!m.empty()) {
                s.score += 5;
                // Chi ghi them dong "khop tu khoa" khi gia tri khop KHAC gia tri cua chinh minh
                // (neu giong thi da co dong "Cung ..." / "Chung so thich" roi, tranh lap)
                vector<string> own = net.getUser(uid)->values((Attr)attr);
                vector<string> extra;
                for (size_t x = 0; x < m.size(); ++x) {
                    bool covered = false;
                    for (size_t y = 0; y < own.size(); ++y) if (sameValue(m[x], own[y])) covered = true;
                    if (!covered) extra.push_back(m[x]);
                }
                if (!extra.empty())
                    s.reasons.insert(s.reasons.begin(),
                                     string(attrLabel((Attr)attr)) + " khop tu khoa: " + TextUtil::join(extra, ", "));
            }
        }
        if (s.reasons.empty()) s.reasons.push_back("Nguoi dung moi");
        res.push_back(s);
    }
    rankSuggestions(res);
    return res;
}

void FriendSuggester::rankSuggestions(vector<Suggestion>& v) const {
    mergeSort(v, [](const Suggestion& a, const Suggestion& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.userId < b.userId;
    });
}

vector<Suggestion> FriendSuggester::suggestFriends(int uid) const {
    vector<Suggestion> res;
    vector<int> cands = collectCandidatesByMutual(uid);
    for (size_t i = 0; i < cands.size(); ++i) {
        Suggestion s;
        s.userId = cands[i];
        s.mutual = countMutualFriends(uid, cands[i]);
        s.score = s.mutual;
        ostringstream o; o << s.mutual << " ban chung";
        s.reasons.push_back(o.str());
        res.push_back(s);
    }
    rankSuggestions(res);
    return res;
}

vector<int> FriendSuggester::collectCandidatesByAttribute(int uid) const {
    set<int> res;
    const User* u = net.getUser(uid);
    if (!u) return vector<int>();
    for (int a = 0; a < A_COUNT; ++a) {
        vector<string> vals = u->values((Attr)a);
        for (size_t i = 0; i < vals.size(); ++i) {
            vector<int> ids = net.attributeIndex().findExact((Attr)a, TextUtil::normalize(vals[i]));
            res.insert(ids.begin(), ids.end());
        }
    }
    return vector<int>(res.begin(), res.end());
}

vector<int> FriendSuggester::mergeCandidates(const vector<int>& byMutual, const vector<int>& byAttr, int uid) const {
    set<int> s(byMutual.begin(), byMutual.end());
    s.insert(byAttr.begin(), byAttr.end());
    s.erase(uid);
    vector<int> res;
    for (set<int>::const_iterator it = s.begin(); it != s.end(); ++it)
        if (!net.areFriends(uid, *it)) res.push_back(*it);
    return res;
}

int FriendSuggester::calculateScore(int uid, int cand) const {
    const User* a = net.getUser(uid);
    const User* b = net.getUser(cand);
    if (!a || !b) return 0;
    int score = countMutualFriends(uid, cand) * 10;
    if (sameValue(a->school, b->school)) score += 4;
    if (sameValue(a->workplace, b->workplace)) score += 4;
    if (sameValue(a->city, b->city)) score += 3;
    if (sameValue(a->hometown, b->hometown)) score += 3;
    score += (int)commonHobbies(*a, *b).size() * 2;
    return score;
}

vector<string> FriendSuggester::getReasons(int uid, int cand) const {
    vector<string> parts;
    const User* a = net.getUser(uid);
    const User* b = net.getUser(cand);
    if (!a || !b) return parts;
    int m = countMutualFriends(uid, cand);
    if (m > 0) { ostringstream o; o << m << " ban chung"; parts.push_back(o.str()); }
    if (sameValue(a->school, b->school)) parts.push_back("Cung truong: " + b->school);
    if (sameValue(a->workplace, b->workplace)) parts.push_back("Cung noi lam viec: " + b->workplace);
    if (sameValue(a->city, b->city)) parts.push_back("Cung noi o: " + b->city);
    if (sameValue(a->hometown, b->hometown)) parts.push_back("Cung que quan: " + b->hometown);
    vector<string> hb = commonHobbies(*a, *b);
    if (!hb.empty()) parts.push_back("Chung so thich: " + TextUtil::join(hb, ", "));
    return parts;
}

vector<Suggestion> FriendSuggester::suggestAll(int uid) const {
    vector<int> cands = mergeCandidates(collectCandidatesByMutual(uid), collectCandidatesByAttribute(uid), uid);
    return buildSuggestions(uid, cands);
}

vector<Suggestion> FriendSuggester::suggestByAttribute(int uid, Attr a, const string& keyword) const {
    const User* u = net.getUser(uid);
    if (!u) return vector<Suggestion>();
    vector<string> keys;
    string k = TextUtil::normalize(keyword);
    if (!k.empty()) keys.push_back(k);
    else {
        vector<string> vals = u->values(a);
        for (size_t i = 0; i < vals.size(); ++i) {
            string nk = TextUtil::normalize(vals[i]);
            if (!nk.empty()) keys.push_back(nk);
        }
    }
    set<int> s;
    for (size_t i = 0; i < keys.size(); ++i) {
        vector<int> ids = net.attributeIndex().findContaining(a, keys[i]);
        s.insert(ids.begin(), ids.end());
    }
    return buildSuggestions(uid, vector<int>(s.begin(), s.end()), (int)a, keys);
}

vector<Suggestion> FriendSuggester::suggestInGroup(int uid, int gid) const {
    const Group* g = net.getGroup(gid);
    if (!g) return vector<Suggestion>();
    return buildSuggestions(uid, vector<int>(g->members.begin(), g->members.end()));
}

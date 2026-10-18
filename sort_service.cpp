// SORT: merge sort tu cai + sap xep nguoi dung - cai dat
#include "sort_service.h"
#include <string>

using namespace std;

vector<int> SortService::sortUsersByName(vector<int> ids) const {
    const SocialNetwork& n = net;
    mergeSort(ids, [&n](const int& a, const int& b) {
        const User* ua = n.getUser(a);
        const User* ub = n.getUser(b);
        string ka = TextUtil::lastWord(ua->nameNorm) + " " + ua->nameNorm;
        string kb = TextUtil::lastWord(ub->nameNorm) + " " + ub->nameNorm;
        if (ka != kb) return ka < kb;
        return a < b;
    });
    return ids;
}

vector<int> SortService::sortUsersByFriendCount(vector<int> ids) const {
    const SocialNetwork& n = net;
    mergeSort(ids, [&n](const int& a, const int& b) {
        size_t fa = n.getUser(a)->friends.size();
        size_t fb = n.getUser(b)->friends.size();
        if (fa != fb) return fa > fb;
        return a < b;
    });
    return ids;
}

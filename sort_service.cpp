#include "sort_service.h"

// Sáº¯p xáº¿p gá»£i Ã½ theo Ä‘á»“ thá»‹ báº¡n bÃ¨: khoáº£ng cÃ¡ch tÄƒng dáº§n, báº¡n chung giáº£m dáº§n, tÃªn A-Z
void sortSuggestionsByGraph(vector<Suggestion>& list) {
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });
}

// Sáº¯p xáº¿p gá»£i Ã½ theo nhÃ³m chat chung: sá»‘ nhÃ³m giáº£m dáº§n, báº¡n chung giáº£m dáº§n, tÃªn A-Z
void sortSuggestionsByCommonGroup(vector<Suggestion>& list) {
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.commonGroups != b.commonGroups) return a.commonGroups > b.commonGroups;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });
}

// Sáº¯p xáº¿p ngÆ°á»i dÃ¹ng ná»•i báº­t: sá»‘ báº¡n bÃ¨ giáº£m dáº§n, tÃªn A-Z, ID tÄƒng dáº§n
void sortSuggestionsByPopularity(vector<Suggestion>& list) {
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.friendCount != b.friendCount) return a.friendCount > b.friendCount;
        if (a.name != b.name) return a.name < b.name;
        return a.id < b.id;
    });
}

// Sáº¯p xáº¿p gá»£i Ã½ theo há»“ sÆ¡ (quÃª quÃ¡n, nÆ¡i á»Ÿ, trÆ°á»ng, nÆ¡i lÃ m viá»‡c, sá»Ÿ thÃ­ch): Ä‘iá»ƒm giáº£m dáº§n, báº¡n chung giáº£m dáº§n, tÃªn A-Z
void sortSuggestionsByProfile(vector<Suggestion>& list) {
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });
}

// Sáº¯p xáº¿p káº¿t quáº£ tÃ¬m kiáº¿m: tÃªn báº¯t Ä‘áº§u báº±ng tá»« khÃ³a lÃªn trÆ°á»›c, sau Ä‘Ã³ theo thá»© tá»± chá»¯ cÃ¡i
void sortUsersByNameMatch(vector<UserProfile>& matched, const string& key) {
    sort(matched.begin(), matched.end(), [&](const UserProfile& a, const UserProfile& b) {
        string na = foldText(a.fullName.empty() ? a.username : a.fullName);
        string nb = foldText(b.fullName.empty() ? b.username : b.fullName);
        bool sa = na.compare(0, key.size(), key) == 0;
        bool sb = nb.compare(0, key.size(), key) == 0;

        if (sa != sb) return sa;
        return na < nb;
    });
}

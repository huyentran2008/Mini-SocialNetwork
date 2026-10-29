#include "friend_suggester.h"
#include "bfs_algorithm.h"
#include "sort_service.h"

vector<Suggestion> suggestByCommonGroup(const string& myId) {
    vector<Suggestion> list;
    vector<Group> mine = getGroupsOf(myId);
    map<string, int> shared;
    for (const Group& group : mine) {
        for (const GroupMember& member : group.members) {
            if (member.id != myId) shared[member.id]++;
        }
    }
    for (const auto& item : shared) {
        UserProfile profile;
        if (!canSuggestTo(myId, item.first, profile)) continue;
        Suggestion suggestion;
        suggestion.id = item.first;
        suggestion.name = profile.fullName.empty() ? profile.username : profile.fullName;
        suggestion.mutual = countMutualFriends(myId, item.first);
        suggestion.friendCount = countFriends(item.first);
        suggestion.commonGroups = item.second;
        suggestion.reason = "Chung " + to_string(item.second) + " nhÃ³m chat";
        list.push_back(suggestion);
    }
    sortSuggestionsByCommonGroup(list);
    if ((int)list.size() > SUGGEST_MAX_RESULT) list.resize(SUGGEST_MAX_RESULT);
    return list;
}

vector<Suggestion> suggestPopularUsers(const string& myId) {
    vector<Suggestion> list;

    for (const string& id : listAccountIds()) {
        UserProfile profile;

        if (!canSuggestTo(myId, id, profile)) {
            continue;
        }

        Suggestion suggestion;
        suggestion.id = id;
        suggestion.name = profile.fullName.empty() ? profile.username : profile.fullName;
        suggestion.distance = 0;
        suggestion.mutual = countMutualFriends(myId, id);
        suggestion.friendCount = countFriends(id);
        list.push_back(suggestion);
    }

    sortSuggestionsByPopularity(list);

    if ((int)list.size() > SUGGEST_MAX_FALLBACK) {
        list.resize(SUGGEST_MAX_FALLBACK);
    }

    return list;
}

vector<Suggestion> computeSuggestions(const string& myId) {
    vector<Suggestion> list = suggestByFriendGraph(myId);

    if (list.empty()) {
        list = suggestPopularUsers(myId);
    }

    return list;
}

string suggestionLabel(const Suggestion& suggestion) {
    string text = suggestion.name;

    if (!suggestion.reason.empty()) {
        text += " (" + to_string(suggestion.mutual) + " Báº¡n Chung) - " + suggestion.reason;
    }
    else if (suggestion.distance == 2) {
        text += " (" + to_string(suggestion.mutual) + " Báº¡n Chung) - Báº¡n cá»§a báº¡n bÃ¨";
    }
    else if (suggestion.distance == 3) {
        text += " (" + to_string(suggestion.mutual) + " Báº¡n Chung) - Báº¡n cá»§a báº¡n cá»§a báº¡n bÃ¨";
    }
    else {
        text += " (" + to_string(suggestion.friendCount) + " Báº¡n BÃ¨) - NgÆ°á»i dÃ¹ng ná»•i báº­t";
    }

    return text;
}

string normalizeProfileText(const string& text) {
    return foldText(trimString(text));
}

vector<Suggestion> suggestByProfile(const string& myId, SuggestKind kind) {
    vector<Suggestion> list;
    UserProfile me;

    if (!findProfileById(myId, me)) {
        return list;
    }

    vector<string> myHobbies = splitList(me.hobbies);

    for (const string& id : listAccountIds()) {
        UserProfile profile;

        if (!canSuggestTo(myId, id, profile)) {
            continue;
        }

        Suggestion suggestion;
        suggestion.id = id;
        suggestion.name = profile.fullName.empty() ? profile.username : profile.fullName;
        suggestion.mutual = countMutualFriends(myId, id);
        suggestion.friendCount = countFriends(id);

        switch (kind) {
            case SUGGEST_HOMETOWN:
                if (me.hometown.empty() || profile.hometown != me.hometown) continue;
                suggestion.reason = "CÃ¹ng quÃª quÃ¡n: " + profile.hometown;
                break;

            case SUGGEST_RESIDENCE:
                if (me.residence.empty() || profile.residence != me.residence) continue;
                suggestion.reason = "CÃ¹ng nÆ¡i á»Ÿ: " + profile.residence;
                break;

            case SUGGEST_SCHOOL:
                if (me.school.empty() || profile.school != me.school) continue;
                suggestion.reason = "CÃ¹ng trÆ°á»ng: " + profile.school;
                break;

            case SUGGEST_WORKPLACE:
                if (normalizeProfileText(me.workplace).empty() ||
                    normalizeProfileText(profile.workplace) != normalizeProfileText(me.workplace)) continue;
                suggestion.reason = "CÃ¹ng nÆ¡i lÃ m viá»‡c: " + profile.workplace;
                break;

            case SUGGEST_HOBBY: {
                vector<string> common;

                for (const string& hobby : splitList(profile.hobbies)) {
                    if (find(myHobbies.begin(), myHobbies.end(), hobby) != myHobbies.end()) {
                        common.push_back(hobby);
                    }
                }

                if (common.empty()) continue;
                suggestion.score = (int)common.size();
                suggestion.reason = "Chung sá»Ÿ thÃ­ch: " + joinList(common, ", ");
                break;
            }
        }

        list.push_back(suggestion);
    }

    sortSuggestionsByProfile(list);

    return list;
}

string suggestionExtra(const Suggestion& suggestion) {
    string extra;

    if (suggestion.mutual > 0) {
        extra = to_string(suggestion.mutual) + " báº¡n chung";
    }

    string reason = suggestion.reason;

    if (reason.empty()) {
        reason = (suggestion.distance == 2) ? "Báº¡n cá»§a báº¡n bÃ¨"
               : (suggestion.distance == 3) ? "Báº¡n cá»§a báº¡n cá»§a báº¡n bÃ¨"
                                            : "NgÆ°á»i dÃ¹ng ná»•i báº­t";
    }

    return extra.empty() ? reason : extra + " Â· " + reason;
}

void suggestionListScreen(
    const string& myId,
    const string& title,
    const function<vector<Suggestion>()>& source,
    const string& emptyMessage
) {
    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = "Xem trang cÃ¡ nhÃ¢n";
    config.noun = "ngÆ°á»i";
    config.moreWord = "gá»£i Ã½ ná»¯a";
    config.pageSize = 5;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const Suggestion& suggestion : source()) {
                CardItem item = makePersonItem(myId, suggestion.id, false, suggestionExtra(suggestion));

                if (!item.key.empty()) {
                    items.push_back(item);
                }
            }

            return items;
        },
        [&](const string& id) {
            viewUserProfile(myId, id);
        }
    );
}

void suggestionScreen(const string& myId) {
    vector<string> items = {
        "Báº¡n Cá»§a Báº¡n BÃ¨",
        "Theo QuÃª QuÃ¡n",
        "Theo NÆ¡i á»ž",
        "Theo TrÆ°á»ng Há»c",
        "Theo NÆ¡i LÃ m Viá»‡c",
        "Theo Sá»Ÿ ThÃ­ch",
        "Chung NhÃ³m Chat",
        "Quay Láº¡i"
    };

    const string note = "HÃ£y cáº­p nháº­t má»¥c nÃ y trong Trang CÃ¡ NhÃ¢n > Chá»‰nh Sá»­a TÃ i Khoáº£n Ä‘á»ƒ nháº­n gá»£i Ã½.";

    while (true) {
        UserProfile me;

        if (!findProfileById(myId, me)) {
            return;
        }

        int choice = menu(
            "=============== Gá»¢I Ã Káº¾T Báº N ===============",
            items,
            "Sá»­ dá»¥ng â–² / â–¼ | ENTER: Chá»n | ESC: Quay láº¡i"
        );

        switch (choice) {
            case 1:
                suggestionListScreen(myId, "=============== Gá»¢I Ã: Báº N Cá»¦A Báº N BÃˆ ===============",
                    [&]() { return computeSuggestions(myId); },
                    "Hiá»‡n chÆ°a cÃ³ gá»£i Ã½ káº¿t báº¡n nÃ o cho báº¡n.");
                break;

            case 2:
                if (me.hometown.empty()) { notify("Báº¡n chÆ°a chá»n quÃª quÃ¡n. " + note); break; }
                suggestionListScreen(myId, "=============== Gá»¢I Ã: THEO QUÃŠ QUÃN ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_HOMETOWN); },
                    "ChÆ°a cÃ³ ai cÃ¹ng quÃª quÃ¡n (" + me.hometown + ") Ä‘á»ƒ gá»£i Ã½.");
                break;

            case 3:
                if (me.residence.empty()) { notify("Báº¡n chÆ°a chá»n nÆ¡i á»Ÿ hiá»‡n táº¡i. " + note); break; }
                suggestionListScreen(myId, "=============== Gá»¢I Ã: THEO NÆ I á»ž ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_RESIDENCE); },
                    "ChÆ°a cÃ³ ai cÃ¹ng nÆ¡i á»Ÿ (" + me.residence + ") Ä‘á»ƒ gá»£i Ã½.");
                break;

            case 4:
                if (me.school.empty()) { notify("Báº¡n chÆ°a chá»n trÆ°á»ng há»c. " + note); break; }
                suggestionListScreen(myId, "=============== Gá»¢I Ã: THEO TRÆ¯á»œNG Há»ŒC ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_SCHOOL); },
                    "ChÆ°a cÃ³ ai cÃ¹ng trÆ°á»ng (" + me.school + ") Ä‘á»ƒ gá»£i Ã½.");
                break;

            case 5:
                if (normalizeProfileText(me.workplace).empty()) { notify("Báº¡n chÆ°a nháº­p nÆ¡i lÃ m viá»‡c. " + note); break; }
                suggestionListScreen(myId, "=============== Gá»¢I Ã: THEO NÆ I LÃ€M VIá»†C ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_WORKPLACE); },
                    "ChÆ°a cÃ³ ai cÃ¹ng nÆ¡i lÃ m viá»‡c (" + me.workplace + ") Ä‘á»ƒ gá»£i Ã½.");
                break;

            case 6:
                if (splitList(me.hobbies).empty()) { notify("Báº¡n chÆ°a chá»n sá»Ÿ thÃ­ch nÃ o. " + note); break; }
                suggestionListScreen(myId, "=============== Gá»¢I Ã: THEO Sá»ž THÃCH ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_HOBBY); },
                    "ChÆ°a cÃ³ ai cÃ¹ng sá»Ÿ thÃ­ch vá»›i báº¡n Ä‘á»ƒ gá»£i Ã½.");
                break;

            case 7:
                suggestionListScreen(myId, "=============== Gá»¢I Ã: CHUNG NHÃ“M CHAT ===============",
                    [&]() { return suggestByCommonGroup(myId); },
                    "ChÆ°a cÃ³ ngÆ°á»i phÃ¹ há»£p trong cÃ¡c nhÃ³m chat báº¡n Ä‘ang tham gia.");
                break;

            default:
                return;
        }
    }
}

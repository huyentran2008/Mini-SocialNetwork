// Module: Gợi ý kết bạn (duyệt BFS trên đồ thị bạn bè)
#pragma once

/*
    GỢI Ý KẾT BẠN
    Mỗi tài khoản là 1 đỉnh, mỗi quan hệ bạn bè (FRIEND) là 1 cạnh của đồ thị.
    Từ người dùng A, duyệt BFS (theo từng tầng):
        tầng 1: bạn của A                 (B)  - đã là bạn nên không gợi ý
        tầng 2: bạn của bạn               (C)  - gợi ý
        tầng 3: bạn của bạn của bạn       (D)  - gợi ý
    Ví dụ A kết bạn B, B kết bạn C, C kết bạn D thì A được gợi ý kết bạn với C và D.
    Người gần A hơn (tầng nhỏ hơn) được xếp trước; cùng tầng thì nhiều bạn chung hơn xếp trước.

    Nếu A chưa có bạn nào (hoặc không còn ai để gợi ý theo quan hệ) thì chỉ gợi ý
    một vài người dùng nổi bật (có nhiều bạn nhất), không liệt kê toàn bộ người dùng.
                                                                                        */

const int SUGGEST_MAX_DEPTH = 3;        // Đi tối đa 3 bước: A -> B -> C -> D
const int SUGGEST_MAX_RESULT = 10;      // Số gợi ý tối đa theo quan hệ bạn bè
const int SUGGEST_MAX_FALLBACK = 5;     // Số gợi ý tối đa khi A chưa có bạn để dựa vào

struct Suggestion {
    string id;
    string name;
    int distance = 0;       // 2: bạn của bạn, 3: bạn của bạn của bạn, 0: gợi ý chung
    int mutual = 0;         // Số bạn chung với tôi
    int friendCount = 0;    // Số bạn bè của người được gợi ý
    string reason;          // Lý do gợi ý theo thông tin cá nhân (cùng quê, cùng trường, chung sở thích...)
    int score = 0;          // Mức độ giống nhau (số sở thích chung...), dùng để xếp thứ tự
    int commonGroups = 0;    // Số nhóm chat cùng tham gia
};
// Người này có thể được gợi ý cho myId không. Đồng thời đọc hồ sơ vào profile
bool canSuggestTo(const string& myId, const string& id, UserProfile& profile) {
    if (id == myId) {
        return false;
    }

    // Đã là bạn, đã gửi / nhận lời mời hoặc đã chặn thì không gợi ý nữa
    if (!getRelation(myId, id).empty()) {
        return false;
    }

    if (getRelation(id, myId) == "BLOCK") {
        return false;
    }

    if (!findProfileById(id, profile)) {
        return false;
    }

    // Tài khoản đang bị quản trị viên khóa thì không gợi ý
    if (profile.status == "LOCKED") {
        return false;
    }

    return true;
}
// Gợi ý theo quan hệ bạn bè: BFS từ myId, lấy những người ở tầng 2 và tầng 3
vector<Suggestion> suggestByFriendGraph(const string& myId) {
    // Danh sách bạn của từng đỉnh, chỉ đọc file 1 lần cho mỗi đỉnh
    map<string, vector<string>> adjacency;

    auto neighbors = [&](const string& id) -> const vector<string>& {
        auto it = adjacency.find(id);

        if (it == adjacency.end()) {
            it = adjacency.emplace(id, getFriendIds(id)).first;
        }

        return it->second;
    };

    // BFS: order đóng vai trò hàng đợi, distance là số bước từ myId
    map<string, int> distance;
    vector<string> order;

    distance[myId] = 0;
    order.push_back(myId);

    for (size_t head = 0; head < order.size(); head++) {
        string current = order[head];
        int d = distance[current];

        if (d >= SUGGEST_MAX_DEPTH) {
            continue;
        }

        for (const string& next : neighbors(current)) {
            if (distance.count(next)) {
                continue;
            }

            distance[next] = d + 1;
            order.push_back(next);
        }
    }

    vector<Suggestion> list;

    for (const string& id : order) {
        int d = distance[id];

        // Tầng 0 là chính mình, tầng 1 là bạn bè hiện tại
        if (d < 2) {
            continue;
        }

        UserProfile profile;

        if (!canSuggestTo(myId, id, profile)) {
            continue;
        }

        Suggestion suggestion;
        suggestion.id = id;
        suggestion.name = profile.fullName.empty() ? profile.username : profile.fullName;
        suggestion.distance = d;
        suggestion.mutual = countMutualFriends(myId, id);
        suggestion.friendCount = (int)neighbors(id).size();
        list.push_back(suggestion);
    }

    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });

    if ((int)list.size() > SUGGEST_MAX_RESULT) {
        list.resize(SUGGEST_MAX_RESULT);
    }

    return list;
}
// Gợi ý người chưa kết bạn nhưng đang cùng tham gia một hoặc nhiều nhóm chat.
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
        suggestion.reason = "Chung " + to_string(item.second) + " nhóm chat";
        list.push_back(suggestion);
    }
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.commonGroups != b.commonGroups) return a.commonGroups > b.commonGroups;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });
    if ((int)list.size() > SUGGEST_MAX_RESULT) list.resize(SUGGEST_MAX_RESULT);
    return list;
}
// Gợi ý chung khi chưa có bạn để dựa vào: chỉ vài người có nhiều bạn nhất
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

    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.friendCount != b.friendCount) return a.friendCount > b.friendCount;
        if (a.name != b.name) return a.name < b.name;
        return a.id < b.id;
    });

    if ((int)list.size() > SUGGEST_MAX_FALLBACK) {
        list.resize(SUGGEST_MAX_FALLBACK);
    }

    return list;
}
// Danh sách gợi ý kết bạn cho myId
vector<Suggestion> computeSuggestions(const string& myId) {
    vector<Suggestion> list = suggestByFriendGraph(myId);

    if (list.empty()) {
        list = suggestPopularUsers(myId);
    }

    return list;
}
// Dòng hiển thị của 1 gợi ý
string suggestionLabel(const Suggestion& suggestion) {
    string text = suggestion.name;

    if (!suggestion.reason.empty()) {
        text += " (" + to_string(suggestion.mutual) + " Bạn Chung) - " + suggestion.reason;
    }
    else if (suggestion.distance == 2) {
        text += " (" + to_string(suggestion.mutual) + " Bạn Chung) - Bạn của bạn bè";
    }
    else if (suggestion.distance == 3) {
        text += " (" + to_string(suggestion.mutual) + " Bạn Chung) - Bạn của bạn của bạn bè";
    }
    else {
        text += " (" + to_string(suggestion.friendCount) + " Bạn Bè) - Người dùng nổi bật";
    }

    return text;
}
// Gợi ý theo thông tin cá nhân: những người (chưa có quan hệ gì với tôi) trùng với tôi ở 1 mục
enum SuggestKind { SUGGEST_HOMETOWN, SUGGEST_RESIDENCE, SUGGEST_SCHOOL, SUGGEST_WORKPLACE, SUGGEST_HOBBY };

// So sánh không phân biệt hoa thường và dấu, bỏ khoảng trắng thừa (dùng cho nơi làm việc nhập tay)
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
                suggestion.reason = "Cùng quê quán: " + profile.hometown;
                break;

            case SUGGEST_RESIDENCE:
                if (me.residence.empty() || profile.residence != me.residence) continue;
                suggestion.reason = "Cùng nơi ở: " + profile.residence;
                break;

            case SUGGEST_SCHOOL:
                if (me.school.empty() || profile.school != me.school) continue;
                suggestion.reason = "Cùng trường: " + profile.school;
                break;

            case SUGGEST_WORKPLACE:
                if (normalizeProfileText(me.workplace).empty() ||
                    normalizeProfileText(profile.workplace) != normalizeProfileText(me.workplace)) continue;
                suggestion.reason = "Cùng nơi làm việc: " + profile.workplace;
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
                suggestion.reason = "Chung sở thích: " + joinList(common, ", ");
                break;
            }
        }

        list.push_back(suggestion);
    }

    // Giống nhau nhiều (sở thích chung) trước, rồi nhiều bạn chung, rồi theo tên
    sort(list.begin(), list.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.mutual != b.mutual) return a.mutual > b.mutual;
        return a.name < b.name;
    });

    return list;
}
// Chữ thêm sau tên: số bạn chung và lý do gợi ý
string suggestionExtra(const Suggestion& suggestion) {
    string extra;

    if (suggestion.mutual > 0) {
        extra = to_string(suggestion.mutual) + " bạn chung";
    }

    string reason = suggestion.reason;

    if (reason.empty()) {
        reason = (suggestion.distance == 2) ? "Bạn của bạn bè"
               : (suggestion.distance == 3) ? "Bạn của bạn của bạn bè"
                                            : "Người dùng nổi bật";
    }

    return extra.empty() ? reason : extra + " · " + reason;
}
// Danh sách gợi ý dạng thẻ (lấy từ source mỗi lần nạp lại). Chọn 1 người để xem trang cá nhân
void suggestionListScreen(
    const string& myId,
    const string& title,
    const function<vector<Suggestion>()>& source,
    const string& emptyMessage
) {
    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = "Xem trang cá nhân";
    config.noun = "người";
    config.moreWord = "gợi ý nữa";
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
// Menu gợi ý kết bạn: chọn cách gợi ý
void suggestionScreen(const string& myId) {
    vector<string> items = {
        "Bạn Của Bạn Bè",
        "Theo Quê Quán",
        "Theo Nơi Ở",
        "Theo Trường Học",
        "Theo Nơi Làm Việc",
        "Theo Sở Thích",
        "Chung Nhóm Chat",
        "Quay Lại"
    };

    const string note = "Hãy cập nhật mục này trong Trang Cá Nhân > Chỉnh Sửa Tài Khoản để nhận gợi ý.";

    while (true) {
        UserProfile me;

        if (!findProfileById(myId, me)) {
            return;
        }

        int choice = menu(
            "=============== GỢI Ý KẾT BẠN ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                suggestionListScreen(myId, "=============== GỢI Ý: BẠN CỦA BẠN BÈ ===============",
                    [&]() { return computeSuggestions(myId); },
                    "Hiện chưa có gợi ý kết bạn nào cho bạn.");
                break;

            case 2:
                if (me.hometown.empty()) { notify("Bạn chưa chọn quê quán. " + note); break; }
                suggestionListScreen(myId, "=============== GỢI Ý: THEO QUÊ QUÁN ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_HOMETOWN); },
                    "Chưa có ai cùng quê quán (" + me.hometown + ") để gợi ý.");
                break;

            case 3:
                if (me.residence.empty()) { notify("Bạn chưa chọn nơi ở hiện tại. " + note); break; }
                suggestionListScreen(myId, "=============== GỢI Ý: THEO NƠI Ở ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_RESIDENCE); },
                    "Chưa có ai cùng nơi ở (" + me.residence + ") để gợi ý.");
                break;

            case 4:
                if (me.school.empty()) { notify("Bạn chưa chọn trường học. " + note); break; }
                suggestionListScreen(myId, "=============== GỢI Ý: THEO TRƯỜNG HỌC ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_SCHOOL); },
                    "Chưa có ai cùng trường (" + me.school + ") để gợi ý.");
                break;

            case 5:
                if (normalizeProfileText(me.workplace).empty()) { notify("Bạn chưa nhập nơi làm việc. " + note); break; }
                suggestionListScreen(myId, "=============== GỢI Ý: THEO NƠI LÀM VIỆC ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_WORKPLACE); },
                    "Chưa có ai cùng nơi làm việc (" + me.workplace + ") để gợi ý.");
                break;

            case 6:
                if (splitList(me.hobbies).empty()) { notify("Bạn chưa chọn sở thích nào. " + note); break; }
                suggestionListScreen(myId, "=============== GỢI Ý: THEO SỞ THÍCH ===============",
                    [&]() { return suggestByProfile(myId, SUGGEST_HOBBY); },
                    "Chưa có ai cùng sở thích với bạn để gợi ý.");
                break;

            case 7:
                suggestionListScreen(myId, "=============== GỢI Ý: CHUNG NHÓM CHAT ===============",
                    [&]() { return suggestByCommonGroup(myId); },
                    "Chưa có người phù hợp trong các nhóm chat bạn đang tham gia.");
                break;

            default:
                return;
        }
    }
}

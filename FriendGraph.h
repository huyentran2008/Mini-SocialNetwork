// Module: Đồ thị bạn bè + gợi ý kết bạn bằng BFS (tách từ Profile.h và Suggest.h)
#pragma once

/*  Cần có trước: Profile.h (UserProfile, findProfileById, isValidAccountId), Friend/FriendRequest.h (getRelation),
    Cards.h (CardListConfig, runCardList), ProfileScreens.h (makePersonItem, viewUserProfile)
                                                                                        */

/*
    ĐỒ THỊ BẠN BÈ: mỗi tài khoản là 1 đỉnh, mỗi quan hệ FRIEND là 1 cạnh
                                                            */
// Một dòng trong friend.txt: ACCID|STATUS|DATE
struct FriendEntry {
    string id;
    string status;      // FRIEND, PENDING_SENT, PENDING_RECEIVED, BLOCK
    string date;
};
// Đọc toàn bộ Data/Account/<ID>/friend.txt (chưa có file thì trả về danh sách rỗng)
vector<FriendEntry> readFriendList(const string& accountId) {
    vector<FriendEntry> list;

    if (!isValidAccountId(accountId)) {
        return list;
    }

    ifstream file(fs::path("Data/Account") / accountId / "friend.txt");

    if (!file.is_open()) {
        return list;
    }

    string line;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t first = line.find('|');

        if (first == string::npos) {
            continue;
        }

        size_t second = line.find('|', first + 1);

        FriendEntry entry;
        entry.id = trimString(line.substr(0, first));

        if (second == string::npos) {
            entry.status = trimString(line.substr(first + 1));
        }
        else {
            entry.status = trimString(line.substr(first + 1, second - first - 1));
            entry.date = trimString(line.substr(second + 1));
        }

        if (!entry.id.empty()) {
            list.push_back(entry);
        }
    }

    return list;
}
// ID của những người ĐÃ là bạn bè (trạng thái FRIEND), không trùng lặp
vector<string> getFriendIds(const string& accountId) {
    vector<string> ids;

    for (const auto& entry : readFriendList(accountId)) {
        if (entry.status != "FRIEND") {
            continue;
        }

        if (find(ids.begin(), ids.end(), entry.id) == ids.end()) {
            ids.push_back(entry.id);
        }
    }

    return ids;
}
// Số bạn bè của 1 tài khoản
int countFriends(const string& accountId) {
    return (int)getFriendIds(accountId).size();
}
// Số bạn chung giữa 2 tài khoản
int countMutualFriends(const string& idA, const string& idB) {
    vector<string> friendsA = getFriendIds(idA);
    vector<string> friendsB = getFriendIds(idB);

    int count = 0;

    for (const string& id : friendsA) {
        if (find(friendsB.begin(), friendsB.end(), id) != friendsB.end()) {
            count++;
        }
    }

    return count;
}
// ID các bạn chung giữa 2 tài khoản
vector<string> getMutualFriendIds(const string& idA, const string& idB) {
    vector<string> friendsB = getFriendIds(idB);
    vector<string> result;

    for (const string& id : getFriendIds(idA)) {
        if (find(friendsB.begin(), friendsB.end(), id) != friendsB.end()) {
            result.push_back(id);
        }
    }

    return result;
}


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

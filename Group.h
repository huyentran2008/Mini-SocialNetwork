// Module: Nhóm chat
#pragma once

// Thành viên của nhóm
struct GroupMember {
    string id;
    string addedBy;     // Người đã thêm (phải là bạn bè của thành viên mới)
    string date;
};
struct Group {
    string id;
    string name;
    string ownerId;
    string createdAt;
    vector<GroupMember> members;
};
fs::path groupDir(const string& groupId) {
    return fs::path("Data/Conversations/.Group") / groupId;
}
// Đọc thông tin 1 nhóm
bool readGroup(const string& groupId, Group& group) {
    fs::path dir = groupDir(groupId);
    vector<string> info = readLines(dir / "info.txt");

    if (info.empty()) {
        return false;
    }

    Group current;
    current.id = groupId;

    for (const string& line : info) {
        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "name")              current.name = value;
        else if (key == "owner")        current.ownerId = value;
        else if (key == "created_at")   current.createdAt = value;
    }

    for (const string& line : readLines(dir / "members.txt")) {
        size_t first = line.find('|');
        GroupMember member;

        if (first == string::npos) {
            member.id = trimString(line);
        }
        else {
            member.id = trimString(line.substr(0, first));
            size_t second = line.find('|', first + 1);

            if (second == string::npos) {
                member.addedBy = trimString(line.substr(first + 1));
            }
            else {
                member.addedBy = trimString(line.substr(first + 1, second - first - 1));
                member.date = trimString(line.substr(second + 1));
            }
        }

        if (!member.id.empty()) {
            current.members.push_back(member);
        }
    }

    if (current.name.empty()) {
        return false;
    }

    group = current;
    return true;
}
// Ghi thông tin và danh sách thành viên của nhóm
bool writeGroup(const Group& group) {
    error_code ec;
    fs::create_directories(groupDir(group.id), ec);

    vector<string> info = {
        "id=" + group.id,
        "name=" + group.name,
        "owner=" + group.ownerId,
        "created_at=" + group.createdAt
    };

    vector<string> members;

    for (const GroupMember& member : group.members) {
        members.push_back(member.id + "|" + member.addedBy + "|" + member.date);
    }

    return writeLines(groupDir(group.id) / "info.txt", info) &&
           writeLines(groupDir(group.id) / "members.txt", members);
}
vector<string> listGroupIds() {
    vector<string> ids;
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Conversations/.Group", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (name.size() > 3 && name.compare(0, 3, "GRP") == 0) {
            ids.push_back(name);
        }
    }

    sort(ids.begin(), ids.end());
    return ids;
}
bool isGroupMember(const Group& group, const string& accountId) {
    for (const GroupMember& member : group.members) {
        if (member.id == accountId) {
            return true;
        }
    }

    return false;
}
// Ghi 1 thông báo hệ thống vào cuộc trò chuyện của nhóm
void addGroupNotice(const string& groupId, const string& text) {
    appendMessage(groupDir(groupId) / "messages.txt", "SYSTEM", text);
}
// Thời điểm hoạt động gần nhất của nhóm (tin nhắn cuối, nếu chưa có thì lúc tạo nhóm)
string groupLastActivity(const Group& group) {
    vector<ChatMessage> messages = readMessages(groupDir(group.id) / "messages.txt");

    if (!messages.empty()) {
        return messages.back().time;
    }

    return group.createdAt;
}
// Các nhóm mà accountId đang tham gia, hoạt động gần nhất ở trên
vector<Group> getGroupsOf(const string& accountId) {
    vector<Group> groups;

    for (const string& id : listGroupIds()) {
        Group group;

        if (readGroup(id, group) && isGroupMember(group, accountId)) {
            groups.push_back(group);
        }
    }

    sort(groups.begin(), groups.end(), [](const Group& a, const Group& b) {
        string timeA = groupLastActivity(a);
        string timeB = groupLastActivity(b);

        if (timeA != timeB) return timeA > timeB;
        return a.id > b.id;
    });

    return groups;
}
// Gỡ 1 thành viên khỏi nhóm. Chủ nhóm rời đi thì thành viên cũ nhất làm chủ nhóm,
// nhóm hết người thì xóa nhóm. notice: nội dung thông báo ghi vào nhóm (rỗng = không ghi)
void removeGroupMember(const string& groupId, const string& memberId, const string& notice) {
    Group group;

    if (!readGroup(groupId, group)) {
        return;
    }

    group.members.erase(remove_if(group.members.begin(), group.members.end(),
                                  [&](const GroupMember& member) { return member.id == memberId; }),
                        group.members.end());

    if (group.members.empty()) {
        error_code ec;
        fs::remove_all(groupDir(groupId), ec);
        return;
    }

    if (group.ownerId == memberId) {
        group.ownerId = group.members.front().id;
    }

    writeGroup(group);

    if (!notice.empty()) {
        addGroupNotice(groupId, notice);
    }
}
// Danh sách bạn bè (còn tồn tại, không bị chặn) của me, bỏ qua những ID trong exclude
vector<string> getInvitableFriends(const string& me, const vector<string>& exclude) {
    vector<string> result;

    for (const string& id : getFriendIds(me)) {
        if (find(exclude.begin(), exclude.end(), id) != exclude.end()) {
            continue;
        }

        UserProfile profile;

        if (!findProfileById(id, profile) || isBlockedEither(me, id)) {
            continue;
        }

        result.push_back(id);
    }

    return result;
}
// Thêm bạn bè của mình vào nhóm (chỉ bạn bè của người thêm mới được thêm)
void addGroupMemberFlow(const string& me, const string& groupId) {
    Group group;

    if (!readGroup(groupId, group)) {
        return;
    }

    vector<string> existing;

    for (const GroupMember& member : group.members) {
        existing.push_back(member.id);
    }

    vector<string> candidates = getInvitableFriends(me, existing);

    if (candidates.empty()) {
        notify("Không còn người bạn nào của bạn để thêm vào nhóm!");
        return;
    }

    vector<string> items;

    for (const string& id : candidates) {
        items.push_back(getDisplayName(id));
    }

    int choice = scrollMenu(
        "=============== THÊM THÀNH VIÊN VÀO NHÓM ===============",
        items,
        "Chỉ thêm được bạn bè của bạn | Sử dụng ▲ / ▼ | ENTER: Thêm | ESC: Quay lại"
    );

    if (choice == 0) {
        return;
    }

    const string& newId = candidates[choice - 1];

    // Kiểm tra lại điều kiện: người thêm phải là bạn bè của thành viên mới
    if (!areFriends(me, newId)) {
        notify("Bạn chỉ có thể thêm bạn bè của mình vào nhóm!");
        return;
    }

    GroupMember member;
    member.id = newId;
    member.addedBy = me;
    member.date = todayIso();
    group.members.push_back(member);

    if (!writeGroup(group)) {
        notify("Không thể lưu nhóm, vui lòng thử lại!");
        return;
    }

    addGroupNotice(groupId, getDisplayName(me) + " đã thêm " + getDisplayName(newId) + " vào nhóm");
    notify("Đã thêm " + getDisplayName(newId) + " vào nhóm!");
}
// Xem danh sách thành viên nhóm (chủ nhóm có thể xóa thành viên)
void groupMembersScreen(const string& me, const string& groupId) {
    int lastChoice = 1;

    while (true) {
        Group group;

        if (!readGroup(groupId, group) || !isGroupMember(group, me)) {
            return;
        }

        string title = "=============== THÀNH VIÊN NHÓM " + group.name + " ===============";
        vector<string> items;

        for (const GroupMember& member : group.members) {
            string item = getDisplayName(member.id);

            if (member.id == group.ownerId) item += " (Chủ nhóm)";
            if (member.id == me)            item += " (Bạn)";

            items.push_back(item);
        }

        int choice = scrollMenu(
            title,
            items,
            group.ownerId == me
                ? "Sử dụng ▲ / ▼ | ENTER: Chọn thành viên | ESC: Quay lại"
                : "Sử dụng ▲ / ▼ | ENTER: Xem trang cá nhân | ESC: Quay lại",
            lastChoice
        );

        if (choice == 0) {
            return;
        }

        lastChoice = choice;
        string memberId = group.members[choice - 1].id;

        if (memberId == me) {
            continue;
        }

        // Chủ nhóm: chọn xem trang cá nhân hoặc xóa khỏi nhóm
        if (group.ownerId == me) {
            vector<string> actions = {
                "Xem Trang Cá Nhân",
                "Xóa Khỏi Nhóm",
                "Quay Lại"
            };

            int action = menu(
                "=============== " + getDisplayName(memberId) + " ===============",
                actions,
                "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
            );

            if (action == 1) {
                viewUserProfile(me, memberId);
            }
            else if (action == 2) {
                if (confirmScreen("Xóa " + getDisplayName(memberId) + " khỏi nhóm?")) {
                    removeGroupMember(groupId, memberId,
                        getDisplayName(me) + " đã xóa " + getDisplayName(memberId) + " khỏi nhóm");
                    notify("Đã xóa thành viên khỏi nhóm!");
                }
            }
        }
        else {
            viewUserProfile(me, memberId);
        }
    }
}
// Menu của 1 nhóm: mở trò chuyện, xem / thêm thành viên, đổi tên, rời nhóm
void groupMenu(const string& me, const string& groupId) {
    while (true) {
        Group group;

        if (!readGroup(groupId, group) || !isGroupMember(group, me)) {
            notify("Bạn không còn là thành viên của nhóm này!");
            return;
        }

        bool isOwner = (group.ownerId == me);

        vector<string> items, actions;
        auto add = [&](const string& label, const string& action) {
            items.push_back(label);
            actions.push_back(action);
        };

        add("Mở Cuộc Trò Chuyện", "chat");
        add("Xem Thành Viên (" + to_string(group.members.size()) + ")", "members");
        add("Thêm Thành Viên", "add");
        if (isOwner) add("Đổi Tên Nhóm", "rename");
        add("Rời Nhóm", "leave");
        add("Quay Lại", "back");

        int choice = menu(
            "=============== NHÓM: " + group.name + " ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        if (choice == 0) {
            return;
        }

        const string& action = actions[choice - 1];

        if (action == "back") {
            return;
        }
        else if (action == "chat") {
            runChat(me, "=============== NHÓM: " + group.name + " ===============",
                    groupDir(groupId) / "messages.txt",
                    [&]() -> string {
                        Group current;

                        if (!readGroup(groupId, current) || !isGroupMember(current, me)) {
                            return "Bạn không còn là thành viên của nhóm này!";
                        }

                        return "";
                    });
        }
        else if (action == "members") {
            groupMembersScreen(me, groupId);
        }
        else if (action == "add") {
            addGroupMemberFlow(me, groupId);
        }
        else if (action == "rename") {
            InputOptions options;
            options.title = "=============== ĐỔI TÊN NHÓM ===============";
            options.prompt = "Nhập tên mới cho nhóm (tối đa 30 ký tự)";
            options.maxLength = 30;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "Tên nhóm không được để trống!" : "";
            };

            string newName = group.name;

            if (inputText(options, newName) && newName != group.name) {
                string oldName = group.name;
                group.name = newName;

                if (writeGroup(group)) {
                    addGroupNotice(groupId, getDisplayName(me) + " đã đổi tên nhóm từ \""
                                            + oldName + "\" thành \"" + newName + "\"");
                    notify("Đã đổi tên nhóm!");
                }
            }
        }
        else if (action == "leave") {
            if (confirmScreen("Bạn có chắc muốn rời nhóm " + group.name + "?")) {
                removeGroupMember(groupId, me, getDisplayName(me) + " đã rời nhóm");
                notify("Bạn đã rời nhóm!");
                return;
            }
        }
    }
}
// Tạo nhóm mới: đặt tên, chọn thành viên từ danh sách bạn bè của mình
void createGroupFlow(const string& me) {
    vector<string> friends = getInvitableFriends(me, {});

    if (friends.empty()) {
        notify("Bạn cần có bạn bè để tạo nhóm! Hãy đi kết bạn trước nhé!");
        return;
    }

    InputOptions options;
    options.title = "=============== TẠO NHÓM MỚI ===============";
    options.prompt = "Nhập tên nhóm (tối đa 30 ký tự)";
    options.maxLength = 30;
    options.validate = [](const string& s) -> string {
        return s.empty() ? "Tên nhóm không được để trống!" : "";
    };

    string name;

    if (!inputText(options, name)) {
        return;
    }

    // Chọn thành viên: ENTER để chọn / bỏ chọn, mục cuối để hoàn tất
    vector<bool> selected(friends.size(), false);
    int lastChoice = 1;

    while (true) {
        vector<string> items;
        int count = 0;

        for (size_t i = 0; i < friends.size(); i++) {
            items.push_back(string(selected[i] ? "[x] " : "[ ] ") + getDisplayName(friends[i]));
            if (selected[i]) count++;
        }

        items.push_back("=> Hoàn Tất Tạo Nhóm (" + to_string(count) + " người đã chọn)");

        int choice = scrollMenu(
            "=============== CHỌN THÀNH VIÊN CHO NHÓM " + name + " ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn / Bỏ chọn | ESC: Hủy tạo nhóm",
            lastChoice
        );

        if (choice == 0) {
            notify("Đã hủy tạo nhóm!");
            return;
        }

        lastChoice = choice;

        if (choice <= (int)friends.size()) {
            selected[choice - 1] = !selected[choice - 1];
            continue;
        }

        if (count == 0) {
            notify("Hãy chọn ít nhất 1 người bạn để tạo nhóm!");
            continue;
        }

        break;
    }

    Group group;
    group.id = nextId("Data/Conversations/.Group", "GRP");
    group.name = name;
    group.ownerId = me;
    group.createdAt = nowTimestamp();

    GroupMember owner;
    owner.id = me;
    owner.addedBy = me;
    owner.date = todayIso();
    group.members.push_back(owner);

    // Mỗi thành viên đều là bạn bè của người thêm (chính là người tạo nhóm)
    for (size_t i = 0; i < friends.size(); i++) {
        if (!selected[i]) continue;

        GroupMember member;
        member.id = friends[i];
        member.addedBy = me;
        member.date = todayIso();
        group.members.push_back(member);
    }

    if (!writeGroup(group)) {
        notify("Không thể lưu nhóm, vui lòng thử lại!");
        return;
    }

    addGroupNotice(group.id, getDisplayName(me) + " đã tạo nhóm \"" + name + "\"");
    notify("Tạo nhóm thành công!");

    groupMenu(me, group.id);
}
// Chữ xem trước của tin nhắn cuối trong 1 cuộc trò chuyện: "Bạn: ..." / "Tên: ..." (dòng quá dài sẽ được cắt "...")
string lastMessagePreview(const vector<ChatMessage>& messages, const string& me, bool showSenderName) {
    if (messages.empty()) {
        return "(chưa có tin nhắn)";
    }

    const ChatMessage& last = messages.back();
    // Tin nhắn cuối đã thu hồi cần có bản xem trước rõ ràng ngoài danh sách chat.
    string text = last.text == WITHDRAWN_MESSAGE_TEXT
        ? "Bạn đã thu hồi tin nhắn" : oneLine(last.text);

    if (last.sender == "SYSTEM") {
        return text;
    }

    if (last.sender == me) {
        return "Bạn: " + text;
    }

    return showSenderName ? getDisplayName(last.sender) + ": " + text : text;
}
// Thẻ 2 dòng của 1 cuộc trò chuyện: tên (và thời gian) / tin nhắn mới nhất
CardItem makeConversationItem(const string& key, const string& head, const string& lastTime,
                              const string& preview) {
    CardItem item;
    item.key = key;
    item.card.head = head;

    string ago = timeAgo(lastTime);
    if (!ago.empty()) item.card.head += " · " + ago;

    item.card.title = preview;
    item.card.titleColor = 7;
    item.searchText = head;
    return item;
}
// Danh sách nhóm của tôi: hiện đủ mọi nhóm, mỗi nhóm 2 dòng (tên nhóm + số thành viên / tin nhắn mới nhất)
void groupListScreen(const string& me) {
    CardListConfig config;
    config.title = "=============== NHÓM CHAT CỦA TÔI ===============";
    config.emptyMessage = "Bạn chưa tham gia nhóm nào. Hãy tạo nhóm mới nhé!";
    config.selectHint = "Mở nhóm";
    config.noun = "nhóm";
    config.pageSize = 0;            // Hiện toàn bộ danh sách
    config.searchable = false;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const Group& group : getGroupsOf(me)) {
                vector<ChatMessage> messages = readMessages(groupDir(group.id) / "messages.txt");
                string lastTime = messages.empty() ? group.createdAt : messages.back().time;

                items.push_back(makeConversationItem(
                    group.id,
                    group.name + " (" + to_string(group.members.size()) + " thành viên)",
                    lastTime,
                    lastMessagePreview(messages, me, true)
                ));
            }

            return items;
        },
        [&](const string& groupId) {
            groupMenu(me, groupId);
        }
    );
}
// Danh sách bạn bè để nhắn tin riêng: hiện đủ mọi người, mỗi người 2 dòng (tên / tin nhắn mới nhất),
// người có tin nhắn gần nhất ở trên
void privateChatListScreen(const string& me) {
    CardListConfig config;
    config.title = "=============== TIN NHẮN RIÊNG ===============";
    config.emptyMessage = getNoFriendMessage();
    config.selectHint = "Mở cuộc trò chuyện";
    config.noun = "cuộc trò chuyện";
    config.pageSize = 0;            // Hiện toàn bộ danh sách
    config.searchable = false;

    runCardList(
        config,
        [&](int) {
            struct Conversation {
                CardItem item;
                string lastTime;
                string name;
            };

            vector<Conversation> list;

            for (const string& id : getFriendIds(me)) {
                UserProfile profile;

                if (!findProfileById(id, profile) || isBlockedEither(me, id)) {
                    continue;
                }

                string name = profile.fullName.empty() ? profile.username : profile.fullName;
                string nickname = getNickname(me, id);
                if (!nickname.empty()) name = nickname;
                vector<ChatMessage> messages = readMessages(privateChatFile(me, id));

                Conversation conversation;
                conversation.name = name;
                conversation.lastTime = messages.empty() ? "" : messages.back().time;
                conversation.item = makeConversationItem(id, name, conversation.lastTime,
                                                         lastMessagePreview(messages, me, false));
                list.push_back(conversation);
            }

            sort(list.begin(), list.end(), [](const Conversation& a, const Conversation& b) {
                if (a.lastTime != b.lastTime) return a.lastTime > b.lastTime;
                return a.name < b.name;
            });

            vector<CardItem> items;

            for (const Conversation& conversation : list) {
                items.push_back(conversation.item);
            }

            return items;
        },
        [&](const string& friendId) {
            openPrivateChat(me, friendId);
        }
    );
}

// Module: Quan hệ bạn bè, cập nhật hồ sơ
#pragma once

/*
    QUAN HỆ BẠN BÈ (friend.txt: ACCID|STATUS|YYYY-MM-DD)
                                                            */

// Ghi lại toàn bộ friend.txt của 1 tài khoản
bool writeFriendList(const string& accountId, const vector<FriendEntry>& list) {
    if (!isValidAccountId(accountId)) {
        return false;
    }

    vector<string> lines;

    for (const FriendEntry& entry : list) {
        lines.push_back(entry.id + "|" + entry.status + "|" + entry.date);
    }

    return writeLines(fs::path("Data/Account") / accountId / "friend.txt", lines);
}
// Trạng thái mà owner đang ghi cho other ("" nếu không có)
string getRelation(const string& owner, const string& other) {
    for (const FriendEntry& entry : readFriendList(owner)) {
        if (entry.id == other) {
            return entry.status;
        }
    }

    return "";
}
// Đặt trạng thái owner -> other (status rỗng = xóa quan hệ)
bool setRelation(const string& owner, const string& other, const string& status) {
    vector<FriendEntry> list = readFriendList(owner);

    list.erase(remove_if(list.begin(), list.end(),
                         [&](const FriendEntry& entry) { return entry.id == other; }),
               list.end());

    if (!status.empty()) {
        FriendEntry entry;
        entry.id = other;
        entry.status = status;
        entry.date = todayIso();
        list.push_back(entry);
    }

    return writeFriendList(owner, list);
}
// Hai tài khoản có là bạn bè không
bool areFriends(const string& a, const string& b) {
    return getRelation(a, b) == "FRIEND" || getRelation(b, a) == "FRIEND";
}
// Có ai chặn ai giữa hai tài khoản không
bool isBlockedEither(const string& a, const string& b) {
    return getRelation(a, b) == "BLOCK" || getRelation(b, a) == "BLOCK";
}
// ID của những người có trạng thái status trong danh sách của accountId
vector<string> getIdsByStatus(const string& accountId, const string& status) {
    vector<string> ids;

    for (const FriendEntry& entry : readFriendList(accountId)) {
        if (entry.status != status) {
            continue;
        }

        if (find(ids.begin(), ids.end(), entry.id) == ids.end()) {
            ids.push_back(entry.id);
        }
    }

    return ids;
}
// Gửi lời mời kết bạn. Trả về thông báo kết quả để hiển thị
string sendFriendRequest(const string& me, const string& other) {
    if (me == other) {
        return "Bạn không thể kết bạn với chính mình!";
    }

    if (isBlockedEither(me, other)) {
        return "Không thể gửi lời mời cho người này!";
    }

    string status = getRelation(me, other);

    if (status == "FRIEND") {
        return "Hai bạn đã là bạn bè rồi!";
    }

    if (status == "PENDING_SENT") {
        return "Bạn đã gửi lời mời cho người này rồi!";
    }

    // Người kia đã gửi lời mời cho mình trước: kết bạn luôn
    if (status == "PENDING_RECEIVED") {
        setRelation(me, other, "FRIEND");
        setRelation(other, me, "FRIEND");
        dropFriendRequestNotifications(me, other);
        addNotification(other, NOTI_FRIEND_ACCEPT, me);
        return "Người này đã gửi lời mời cho bạn nên hai bạn đã trở thành bạn bè!";
    }

    setRelation(me, other, "PENDING_SENT");
    setRelation(other, me, "PENDING_RECEIVED");
    addNotification(other, NOTI_FRIEND_REQUEST, me);
    return "Đã gửi lời mời kết bạn!";
}
// Chấp nhận lời mời kết bạn đã nhận
bool acceptFriendRequest(const string& me, const string& other) {
    if (getRelation(me, other) != "PENDING_RECEIVED") {
        return false;
    }

    setRelation(me, other, "FRIEND");
    setRelation(other, me, "FRIEND");
    dropFriendRequestNotifications(me, other);
    addNotification(other, NOTI_FRIEND_ACCEPT, me);
    return true;
}
// Xóa quan hệ ở cả hai phía (từ chối / hủy lời mời / hủy kết bạn)
void removeRelationBoth(const string& a, const string& b) {
    setRelation(a, b, "");
    setRelation(b, a, "");
    dropFriendRequestNotifications(a, b);
    dropFriendRequestNotifications(b, a);
}
// Chặn: xóa mọi quan hệ cũ rồi ghi BLOCK ở phía người chặn
void blockUser(const string& me, const string& other) {
    // Giữ lại BLOCK của người kia (nếu họ cũng đã chặn mình)
    if (getRelation(other, me) != "BLOCK") {
        setRelation(other, me, "");
    }

    setRelation(me, other, "BLOCK");
}
// Bỏ chặn
void unblockUser(const string& me, const string& other) {
    if (getRelation(me, other) == "BLOCK") {
        setRelation(me, other, "");
    }
}
// Tên đã có người dùng chưa (không phân biệt hoa thường với chữ ASCII)
bool fullNameTaken(const string& name, const string& exceptAccountId) {
    string wanted = lowerAscii(trimString(name));

    for (const string& id : listAccountIds()) {
        if (id == exceptAccountId) {
            continue;
        }

        UserProfile profile;

        if (!findProfileById(id, profile)) {
            continue;
        }

        if (lowerAscii(trimString(profile.fullName)) == wanted) {
            return true;
        }
    }

    return false;
}
// Sửa (hoặc thêm) 1 dòng key=value trong profile.txt
bool updateProfileField(const string& accountId, const string& key, const string& value) {
    if (!isValidAccountId(accountId)) {
        return false;
    }

    fs::path file = fs::path("Data/Account") / accountId / "profile.txt";
    vector<string> lines = readLines(file);
    bool found = false;

    for (string& line : lines) {
        if (line.compare(0, key.size() + 1, key + "=") == 0) {
            line = key + "=" + value;
            found = true;
        }
    }

    if (!found) {
        lines.push_back(key + "=" + value);
    }

    return writeLines(file, lines);
}


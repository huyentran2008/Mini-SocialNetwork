// Module: Lời mời kết bạn (tách từ Friend.h, Notification.h, ProfileScreens.h)
#pragma once

/*  Cần có trước: Profile.h (FriendEntry, readFriendList, isValidAccountId), Notification.h (addNotification, NOTI_*),
    Utils.h (writeLines, todayIso), ProfileScreens.h (personListScreen)
                                                            */

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

// Gỡ thông báo "lời mời kết bạn" của fromId khỏi hộp thông báo của owner
// (lời mời đã được chấp nhận, từ chối hoặc hủy thì thông báo không còn ý nghĩa)
void dropFriendRequestNotifications(const string& ownerId, const string& fromId) {
    vector<Notification> list = readNotifications(ownerId);
    size_t before = list.size();

    list.erase(remove_if(list.begin(), list.end(), [&](const Notification& n) {
        return n.type == NOTI_FRIEND_REQUEST && n.fromId == fromId;
    }), list.end());

    if (list.size() != before) {
        writeNotifications(ownerId, list);
    }
}
/*
    DANH SÁCH LỜI MỜI KẾT BẠN
                                */

// Danh sách lời mời kết bạn (received = true: đã nhận, false: đã gửi)
void requestListScreen(const string& myId, bool received) {
    string status = received ? "PENDING_RECEIVED" : "PENDING_SENT";

    personListScreen(
        myId,
        received ? "=============== LỜI MỜI KẾT BẠN ĐÃ NHẬN ==============="
                 : "=============== LỜI MỜI KẾT BẠN ĐÃ GỬI ===============",
        received ? "Bạn không có lời mời kết bạn nào."
                 : "Bạn chưa gửi lời mời kết bạn nào.",
        received ? "Xem & phản hồi" : "Xem trang cá nhân",
        [&]() {
            vector<string> ids;

            for (const string& id : getIdsByStatus(myId, status)) {
                if (!isBlockedEither(myId, id)) {
                    ids.push_back(id);
                }
            }

            return ids;
        }
    );
}

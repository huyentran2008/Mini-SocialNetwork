// Module: Thông báo (lưu trữ dữ liệu)
#pragma once

/*
    THÔNG BÁO CỦA NGƯỜI DÙNG (Data/Account/ACCxxxxxx/notifications.txt)
    Mỗi dòng: LOẠI|NGƯỜI GÂY RA|ĐỐI TƯỢNG|THỜI GIAN|ĐÃ ĐỌC (0/1)
        FRIEND_REQUEST : có người gửi lời mời kết bạn cho mình
        FRIEND_ACCEPT  : có người chấp nhận lời mời kết bạn của mình
        POST_LIKE      : có người thích bài đăng của mình  (ĐỐI TƯỢNG = ID bài đăng)
        POST_COMMENT   : có người bình luận bài đăng của mình (ĐỐI TƯỢNG = ID bài đăng)
        MESSAGE        : có người nhắn tin (chỉ báo khi nhắn lần đầu, hoặc nhắn lại sau
                         NOTI_MESSAGE_GAP_SECONDS không ai nhắn - không báo mỗi tin nhắn)
    Nội dung chữ của thông báo được tạo lúc hiển thị (theo tên hiện tại của người gửi).
                                                                                        */

const string NOTI_FRIEND_REQUEST = "FRIEND_REQUEST";
const string NOTI_FRIEND_ACCEPT = "FRIEND_ACCEPT";
const string NOTI_POST_LIKE = "POST_LIKE";
const string NOTI_POST_COMMENT = "POST_COMMENT";
const string NOTI_MESSAGE = "MESSAGE";

const long long NOTI_MESSAGE_GAP_SECONDS = 3LL * 24 * 3600;   // 3 ngày không nhắn thì nhắn lại sẽ báo
const size_t NOTI_MAX_KEEP = 200;                             // Mỗi người giữ tối đa 200 thông báo gần nhất

struct Notification {
    string type;
    string fromId;
    string target;
    string time;
    bool read = false;
};

fs::path notificationFile(const string& accountId) {
    return fs::path("Data/Account") / accountId / "notifications.txt";
}
// Đọc thông báo của 1 tài khoản (cũ nhất ở đầu)
vector<Notification> readNotifications(const string& accountId) {
    vector<Notification> list;

    for (const string& line : readLines(notificationFile(accountId))) {
        vector<string> parts;
        size_t start = 0;

        while (parts.size() < 4) {
            size_t pos = line.find('|', start);

            if (pos == string::npos) {
                break;
            }

            parts.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }

        if (parts.size() < 4) {
            continue;
        }

        Notification n;
        n.type = parts[0];
        n.fromId = parts[1];
        n.target = parts[2];
        n.time = parts[3];
        n.read = (line.substr(start) == "1");
        list.push_back(n);
    }

    return list;
}
bool writeNotifications(const string& accountId, vector<Notification> list) {
    error_code ec;

    if (!fs::is_directory(fs::path("Data/Account") / accountId, ec)) {
        return false;
    }

    if (list.size() > NOTI_MAX_KEEP) {
        list.erase(list.begin(), list.end() - NOTI_MAX_KEEP);
    }

    vector<string> lines;

    for (const Notification& n : list) {
        lines.push_back(n.type + "|" + n.fromId + "|" + n.target + "|" + n.time + "|" + (n.read ? "1" : "0"));
    }

    return writeLines(notificationFile(accountId), lines);
}
// Gửi thông báo cho toId. Không báo cho chính mình, không báo trùng thông báo chưa đọc giống hệt
// (thích bài thì mỗi người chỉ báo 1 lần dù bấm thích / bỏ thích nhiều lần)
void addNotification(const string& toId, const string& type, const string& fromId,
                     const string& target = "") {
    if (toId == fromId || toId.empty()) {
        return;
    }

    error_code ec;

    if (!fs::is_directory(fs::path("Data/Account") / toId, ec)) {
        return;
    }

    vector<Notification> list = readNotifications(toId);

    for (const Notification& n : list) {
        if (n.type == type && n.fromId == fromId && n.target == target &&
            (!n.read || type == NOTI_POST_LIKE)) {
            return;
        }
    }

    Notification n;
    n.type = type;
    n.fromId = fromId;
    n.target = target;
    n.time = nowTimestamp();
    list.push_back(n);

    writeNotifications(toId, list);
}
int countUnreadNotifications(const string& accountId) {
    int count = 0;

    for (const Notification& n : readNotifications(accountId)) {
        if (!n.read) {
            count++;
        }
    }

    return count;
}
// Đánh dấu 1 thông báo là đã đọc
void markNotificationRead(const string& accountId, const Notification& target) {
    vector<Notification> list = readNotifications(accountId);

    for (Notification& n : list) {
        if (n.type == target.type && n.fromId == target.fromId &&
            n.target == target.target && n.time == target.time) {
            n.read = true;
        }
    }

    writeNotifications(accountId, list);
}
void markAllNotificationsRead(const string& accountId) {
    vector<Notification> list = readNotifications(accountId);

    for (Notification& n : list) {
        n.read = true;
    }

    writeNotifications(accountId, list);
}
void clearNotifications(const string& accountId) {
    writeNotifications(accountId, {});
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
// Xóa mọi thông báo do 1 tài khoản gây ra (khi tài khoản đó bị xóa)
void removeNotificationsFrom(const string& fromId) {
    for (const string& id : listAccountIds()) {
        vector<Notification> list = readNotifications(id);
        size_t before = list.size();

        list.erase(remove_if(list.begin(), list.end(), [&](const Notification& n) {
            return n.fromId == fromId;
        }), list.end());

        if (list.size() != before) {
            writeNotifications(id, list);
        }
    }
}
// Tin nhắn mới có cần báo cho người kia không: nhắn lần đầu (lastMessageTime rỗng)
// hoặc đã lâu (>= NOTI_MESSAGE_GAP_SECONDS) chưa ai nhắn trong cuộc trò chuyện
bool shouldNotifyMessage(const string& lastMessageTime) {
    if (lastMessageTime.empty()) {
        return true;
    }

    time_t last;

    if (!parseTimestamp(lastMessageTime, last)) {
        return true;
    }

    return difftime(time(nullptr), last) >= (double)NOTI_MESSAGE_GAP_SECONDS;
}

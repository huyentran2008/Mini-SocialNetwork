// Module: Màn hình thông báo của người dùng
#pragma once

/*
    THÔNG BÁO: danh sách (mới nhất ở trên, thông báo chưa đọc có dấu ●),
    chọn 1 thông báo để mở đúng nơi liên quan (trang cá nhân, bài đăng, cuộc trò chuyện)
                                                                                        */

// Nội dung chữ của 1 thông báo
string notificationText(const Notification& n) {
    string name = getDisplayName(n.fromId);

    if (n.type == NOTI_FRIEND_REQUEST) {
        return name + " đã gửi cho bạn lời mời kết bạn";
    }

    if (n.type == NOTI_FRIEND_ACCEPT) {
        return name + " đã chấp nhận lời mời kết bạn của bạn";
    }

    if (n.type == NOTI_MESSAGE) {
        return name + " đã nhắn tin cho bạn";
    }

    // Thích / bình luận bài đăng
    string postName = "một bài đăng đã bị xóa";
    Post post;

    if (readPost(n.target, post)) {
        postName = "\"" + displayTitle(post, 30) + "\"";
    }

    if (n.type == NOTI_POST_LIKE) {
        return name + " đã thích bài đăng " + postName + " của bạn";
    }

    return name + " đã bình luận bài đăng " + postName + " của bạn";
}
// Mở nơi liên quan tới thông báo (và đánh dấu đã đọc)
void openNotification(const string& myId, const Notification& n) {
    markNotificationRead(myId, n);

    if (n.type == NOTI_POST_LIKE || n.type == NOTI_POST_COMMENT) {
        Post post;

        if (readPost(n.target, post)) {
            viewPost(myId, n.target);
        }
        else {
            notify("Bài đăng này không còn tồn tại!");
        }

        return;
    }

    UserProfile profile;

    if (!findProfileById(n.fromId, profile)) {
        notify("Người này không còn tồn tại!");
        return;
    }

    // Tin nhắn: vào thẳng cuộc trò chuyện nếu vẫn là bạn bè, ngược lại xem trang cá nhân
    if (n.type == NOTI_MESSAGE && areFriends(myId, n.fromId) && !isBlockedEither(myId, n.fromId)) {
        openPrivateChat(myId, n.fromId);
        return;
    }

    viewUserProfile(myId, n.fromId);
}
// Danh sách thông báo: 10 thông báo mới nhất, nhấn M để xem thêm, A để xem tất cả
void notificationListScreen(const string& myId) {
    CardListConfig config;
    config.title = "=============== THÔNG BÁO ===============";
    config.emptyMessage = "Bạn chưa có thông báo nào.";
    config.selectHint = "Mở thông báo (● là chưa đọc)";
    config.noun = "thông báo";
    config.moreWord = "thông báo cũ hơn";
    config.pageSize = 10;
    config.searchable = false;

    // Mới nhất ở trên
    auto loadList = [&]() {
        vector<Notification> list = readNotifications(myId);
        reverse(list.begin(), list.end());
        return list;
    };

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;
            vector<Notification> list = loadList();

            for (size_t i = 0; i < list.size(); i++) {
                CardItem item;
                item.key = to_string(i);        // Vị trí trong danh sách mới nhất -> cũ nhất
                item.card.head = string(list[i].read ? "  " : "● ") + notificationText(list[i]);
                item.card.headColor = list[i].read ? 7 : 14;
                item.card.title = timeAgo(list[i].time);
                item.card.titleColor = 8;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& key) {
            vector<Notification> list = loadList();
            size_t index = (size_t)atoi(key.c_str());

            if (index < list.size()) {
                openNotification(myId, list[index]);
            }
        }
    );
}
// Menu thông báo
void notificationMenu(const string& username) {
    UserProfile me;

    if (!findProfile(username, me)) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    while (true) {
        int unread = countUnreadNotifications(me.id);

        vector<string> items = {
            "Xem Thông Báo (" + to_string(unread) + " chưa đọc)",
            "Đánh Dấu Tất Cả Đã Đọc",
            "Xóa Tất Cả Thông Báo",
            "Quay Lại"
        };

        int choice = menu(
            "=============== THÔNG BÁO ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                notificationListScreen(me.id);
                break;

            case 2:
                markAllNotificationsRead(me.id);
                notify("Đã đánh dấu tất cả thông báo là đã đọc!");
                break;

            case 3:
                if (confirmScreen("Xóa tất cả thông báo?")) {
                    clearNotifications(me.id);
                    notify("Đã xóa tất cả thông báo!");
                }
                break;

            default:
                return;
        }
    }
}

// Module: Xem trang cá nhân người khác, danh sách bạn bè, lời mời
#pragma once

/*
    XEM TRANG CÁ NHÂN CỦA NGƯỜI KHÁC VÀ CÁC THAO TÁC BẠN BÈ
                                                                */

string relationLabel(const string& status) {
    if (status == "FRIEND")             return "Bạn bè";
    if (status == "PENDING_SENT")       return "Bạn đã gửi lời mời kết bạn";
    if (status == "PENDING_RECEIVED")   return "Người này đã gửi lời mời cho bạn";
    if (status == "BLOCK")              return "Bạn đã chặn người này";
    return "Chưa kết bạn";
}
// Thẻ hiển thị 1 người: tên / giới thiệu (bio) / số bạn bè (hoặc số bạn chung với tôi).
// extra: chữ thêm vào cuối dòng tên (lý do gợi ý...). key rỗng nếu tài khoản không còn tồn tại
CardItem makePersonItem(const string& myId, const string& id, bool showMutual, const string& extra = "") {
    CardItem item;
    UserProfile profile;

    if (!findProfileById(id, profile)) {
        return item;
    }

    string name = profile.fullName.empty() ? profile.username : profile.fullName;

    item.key = id;
    item.card.head = name + (extra.empty() ? "" : " · " + extra);
    item.card.title = profile.bio.empty() ? "(chưa có giới thiệu)" : oneLine(profile.bio);
    item.card.titleColor = profile.bio.empty() ? 8 : 7;
    item.card.body = {showMutual ? to_string(countMutualFriends(myId, id)) + " bạn chung"
                                 : to_string(countFriends(id)) + " bạn bè"};
    item.card.bodyColor = 3;
    item.searchText = name + " " + profile.username;
    return item;
}
// Danh sách người dùng dạng thẻ (5 người đầu, M xem thêm, A xem tất cả, S tìm kiếm).
// idsOf() trả về danh sách ID cần hiển thị; chọn 1 người thì xem trang cá nhân
void personListScreen(
    const string& myId,
    const string& title,
    const string& emptyMessage,
    const string& selectHint,
    const function<vector<string>()>& idsOf,
    bool showMutual = false
) {
    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = selectHint;
    config.noun = "người";
    config.moreWord = "người nữa";
    config.pageSize = 5;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const string& id : idsOf()) {
                CardItem item = makePersonItem(myId, id, showMutual);

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
// Danh sách bạn chung giữa tôi và 1 người. Chọn 1 người để xem trang cá nhân
void mutualFriendsScreen(const string& myId, const string& otherId) {
    personListScreen(
        myId,
        "=============== BẠN CHUNG VỚI " + getDisplayName(otherId) + " ===============",
        "Hai bạn không có bạn chung nào.",
        "Xem trang cá nhân",
        [&]() {
            vector<string> ids;

            for (const string& id : getMutualFriendIds(myId, otherId)) {
                UserProfile profile;

                // Bỏ qua người đã xóa, tài khoản bị khóa hoặc người đã chặn mình
                if (findProfileById(id, profile) && profile.status != "LOCKED" &&
                    getRelation(id, myId) != "BLOCK") {
                    ids.push_back(id);
                }
            }

            return ids;
        }
    );
}
// Xem trang cá nhân của 1 người (myId: ID của tôi). Các nút thay đổi theo quan hệ hiện tại
void viewUserProfile(const string& myId, const string& otherId) {
    while (true) {
        UserProfile profile;

        if (!findProfileById(otherId, profile)) {
            notify("Không đọc được thông tin người này!");
            return;
        }

        string status = getRelation(myId, otherId);

        // Người kia đã chặn mình thì không được xem
        if (status != "BLOCK" && getRelation(otherId, myId) == "BLOCK") {
            notify("Bạn không thể xem trang cá nhân của người này!");
            return;
        }

        int mutualCount = countMutualFriends(myId, otherId);

        vector<pair<string, string>> info =
            buildProfileInfo(profile, mutualCount);
        info.push_back({"Quan Hệ", relationLabel(status)});

        vector<string> items, actions;
        auto add = [&](const string& label, const string& action) {
            items.push_back(label);
            actions.push_back(action);
        };

        if (status == "BLOCK") {
            add("Bỏ Chặn", "unblock");
        }
        else {
            if (status == "FRIEND") {
                add("Nhắn Tin", "chat");
            }
            else if (status == "PENDING_SENT") {
                add("Hủy Lời Mời Đã Gửi", "cancel");
            }
            else if (status == "PENDING_RECEIVED") {
                add("Chấp Nhận Lời Mời", "accept");
                add("Từ Chối Lời Mời", "decline");
            }
            else {
                add("Gửi Lời Mời Kết Bạn", "request");
            }

            add("Xem Bài Đăng", "posts");

            if (mutualCount > 0) {
                add("Xem Bạn Chung (" + to_string(mutualCount) + ")", "mutual");
            }

            if (status == "FRIEND") {
                add("Hủy Kết Bạn", "unfriend");
            }

            add("Báo Cáo Người Dùng", "report");
            add("Chặn", "block");
        }

        add("Quay Lại", "back");

        int choice = menuWithInfo(
            "=============== TRANG CÁ NHÂN ===============",
            info,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        if (choice == 0) {
            return;
        }

        const string& action = actions[choice - 1];
        string name = getDisplayName(otherId);

        if (action == "back") {
            return;
        }
        else if (action == "chat") {
            openPrivateChat(myId, otherId);
        }
        else if (action == "request") {
            notify(sendFriendRequest(myId, otherId));
        }
        else if (action == "cancel") {
            removeRelationBoth(myId, otherId);
            notify("Đã hủy lời mời kết bạn!");
        }
        else if (action == "accept") {
            if (acceptFriendRequest(myId, otherId)) {
                notify("Hai bạn đã trở thành bạn bè!");
            }
        }
        else if (action == "decline") {
            removeRelationBoth(myId, otherId);
            notify("Đã từ chối lời mời kết bạn!");
        }
        else if (action == "posts") {
            viewUserPosts(myId, otherId);
        }
        else if (action == "mutual") {
            mutualFriendsScreen(myId, otherId);
        }
        else if (action == "unfriend") {
            if (confirmScreen("Bạn có chắc muốn hủy kết bạn với " + name + "?")) {
                removeRelationBoth(myId, otherId);
                notify("Đã hủy kết bạn!");
            }
        }
        else if (action == "block") {
            if (confirmScreen("Chặn " + name + "? Hai bạn sẽ không thấy nhau nữa.")) {
                blockUser(myId, otherId);
                notify("Đã chặn người này!");
            }
        }
        else if (action == "report") {
            reportUserFlow(myId, otherId);
        }
        else if (action == "unblock") {
            unblockUser(myId, otherId);
            notify("Đã bỏ chặn người này!");
        }
    }
}
// Xem trang cá nhân của 1 người bạn (myId: ID của tôi, dùng để tính số bạn chung)
void viewFriendProfile(const string& myId, const string& friendId) {
    viewUserProfile(myId, friendId);
}

// Xem danh sách bạn bè của tôi. showMutual = true: dòng cuối của mỗi người là số bạn chung với tôi,
// false: số bạn bè của người đó
void viewFriends(const string& username, bool showMutual = false) {
    UserProfile me;

    if (!findProfile(username, me)) {
        printCenter("Không đọc được thông tin tài khoản!");
        sleep_for(2s);
        return;
    }

    personListScreen(
        me.id,
        "=============== DANH SÁCH BẠN BÈ CỦA (" + username + ") ===============",
        getNoFriendMessage(),
        "Xem trang cá nhân",
        [&]() { return getFriendIds(me.id); },
        showMutual
    );
}
/*
    MENU BẠN BÈ
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

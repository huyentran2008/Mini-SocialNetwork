// Module: Tìm kiếm, danh sách chặn, menu Bạn bè / Bài đăng / Nhắn tin
#pragma once

// Tìm người dùng theo tên hoặc tài khoản (không phân biệt hoa thường và dấu tiếng Việt).
// Gõ tới đâu tìm tới đó (không cần nhấn ENTER). Mỗi người hiển thị dạng thẻ 3 dòng: tên, bio, số bạn bè
void searchUsersScreen(const string& myId) {
    const string title = "=============== TÌM KIẾM NGƯỜI DÙNG ===============";
    const int MAX_RESULTS = 20;

    string query;
    int selected = 0;
    ReaderState state;
    state.viewHeight = 20;

    vector<UserProfile> results;
    int totalMatches = 0;

    // Tìm lại theo từ khóa hiện tại
    auto refresh = [&]() {
        results.clear();
        totalMatches = 0;

        string key = foldText(trimString(query));

        if (key.empty()) {
            return;
        }

        vector<UserProfile> matched;

        for (const string& id : listAccountIds()) {
            if (id == myId) {
                continue;
            }

            UserProfile profile;

            if (!findProfileById(id, profile)) {
                continue;
            }

            // Người đã chặn mình, hoặc tài khoản đang bị khóa thì không hiện ra
            if (getRelation(id, myId) == "BLOCK" || profile.status == "LOCKED") {
                continue;
            }

            if (foldText(profile.fullName).find(key) != string::npos ||
                foldText(profile.username).find(key) != string::npos ||
                foldText(profile.id).find(key) != string::npos) {
                matched.push_back(profile);
            }
        }

        // Tên bắt đầu bằng từ khóa xếp trước, sau đó theo tên
        sort(matched.begin(), matched.end(), [&](const UserProfile& a, const UserProfile& b) {
            string na = foldText(a.fullName.empty() ? a.username : a.fullName);
            string nb = foldText(b.fullName.empty() ? b.username : b.fullName);
            bool sa = na.compare(0, key.size(), key) == 0;
            bool sb = nb.compare(0, key.size(), key) == 0;

            if (sa != sb) return sa;
            return na < nb;
        });

        totalMatches = (int)matched.size();

        if ((int)matched.size() > MAX_RESULTS) {
            matched.resize(MAX_RESULTS);
        }

        results = matched;
    };

    auto drawAll = [&]() {
        int count = (int)results.size();

        if (selected >= count) selected = count - 1;
        if (selected < 0) selected = 0;

        int blockWidth = computeReaderBlockWidth(getConsoleWidth());

        // Mỗi người: tên, bio, số lượng bạn bè
        vector<Card> cards;

        for (const UserProfile& profile : results) {
            Card card;
            card.head = (profile.fullName.empty() ? profile.username : profile.fullName)
                      + " (@" + profile.username + ")";

            string status = getRelation(myId, profile.id);

            if (status == "FRIEND")                 card.head += " [Bạn bè]";
            else if (status == "PENDING_SENT")      card.head += " [Đã gửi lời mời]";
            else if (status == "PENDING_RECEIVED")  card.head += " [Chờ bạn chấp nhận]";
            else if (status == "BLOCK")             card.head += " [Đã chặn]";

            card.title = profile.bio.empty() ? "(chưa có giới thiệu)" : oneLine(profile.bio);
            card.titleColor = profile.bio.empty() ? 8 : 7;
            card.body = {to_string(countFriends(profile.id)) + " bạn bè"};
            card.bodyColor = 3;
            cards.push_back(card);
        }

        vector<TextBlock> blocks;
        vector<int> startLine, endLine;
        buildCardBlocks(cards, selected, blockWidth, blocks, startLine, endLine);

        // Dòng thông báo ở cuối danh sách
        string notice;
        int noticeColor = 8;

        if (trimString(query).empty()) {
            notice = "Gõ tên hoặc tài khoản để bắt đầu tìm kiếm (không cần nhấn ENTER)";
        }
        else if (count == 0) {
            notice = "Không tìm thấy người dùng nào phù hợp";
            noticeColor = 12;
        }
        else if (totalMatches > count) {
            notice = "── Hiển thị " + to_string(count) + "/" + to_string(totalMatches)
                   + " kết quả, hãy gõ thêm để thu hẹp ──";
            noticeColor = 14;
        }
        else {
            notice = "── Tìm thấy " + to_string(totalMatches) + " người dùng ──";
        }

        blocks.push_back(makeCenteredNotice(notice, noticeColor, blockWidth));
        int lastLine = (int)blocks.size() - 1;

        vector<BottomLine> bottom = {makeSearchLine(query)};
        string footer = "Gõ để tìm | ▲ / ▼ / Cuộn chuột: Chọn | ENTER: Xem trang cá nhân | ESC: Quay lại";

        auto ensureVisible = [&]() {
            if (count == 0) {
                state.offset = 0;
                return;
            }

            int e = (selected == count - 1) ? lastLine : endLine[selected];
            ensureLinesVisible(state, startLine[selected], e);
        };

        ensureVisible();
        drawReaderFrame(title, blocks, state, bottom, footer);

        int oldOffset = state.offset;
        ensureVisible();

        if (state.offset != oldOffset) {
            drawReaderFrame(title, blocks, state, bottom, footer);
        }
    };

    runLoop(
        drawAll,
        [&](const KeyInput& key) -> bool {
            int count = (int)results.size();

            switch (key.type) {
                case KEY_UP:          selected -= 1;  break;
                case KEY_DOWN:        selected += 1;  break;
                case KEY_SCROLL_UP:   selected -= 2;  break;
                case KEY_SCROLL_DOWN: selected += 2;  break;
                case KEY_PAGE_UP:     selected -= 5;  break;
                case KEY_PAGE_DOWN:   selected += 5;  break;
                case KEY_HOME:        selected = 0;   break;
                case KEY_END:         selected = count - 1; break;

                case KEY_ESCAPE:
                    return true;

                case KEY_ENTER:
                    if (count > 0) {
                        viewUserProfile(myId, results[max(0, min(selected, count - 1))].id);
                        refresh();      // Quan hệ có thể đã đổi (kết bạn, chặn...)
                    }

                    return false;

                case KEY_BACKSPACE:
                case KEY_CHARACTER:
                    if (editTextKey(key, query, 50)) {
                        selected = 0;
                        refresh();      // Gõ tới đâu tìm tới đó
                    }

                    return false;

                default:
                    return false;
            }

            if (selected >= count) selected = count - 1;
            if (selected < 0) selected = 0;

            return false;
        }
    );

    hideCursor();
    setColor(7);
}
// Danh sách những người tôi đã chặn
void blockedListScreen(const string& myId) {
    personListScreen(
        myId,
        "=============== DANH SÁCH ĐÃ CHẶN ===============",
        "Bạn chưa chặn ai cả.",
        "Xem & bỏ chặn",
        [&]() { return getIdsByStatus(myId, "BLOCK"); }
    );
}
// Menu Bạn Bè
void friendMenu(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    while (true) {
        // Tính lại số lượng mỗi vòng để luôn đúng sau khi thao tác
        vector<string> items = {
            "Danh Sách Bạn Bè (" + to_string(getFriendIds(myId).size()) + ")",
            "Lời Mời Kết Bạn Đã Nhận (" + to_string(getIdsByStatus(myId, "PENDING_RECEIVED").size()) + ")",
            "Lời Mời Đã Gửi (" + to_string(getIdsByStatus(myId, "PENDING_SENT").size()) + ")",
            "Gợi Ý Kết Bạn",
            "Tìm Kiếm Người Dùng",
            "Danh Sách Đã Chặn (" + to_string(getIdsByStatus(myId, "BLOCK").size()) + ")",
            "Quay Lại"
        };

        int choice = menu(
            "=============== BẠN BÈ ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1: viewFriends(username);              break;
            case 2: requestListScreen(myId, true);      break;
            case 3: requestListScreen(myId, false);     break;
            case 4: suggestionScreen(myId);             break;
            case 5: searchUsersScreen(myId);            break;
            case 6: blockedListScreen(myId);            break;
            default: return;    // "Quay Lại" (7) hoặc ESC (0)
        }
    }
}

/*
    MENU BÀI ĐĂNG
                    */

// Xem các bài đăng của tôi
void viewMyPosts(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    browsePosts(
        myId,
        "=============== BÀI ĐĂNG CỦA TÔI ===============",
        [&]() { return loadPosts(myId, myId); },
        false,
        "Bạn chưa có bài đăng nào. Hãy đăng bài đầu tiên của bạn nhé!"
    );
}
// Menu Bài Đăng
void postMenu(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    vector<string> items = {
        "Bảng Tin",
        "Đăng Bài Mới",
        "Bài Đăng Của Tôi",
        "Quay Lại"
    };

    while (true) {
        int choice = menu(
            "=============== BÀI ĐĂNG ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                browsePosts(
                    myId,
                    "=============== BẢNG TIN ===============",
                    [&]() { return loadPosts(myId, ""); },
                    true,
                    "Chưa có bài đăng nào để hiển thị. Hãy đăng bài hoặc kết bạn thêm nhé!"
                );
                break;

            case 2:
                createPost(myId);
                break;

            case 3:
                viewMyPosts(username);
                break;

            default:
                return;     // "Quay Lại" (4) hoặc ESC (0)
        }
    }
}

/*
    MENU NHẮN TIN
                    */

void messageMenu(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    vector<string> items = {
        "Tin Nhắn Riêng",
        "Nhóm Chat",
        "Tạo Nhóm Mới",
        "Quay Lại"
    };

    while (true) {
        int choice = menu(
            "=============== NHẮN TIN ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1: privateChatListScreen(myId);    break;
            case 2: groupListScreen(myId);          break;
            case 3: createGroupFlow(myId);          break;
            default: return;    // "Quay Lại" (4) hoặc ESC (0)
        }
    }
}


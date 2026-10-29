// Module: Quản trị viên
#pragma once

/*
    QUẢN TRỊ VIÊN
    Đăng nhập ở cùng màn hình đăng nhập của người dùng (tài khoản ADMIN_USERNAME / ADMIN_PASSWORD
    khai báo trong AccountCore.h), nhập đúng thì vào menu quản trị.

    Chức năng:
        - Tài khoản: xem số lượng, in danh sách, tìm theo ACC00000x / số ID / tài khoản / tên,
          xem hồ sơ, khóa, mở khóa, xóa
        - Báo cáo của người dùng (ACC000001 report ACC000002: lý do...)
        - Báo cáo sự cố tài khoản (quên mật khẩu): đặt lại mật khẩu, khóa / mở khóa, xem hồ sơ
        - Bài viết: xem, ẩn / hiện lại, xóa
        - Nhóm chat: xem thông tin, thành viên, xóa nhóm vi phạm (không xem được nội dung tin nhắn, để đảm bảo riêng tư)
        - Thống kê hệ thống
                                                                                            */

// Khai báo trước các màn hình gọi chéo nhau
void adminProfileScreen(const string& accountId);
void adminPostScreen(const string& postId);
void adminPostListScreen(const string& heading, const string& authorFilter, bool onlyHidden);
// statusFilter: rỗng = mọi trạng thái, "PENDING" = chưa xử lý, "DONE" = đã xử lý
void adminReportListScreen(const string& heading, const string& typeFilter, const string& targetFilter,
                           const string& statusFilter = "");

/*
    TIỆN ÍCH
                */

// Đọc hồ sơ của mọi tài khoản (đã sắp xếp theo ID)
vector<UserProfile> loadAllProfiles() {
    vector<UserProfile> list;

    for (const string& id : listAccountIds()) {
        UserProfile profile;

        if (findProfileById(id, profile)) {
            list.push_back(profile);
        }
    }

    return list;
}
string accountStatusLabel(const UserProfile& profile) {
    return profile.status == "LOCKED" ? "Đã khóa" : "Hoạt động";
}
string profileName(const UserProfile& profile) {
    return profile.fullName.empty() ? profile.username : profile.fullName;
}
// Đặt trạng thái khóa / mở khóa cho 1 tài khoản
bool setAccountLocked(const string& accountId, bool locked) {
    return updateProfileField(accountId, "status", locked ? "LOCKED" : "ACTIVE");
}
// Số bài đăng của 1 tài khoản (kể cả bài riêng tư và bài bị ẩn)
int countPostsOf(const string& accountId) {
    int count = 0;

    for (const string& id : listPostIds()) {
        Post post;

        if (readPost(id, post) && post.authorId == accountId) {
            count++;
        }
    }

    return count;
}
// Số báo cáo nhắm vào 1 tài khoản hoặc 1 bài viết
int countReportsAbout(const string& targetId) {
    int count = 0;

    for (const Report& report : readReports()) {
        if (report.targetId == targetId) {
            count++;
        }
    }

    return count;
}
/*
    QUẢN LÝ TÀI KHOẢN
                        */

// Danh sách tài khoản dạng thẻ 2 dòng: ACC00000x - tên người dùng - tên đăng nhập / trạng thái
// (xanh: hoạt động, đỏ: đã khóa). Hiện 10 tài khoản đầu, M xem thêm, A xem tất cả,
// S tìm kiếm theo ACC00000x, tên người dùng hoặc tên đăng nhập (gõ tới đâu tìm tới đó, không cần ENTER).
// Chọn 1 tài khoản để xem hồ sơ
void adminAccountListScreen() {
    CardListConfig config;
    config.title = "=============== DANH SÁCH TÀI KHOẢN ===============";
    config.emptyMessage = "Hệ thống chưa có tài khoản nào.";
    config.selectHint = "Xem hồ sơ";
    config.noun = "tài khoản";
    config.moreWord = "tài khoản nữa";
    config.pageSize = 10;
    config.searchable = true;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const UserProfile& profile : loadAllProfiles()) {
                bool locked = (profile.status == "LOCKED");

                CardItem item;
                item.key = profile.id;
                item.card.head = profile.id + " · " + profileName(profile) + " (@" + profile.username + ")";
                item.card.title = "Trạng thái: " + accountStatusLabel(profile);
                item.card.titleColor = locked ? 12 : 10;
                item.searchText = profile.id + " " + profile.fullName + " " + profile.username;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& accountId) {
            adminProfileScreen(accountId);
        }
    );
}
// Hồ sơ của 1 tài khoản: xem thông tin, khóa / mở khóa, xóa, xem bài đăng và báo cáo
void adminProfileScreen(const string& accountId) {
    while (true) {
        UserProfile profile;

        if (!findProfileById(accountId, profile)) {
            notify("Không đọc được thông tin tài khoản này!");
            return;
        }

        bool locked = (profile.status == "LOCKED");

        vector<pair<string, string>> info = {
            {"ID", profile.id},
            {"Tài Khoản", profile.username},
            {"Trạng Thái", accountStatusLabel(profile)}
        };

        for (const auto& line : buildProfileInfo(profile, -1, true)) {
            info.push_back(line);
        }

        info.push_back({"Số Bài Đăng", to_string(countPostsOf(profile.id))});
        info.push_back({"Báo Cáo Nhận Được", to_string(countReportsAbout(profile.id))});

        vector<string> items = {
            locked ? "Mở Khóa Tài Khoản" : "Khóa Tài Khoản",
            "Xóa Tài Khoản",
            "Xem Bài Đăng Của Người Này",
            "Xem Báo Cáo Về Người Này",
            "Quay Lại"
        };

        int choice = menuWithInfo(
            "=============== HỒ SƠ TÀI KHOẢN ===============",
            info,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        string name = profileName(profile);

        if (choice == 0 || choice == 5) {
            return;
        }

        if (choice == 1) {
            string question = locked
                ? "Mở khóa tài khoản " + name + "?"
                : "Khóa tài khoản " + name + "? Người này sẽ không đăng nhập được.";

            if (confirmScreen(question)) {
                if (setAccountLocked(profile.id, !locked)) {
                    notify(locked ? "Đã mở khóa tài khoản!" : "Đã khóa tài khoản!");
                }
                else {
                    notify("Không thể cập nhật tài khoản, vui lòng thử lại!");
                }
            }
        }
        else if (choice == 2) {
            if (confirmScreen("Xóa vĩnh viễn tài khoản " + name + " và toàn bộ dữ liệu?")) {
                deleteAccountData(profile.id);
                notify("Đã xóa tài khoản!");
                return;
            }
        }
        else if (choice == 3) {
            adminPostListScreen("BÀI ĐĂNG CỦA " + name, profile.id, false);
        }
        else if (choice == 4) {
            adminReportListScreen("BÁO CÁO VỀ " + name, REPORT_USER, profile.id);
        }
    }
}

/*
    BÁO CÁO CỦA NGƯỜI DÙNG
                            */

// Dòng hiển thị 1 báo cáo: ACC000001 report ACC000002: lý do...
string reportLine(const Report& report) {
    return report.reporterId + " report " + report.targetId + ": " + oneLine(report.reason);
}
// Dòng trong danh sách: báo cáo đã xử lý có thêm dấu [Đã xử lý] ở đầu
string reportListLine(const Report& report) {
    return (report.handled ? "[Đã xử lý] " : "") + reportLine(report);
}
// Chi tiết 1 báo cáo. Trả về true nếu danh sách cần nạp lại (đã xóa báo cáo hoặc đổi trạng thái)
bool adminReportDetail(const Report& report) {
    bool isUserReport = (report.type == REPORT_USER);

    ReaderState state;
    vector<TextBlock> blocks;

    blocks.push_back(makeBlock(reportLine(report), 14));
    blocks.push_back(makeBlock(report.handled ? "Trạng thái   : Đã xử lý" : "Trạng thái   : Chưa xử lý",
                               report.handled ? 10 : 12));
    blocks.push_back(makeBlock("", 7));
    blocks.push_back(makeBlock("Loại báo cáo : " + string(isUserReport ? "Báo cáo người dùng"
                                                                       : "Báo cáo bài viết"), 15));
    blocks.push_back(makeBlock("Thời gian    : " + report.time, 15));
    blocks.push_back(makeBlock("Người báo cáo: " + getDisplayName(report.reporterId)
                               + " (" + report.reporterId + ")", 15));

    if (isUserReport) {
        blocks.push_back(makeBlock("Bị báo cáo   : " + getDisplayName(report.targetId)
                                   + " (" + report.targetId + ")", 15));
    }
    else {
        Post post;

        if (readPost(report.targetId, post)) {
            blocks.push_back(makeBlock("Bài viết     : " + report.targetId + " - "
                                       + displayTitle(post, 60), 15));
            blocks.push_back(makeBlock("Tác giả      : " + getDisplayName(post.authorId)
                                       + " (" + post.authorId + ")", 15));
        }
        else {
            blocks.push_back(makeBlock("Bài viết     : " + report.targetId
                                       + " (bài viết đã bị xóa)", 12));
        }
    }

    blocks.push_back(makeBlock("", 7));
    blocks.push_back(makeBlock("Lý do:", 3));
    blocks.push_back(makeBlock(report.reason, 15, 0, 2));

    string footer = "Cuộn chuột / ▲ / ▼: Đọc | V: Xem "
                  + string(isUserReport ? "người bị báo cáo" : "bài viết bị báo cáo")
                  + " | B: Xem người báo cáo | H: Đánh dấu " + string(report.handled ? "chưa xử lý" : "đã xử lý")
                  + " | X: Xóa báo cáo | ESC: Quay lại";

    while (true) {
        int action = runReader("=============== CHI TIẾT BÁO CÁO ===============",
                               blocks, state, footer, "VBHX");

        if (action == 0) {
            return false;
        }

        if (action == 'V') {
            if (isUserReport) {
                adminProfileScreen(report.targetId);
            }
            else {
                Post post;

                if (readPost(report.targetId, post)) {
                    adminPostScreen(report.targetId);
                }
                else {
                    notify("Bài viết này đã bị xóa!");
                }
            }
        }
        else if (action == 'B') {
            adminProfileScreen(report.reporterId);
        }
        else if (action == 'H') {
            // Đổi trạng thái rồi quay lại danh sách (danh sách tự nạp lại)
            if (setReportHandled(report, !report.handled)) {
                notify(report.handled ? "Đã đánh dấu là chưa xử lý!" : "Đã đánh dấu là đã xử lý!");
                return true;
            }

            notify("Không thể cập nhật báo cáo, vui lòng thử lại!");
        }
        else if (action == 'X') {
            if (confirmScreen("Xóa hẳn báo cáo này?")) {
                removeReport(report);
                notify("Đã xóa báo cáo!");
                return true;
            }
        }
    }
}
// Danh sách báo cáo, mới nhất ở trên.
// Các báo cáo theo bộ lọc, mới nhất ở trên.
// typeFilter: rỗng = mọi loại, REPORT_USER hoặc REPORT_POST. targetFilter: rỗng = mọi đối tượng.
// statusFilter: rỗng = mọi trạng thái, "PENDING" = chưa xử lý, "DONE" = đã xử lý
vector<Report> filterReports(const string& typeFilter, const string& targetFilter, const string& statusFilter) {
    vector<Report> reports;

    for (const Report& report : readReports()) {
        if (!typeFilter.empty() && report.type != typeFilter) continue;
        if (!targetFilter.empty() && report.targetId != targetFilter) continue;
        if (statusFilter == "PENDING" && report.handled) continue;
        if (statusFilter == "DONE" && !report.handled) continue;
        reports.push_back(report);
    }

    reverse(reports.begin(), reports.end());
    return reports;
}
// Danh sách báo cáo dạng thẻ nhiều dòng: ACC000001 report ACC000002 (trạng thái) / lý do / người báo cáo - bị báo cáo / thời gian.
// Hiện vài báo cáo đầu, M xem thêm, A xem tất cả, S tìm kiếm (gõ tới đâu tìm tới đó)
void adminReportListScreen(const string& heading, const string& typeFilter, const string& targetFilter,
                           const string& statusFilter) {
    CardListConfig config;
    config.title = "=============== " + heading + " ===============";
    config.emptyMessage = "Chưa có báo cáo nào.";
    config.selectHint = "Xem chi tiết";
    config.noun = "báo cáo";
    config.moreWord = "báo cáo nữa";
    config.pageSize = 5;
    config.searchable = true;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;
            vector<Report> reports = filterReports(typeFilter, targetFilter, statusFilter);

            for (size_t i = 0; i < reports.size(); i++) {
                const Report& report = reports[i];
                bool isUser = (report.type == REPORT_USER);

                string targetText;

                if (isUser) {
                    targetText = getDisplayName(report.targetId) + " (" + report.targetId + ")";
                }
                else {
                    Post post;
                    targetText = readPost(report.targetId, post) ? "bài \"" + displayTitle(post, 40) + "\""
                                                                 : "bài viết đã bị xóa";
                }

                CardItem item;
                item.key = to_string(i);        // Vị trí trong danh sách đã lọc (mới nhất trước)
                item.card.head = report.reporterId + " report " + report.targetId
                               + " · " + string(isUser ? "Báo cáo người dùng" : "Báo cáo bài viết")
                               + " · " + string(report.handled ? "Đã xử lý" : "Chưa xử lý");
                item.card.headColor = report.handled ? 10 : 14;
                item.card.title = "Lý do: " + oneLine(report.reason);
                item.card.titleColor = 15;
                item.card.body = {
                    "Người báo cáo: " + getDisplayName(report.reporterId) + " (" + report.reporterId + ")",
                    "Bị báo cáo: " + targetText,
                    "Thời gian: " + report.time
                };
                item.card.bodyColor = 7;
                item.searchText = report.reporterId + " " + report.targetId + " " + report.reason + " "
                                + getDisplayName(report.reporterId) + " " + targetText;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& key) {
            vector<Report> reports = filterReports(typeFilter, targetFilter, statusFilter);
            size_t index = (size_t)atoi(key.c_str());

            if (index < reports.size()) {
                adminReportDetail(reports[index]);
            }
        }
    );
}
// Menu báo cáo: chọn loại / trạng thái báo cáo cần xem
void adminReportMenu() {
    while (true) {
        int pending = countPendingReports();

        vector<string> items = {
            "Tất Cả Báo Cáo",
            "Chưa Xử Lý (" + to_string(pending) + ")",
            "Đã Xử Lý",
            "Báo Cáo Người Dùng",
            "Báo Cáo Bài Viết",
            "Quay Lại"
        };

        int choice = menu(
            "=============== BÁO CÁO CỦA NGƯỜI DÙNG ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                adminReportListScreen("TẤT CẢ BÁO CÁO", "", "");
                break;

            case 2:
                adminReportListScreen("BÁO CÁO CHƯA XỬ LÝ", "", "", "PENDING");
                break;

            case 3:
                adminReportListScreen("BÁO CÁO ĐÃ XỬ LÝ", "", "", "DONE");
                break;

            case 4:
                adminReportListScreen("BÁO CÁO NGƯỜI DÙNG", REPORT_USER, "");
                break;

            case 5:
                adminReportListScreen("BÁO CÁO BÀI VIẾT", REPORT_POST, "");
                break;

            default:
                return;
        }
    }
}

/*
    QUẢN LÝ BÀI VIẾT
                        */

// Xem 1 bài viết (kể cả bài riêng tư / bị ẩn) và ẩn, hiện lại, xóa
void adminPostScreen(const string& postId) {
    ReaderState state;

    while (true) {
        Post post;

        if (!readPost(postId, post)) {
            notify("Bài viết không còn tồn tại!");
            return;
        }

        string authorName = getDisplayName(post.authorId);
        vector<string> likes = readLikes(postId);
        vector<PostComment> comments = readComments(postId);

        vector<Report> reports;

        for (const Report& report : readReports()) {
            if (report.type == REPORT_POST && report.targetId == postId) {
                reports.push_back(report);
            }
        }

        vector<TextBlock> blocks;

        string meta = post.createdAt;
        if (!post.editedAt.empty()) meta += " (đã chỉnh sửa)";
        meta += " · " + visibilityLabel(post.visibility);

        blocks.push_back(makeBlock(authorName + " (" + post.authorId + ")", 14));
        blocks.push_back(makeBlock(meta, 8));
        blocks.push_back(makeBlock(post.hidden ? "TRẠNG THÁI: ĐANG BỊ ẨN" : "Trạng thái: Đang hiển thị",
                                   post.hidden ? 12 : 10));
        blocks.push_back(makeBlock("", 7));

        if (!post.title.empty()) {
            blocks.push_back(makeBlock(post.title, 11));
            blocks.push_back(makeBlock("", 7));
        }

        blocks.push_back(makeBlock(post.content, 15));
        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock(
            "Lượt thích: " + to_string(likes.size())
            + "   |   Bình luận: " + to_string(comments.size())
            + "   |   Báo cáo: " + to_string(reports.size()), 12));

        if (!reports.empty()) {
            blocks.push_back(makeBlock("", 7));
            blocks.push_back(makeBlock("---------- Báo cáo về bài viết này ----------", 8, 1));

            for (const Report& report : reports) {
                blocks.push_back(makeBlock(reportLine(report), 12, 0, 2));
            }
        }

        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock("---------- Bình luận ----------", 8, 1));
        blocks.push_back(makeBlock("", 7));

        if (comments.empty()) {
            blocks.push_back(makeBlock("Chưa có bình luận nào.", 8, 1));
        }

        for (const PostComment& comment : comments) {
            blocks.push_back(makeBlock(
                getDisplayName(comment.authorId) + " · " + comment.time, 3));
            blocks.push_back(makeBlock(comment.text, 7, 0, 2));
            blocks.push_back(makeBlock("", 7));
        }

        string footer = string("Cuộn chuột / ▲ / ▼: Đọc | H: ")
                      + (post.hidden ? "Hiện lại bài" : "Ẩn bài")
                      + " | D: Xóa bài | P: Hồ sơ tác giả | ESC: Quay lại";

        int action = runReader("=============== BÀI VIẾT " + postId + " ===============",
                               blocks, state, footer, "HDP");

        if (action == 0) {
            return;
        }

        if (action == 'H') {
            post.hidden = !post.hidden;

            if (writePost(post)) {
                notify(post.hidden ? "Đã ẩn bài viết!" : "Đã hiện lại bài viết!");
            }
            else {
                notify("Không thể cập nhật bài viết, vui lòng thử lại!");
            }
        }
        else if (action == 'D') {
            if (confirmScreen("Xóa vĩnh viễn bài viết này?")) {
                deletePost(postId);
                notify("Đã xóa bài viết!");
                return;
            }
        }
        else if (action == 'P') {
            adminProfileScreen(post.authorId);
        }
    }
}
// Danh sách bài viết dạng thẻ (mới nhất ở trên): POSTxxxxxx - người đăng - thời gian / tiêu đề / nội dung rút gọn.
// authorFilter rỗng: mọi tác giả; onlyHidden: chỉ bài bị ẩn.
// Hiện vài bài đầu, M xem thêm, A xem tất cả, S tìm kiếm (gõ tới đâu tìm tới đó)
void adminPostListScreen(const string& heading, const string& authorFilter, bool onlyHidden) {
    CardListConfig config;
    config.title = "=============== " + heading + " ===============";
    config.emptyMessage = "Không có bài viết nào.";
    config.selectHint = "Xem bài viết";
    config.noun = "bài viết";
    config.moreWord = "bài viết nữa";
    config.pageSize = 5;
    config.searchable = true;

    runCardList(
        config,
        [&](int textWidth) {
            vector<Post> posts;

            for (const string& id : listPostIds()) {
                Post post;

                if (!readPost(id, post)) continue;
                if (!authorFilter.empty() && post.authorId != authorFilter) continue;
                if (onlyHidden && !post.hidden) continue;

                posts.push_back(post);
            }

            sort(posts.begin(), posts.end(), [](const Post& a, const Post& b) {
                if (a.createdAt != b.createdAt) return a.createdAt > b.createdAt;
                return a.id > b.id;
            });

            vector<CardItem> items;

            for (const Post& post : posts) {
                CardItem item;
                item.key = post.id;
                item.card = makePostCard(post, textWidth);
                item.card.head = post.id + " · " + item.card.head;
                item.card.headColor = post.hidden ? 12 : 14;        // Bài bị ẩn: dòng đầu màu đỏ
                item.searchText = post.id + " " + getDisplayName(post.authorId) + " " + post.authorId
                                + " " + post.title + " " + post.content;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& postId) {
            adminPostScreen(postId);
        }
    );
}
// Menu quản lý bài viết
void adminPostMenu() {
    vector<string> items = {
        "Tất Cả Bài Viết",
        "Bài Viết Đang Bị Ẩn",
        "Quay Lại"
    };

    while (true) {
        int choice = menu(
            "=============== QUẢN LÝ BÀI VIẾT ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                adminPostListScreen("TẤT CẢ BÀI VIẾT", "", false);
                break;

            case 2:
                adminPostListScreen("BÀI VIẾT ĐANG BỊ ẨN", "", true);
                break;

            default:
                return;
        }
    }
}

/*
    QUẢN LÝ NHÓM CHAT (quản trị viên không xem được nội dung tin nhắn của người dùng)
                                                                                    */

// Danh sách thành viên của 1 nhóm, chọn để xem hồ sơ
void adminGroupMembersScreen(const string& groupId) {
    int lastChoice = 1;

    while (true) {
        Group group;

        if (!readGroup(groupId, group)) {
            return;
        }

        vector<string> items;

        for (const GroupMember& member : group.members) {
            string item = getDisplayName(member.id) + " (" + member.id + ")";

            if (member.id == group.ownerId) {
                item += " - Chủ nhóm";
            }

            items.push_back(item);
        }

        if (items.empty()) {
            return;
        }

        int choice = scrollMenu(
            "=============== THÀNH VIÊN NHÓM " + group.name + " (" + to_string(items.size()) + ") ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Xem hồ sơ | ESC: Quay lại",
            lastChoice
        );

        if (choice == 0) {
            return;
        }

        lastChoice = choice;
        adminProfileScreen(group.members[choice - 1].id);
    }
}
// Xem 1 nhóm chat: thông tin, thành viên, xóa nhóm vi phạm (không xem được nội dung tin nhắn)
void adminGroupScreen(const string& groupId) {
    while (true) {
        Group group;

        if (!readGroup(groupId, group)) {
            notify("Nhóm không còn tồn tại!");
            return;
        }

        vector<pair<string, string>> info = {
            {"ID Nhóm", group.id},
            {"Tên Nhóm", group.name},
            {"Chủ Nhóm", getDisplayName(group.ownerId) + " (" + group.ownerId + ")"},
            {"Số Thành Viên", to_string(group.members.size())},
            {"Ngày Tạo", group.createdAt}
        };

        vector<string> items = {
            "Xem Danh Sách Thành Viên",
            "Xóa Nhóm Vi Phạm",
            "Quay Lại"
        };

        int choice = menuWithInfo(
            "=============== NHÓM CHAT ===============",
            info,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        if (choice == 0 || choice == 3) {
            return;
        }

        if (choice == 1) {
            adminGroupMembersScreen(groupId);
        }
        else if (choice == 2) {
            if (confirmScreen("Xóa nhóm \"" + cutText(group.name, 30) + "\"? Toàn bộ tin nhắn của nhóm sẽ mất!")) {
                error_code ec;
                fs::remove_all(groupDir(groupId), ec);
                notify("Đã xóa nhóm!");
                return;
            }
        }
    }
}
// Danh sách nhóm chat dạng thẻ: GRPxxxxxx - tên nhóm (số thành viên) / chủ nhóm / ngày tạo.
// Hiện vài nhóm đầu, M xem thêm, A xem tất cả, S tìm kiếm (gõ tới đâu tìm tới đó)
void adminGroupListScreen() {
    CardListConfig config;
    config.title = "=============== NHÓM CHAT ===============";
    config.emptyMessage = "Chưa có nhóm chat nào.";
    config.selectHint = "Xem nhóm";
    config.noun = "nhóm";
    config.moreWord = "nhóm nữa";
    config.pageSize = 5;
    config.searchable = true;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const string& id : listGroupIds()) {
                Group group;

                if (!readGroup(id, group)) continue;

                CardItem item;
                item.key = group.id;
                item.card.head = group.id + " · " + group.name + " (" + to_string(group.members.size()) + " thành viên)";
                item.card.title = "Chủ nhóm: " + getDisplayName(group.ownerId) + " (" + group.ownerId + ")";
                item.card.titleColor = 7;
                item.card.body = {"Ngày tạo: " + group.createdAt};
                item.card.bodyColor = 8;
                item.searchText = group.id + " " + group.name + " " + getDisplayName(group.ownerId) + " " + group.ownerId;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& groupId) {
            adminGroupScreen(groupId);
        }
    );
}

/*
    BÁO CÁO SỰ CỐ TÀI KHOẢN (người dùng quên mật khẩu, xin cấp lại)
                                                                    */

// Dòng hiển thị 1 báo cáo sự cố: ACC000012 (@username) quên mật khẩu: nội dung...
string issueLine(const AccountIssue& issue) {
    UserProfile profile;
    string who = issue.accountId;

    if (findProfileById(issue.accountId, profile)) {
        who += " (@" + profile.username + ")";
    }
    else {
        who += " (tài khoản đã bị xóa)";
    }

    return string(issue.handled ? "[Đã xử lý] " : "") + who + " quên mật khẩu: " + oneLine(issue.message);
}
// Quản trị viên đặt mật khẩu mới cho tài khoản. Trả về true nếu đã đặt thành công
bool adminResetPassword(const UserProfile& profile) {
    InputOptions options;
    options.title = "=============== ĐẶT LẠI MẬT KHẨU ===============";
    options.prompt = "Nhập mật khẩu mới cho " + profile.username
                   + " (8-15 ký tự, có chữ hoa, chữ thường, số, ký tự đặc biệt)";
    options.maxLength = 15;
    options.validate = [](const string& s) -> string {
        return validatePassword(s);
    };

    string newPassword;

    if (!inputText(options, newPassword)) {
        return false;
    }

    return updateProfileField(profile.id, "password", stringToHex(newPassword));
}
// Chi tiết 1 báo cáo sự cố: chỉnh sửa tài khoản (đặt lại mật khẩu, khóa / mở khóa), xem hồ sơ, xóa báo cáo
void adminIssueScreen(const string& accountId) {
    while (true) {
        AccountIssue issue;
        bool found = false;

        for (const AccountIssue& item : readAccountIssues()) {
            if (item.accountId == accountId) {
                issue = item;
                found = true;
            }
        }

        if (!found) {
            return;
        }

        UserProfile profile;

        if (!findProfileById(accountId, profile)) {
            removeAccountIssue(accountId);
            notify("Tài khoản này đã bị xóa nên báo cáo được gỡ bỏ!");
            return;
        }

        bool locked = (profile.status == "LOCKED");
        int infoWidth = max(20, getConsoleWidth() - 30);

        vector<pair<string, string>> info = {
            {"ID", profile.id},
            {"Tài Khoản", profile.username},
            {"Họ Tên", profileName(profile)},
            {"Trạng Thái", accountStatusLabel(profile)},
            {"Gửi Lúc", issue.time},
            {"Trạng Thái Báo Cáo", issue.handled ? "Đã xử lý" : "Chưa xử lý"},
            {"Nội Dung", cutText(oneLine(issue.message), infoWidth)}
        };

        vector<string> items = {
            "Đặt Lại Mật Khẩu",
            locked ? "Mở Khóa Tài Khoản" : "Khóa Tài Khoản",
            "Xem Hồ Sơ Tài Khoản",
            "Xem Nội Dung Báo Cáo Đầy Đủ",
            issue.handled ? "Đánh Dấu Chưa Xử Lý" : "Đánh Dấu Đã Xử Lý",
            "Xóa Báo Cáo Này",
            "Quay Lại"
        };

        int choice = menuWithInfo(
            "=============== SỰ CỐ TÀI KHOẢN ===============",
            info,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        if (choice == 0 || choice == 7) {
            return;
        }

        if (choice == 1) {
            if (adminResetPassword(profile)) {
                // Đặt lại mật khẩu xong thì báo cáo được đánh dấu đã xử lý (vẫn lưu lại để tra cứu)
                setAccountIssueHandled(accountId, true);
                notify("Đã đặt lại mật khẩu! Hãy thông báo mật khẩu mới cho người dùng.");
                return;
            }
        }
        else if (choice == 2) {
            string question = locked ? "Mở khóa tài khoản " + profileName(profile) + "?"
                                     : "Khóa tài khoản " + profileName(profile) + "?";

            if (confirmScreen(question)) {
                if (setAccountLocked(profile.id, !locked)) {
                    notify(locked ? "Đã mở khóa tài khoản!" : "Đã khóa tài khoản!");
                }
                else {
                    notify("Không thể cập nhật tài khoản, vui lòng thử lại!");
                }
            }
        }
        else if (choice == 3) {
            adminProfileScreen(accountId);
        }
        else if (choice == 4) {
            showMessageScreen("=============== NỘI DUNG BÁO CÁO ===============",
                              issue.message, "ENTER / ESC: Quay lại");
        }
        else if (choice == 5) {
            setAccountIssueHandled(accountId, !issue.handled);
            notify(issue.handled ? "Đã đánh dấu là chưa xử lý!" : "Đã đánh dấu là đã xử lý!");
        }
        else if (choice == 6) {
            if (confirmScreen("Xóa hẳn báo cáo này?")) {
                removeAccountIssue(accountId);
                notify("Đã xóa báo cáo!");
                return;
            }
        }
    }
}
// Danh sách báo cáo sự cố tài khoản dạng thẻ nhiều dòng: ACC00000x (@tài khoản) - trạng thái / nội dung / họ tên - thời gian.
// Hiện vài báo cáo đầu, M xem thêm, A xem tất cả, S tìm kiếm (gõ tới đâu tìm tới đó)
void adminIssueListScreen() {
    CardListConfig config;
    config.title = "=============== SỰ CỐ TÀI KHOẢN ===============";
    config.emptyMessage = "Chưa có báo cáo sự cố tài khoản nào.";
    config.selectHint = "Xem và chỉnh sửa tài khoản";
    config.noun = "báo cáo";
    config.moreWord = "báo cáo nữa";
    config.pageSize = 5;
    config.searchable = true;

    runCardList(
        config,
        [&](int) {
            vector<AccountIssue> list = readAccountIssues();
            reverse(list.begin(), list.end());      // Mới nhất ở trên

            vector<CardItem> items;

            for (const AccountIssue& issue : list) {
                UserProfile profile;
                bool exists = findProfileById(issue.accountId, profile);

                CardItem item;
                item.key = issue.accountId;
                item.card.head = issue.accountId + (exists ? " (@" + profile.username + ")" : " (tài khoản đã bị xóa)")
                               + " · Quên mật khẩu · " + string(issue.handled ? "Đã xử lý" : "Chưa xử lý");
                item.card.headColor = issue.handled ? 10 : 14;
                item.card.title = "Nội dung: " + oneLine(issue.message);
                item.card.titleColor = 15;
                item.card.body = {
                    "Họ tên: " + (exists ? profileName(profile) : string("(không còn)")),
                    "Gửi lúc: " + issue.time
                };
                item.card.bodyColor = 7;
                item.searchText = issue.accountId + " " + (exists ? profile.username + " " + profile.fullName : "")
                                + " " + issue.message;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& accountId) {
            adminIssueScreen(accountId);
        }
    );
}

// Thống kê số lượng tài khoản, bài viết, nhóm, báo cáo
void adminStatsScreen() {
    vector<UserProfile> profiles = loadAllProfiles();
    int locked = 0;

    for (const UserProfile& profile : profiles) {
        if (profile.status == "LOCKED") {
            locked++;
        }
    }

    int posts = 0;
    int hiddenPosts = 0;

    for (const string& id : listPostIds()) {
        Post post;

        if (!readPost(id, post)) {
            continue;
        }

        posts++;

        if (post.hidden) {
            hiddenPosts++;
        }
    }

    int userReports = 0;
    int postReports = 0;

    for (const Report& report : readReports()) {
        if (report.type == REPORT_USER) userReports++;
        else                            postReports++;
    }

    vector<pair<string, string>> info = {
        {"Tổng Số Tài Khoản", to_string(profiles.size())},
        {"Đang Hoạt Động", to_string((int)profiles.size() - locked)},
        {"Đã Bị Khóa", to_string(locked)},
        {"Tổng Số Bài Viết", to_string(posts)},
        {"Bài Viết Bị Ẩn", to_string(hiddenPosts)},
        {"Nhóm Chat", to_string(listGroupIds().size())},
        {"Báo Cáo Người Dùng", to_string(userReports)},
        {"Báo Cáo Bài Viết", to_string(postReports)},
        {"Báo Cáo Chưa Xử Lý", to_string(countPendingReports())},
        {"Sự Cố Tài Khoản Chưa Xử Lý", to_string(countPendingIssues())}
    };

    menuWithInfo(
        "=============== THỐNG KÊ HỆ THỐNG ===============",
        info,
        {"Quay Lại"},
        "ENTER / ESC: Quay lại"
    );
}
// Menu chính của quản trị viên. Trả về true nếu thoát chương trình, false nếu đăng xuất
bool menuAdmin(const string& username) {
    string title = "================= QUẢN TRỊ VIÊN (" + username + ") =================";

    while (true) {
        // Số báo cáo sự cố đang chờ xử lý hiện ngay trên menu
        int issues = countPendingIssues();

        vector<string> items = {
            "Quản Lý Tài Khoản (" + to_string(listAccountIds().size()) + ")",
            countPendingReports() > 0 ? "Báo Cáo Của Người Dùng (" + to_string(countPendingReports()) + " chưa xử lý)"
                                      : "Báo Cáo Của Người Dùng",
            issues > 0 ? "Sự Cố Tài Khoản (" + to_string(issues) + ")" : "Sự Cố Tài Khoản",
            "Quản Lý Bài Viết",
            "Quản Lý Nhóm Chat",
            "Thống Kê Hệ Thống",
            "Đăng Xuất",
            "Thoát Chương Trình"
        };

        int choice = menu(
            title,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn"
        );

        switch (choice) {
            case 1:
                adminAccountListScreen();
                break;

            case 2:
                adminReportMenu();
                break;

            case 3:
                adminIssueListScreen();
                break;

            case 4:
                adminPostMenu();
                break;

            case 5:
                adminGroupListScreen();
                break;

            case 6:
                adminStatsScreen();
                break;

            case 7:
                printCenter("Đã đăng xuất!");
                sleep_for(2s);
                return false;

            case 8:
                printCenter("Đã thoát chương trình!");
                sleep_for(3s);
                return true;

            default:
                // Nhấn ESC: ở lại menu, không làm gì
                break;
        }
    }
}

// Module: Bài đăng
#pragma once

/*
    BÀI ĐĂNG (Data/Post/POST<số>/ post.txt, likes.txt, comments.txt)
                                                                        */

struct Post {
    string id;
    string authorId;
    string visibility;      // PUBLIC, FRIEND, PRIVATE
    string createdAt;       // YYYY-MM-DD HH:MM:SS
    string editedAt;        // Rỗng nếu chưa chỉnh sửa
    string title;           // Tiêu đề (bài cũ có thể để trống)
    string content;
    bool hidden = false;    // true: bị quản trị viên ẩn (chỉ tác giả và quản trị viên xem được)
};
struct PostComment {
    string authorId;
    string time;
    string text;
};
fs::path postDir(const string& postId) {
    return fs::path("Data/Post") / postId;
}
string visibilityLabel(const string& visibility) {
    if (visibility == "PUBLIC") return "Công Khai";
    if (visibility == "FRIEND") return "Bạn Bè";
    return "Chỉ Mình Tôi";
}
// Đọc 1 bài đăng
bool readPost(const string& postId, Post& post) {
    error_code ec;
    fs::path file = postDir(postId) / "post.txt";

    if (!fs::is_regular_file(file, ec)) {
        return false;
    }

    Post current;
    current.id = postId;

    for (const string& line : readLines(file)) {
        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "author")            current.authorId = value;
        else if (key == "visibility")   current.visibility = value;
        else if (key == "created_at")   current.createdAt = value;
        else if (key == "edited_at")    current.editedAt = value;
        else if (key == "status")       current.hidden = (value == "HIDDEN");
        else if (key == "title")        current.title = unescapeText(value);
        else if (key == "content")      current.content = unescapeText(value);
    }

    if (current.authorId.empty()) {
        return false;
    }

    if (current.visibility.empty()) {
        current.visibility = "PRIVATE";
    }

    post = current;
    return true;
}
// Ghi 1 bài đăng
bool writePost(const Post& post) {
    error_code ec;
    fs::create_directories(postDir(post.id), ec);

    vector<string> lines = {
        "id=" + post.id,
        "author=" + post.authorId,
        "visibility=" + post.visibility,
        "created_at=" + post.createdAt,
        "edited_at=" + post.editedAt,
        "status=" + string(post.hidden ? "HIDDEN" : "VISIBLE"),
        "title=" + escapeText(post.title),
        "content=" + escapeText(post.content)
    };

    return writeLines(postDir(post.id) / "post.txt", lines);
}
// ID của mọi bài đăng
vector<string> listPostIds() {
    vector<string> ids;
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Post", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (name.size() > 4 && name.compare(0, 4, "POST") == 0) {
            ids.push_back(name);
        }
    }

    sort(ids.begin(), ids.end());
    return ids;
}
// Người xem có được xem bài đăng này không
bool canViewPost(const string& viewerId, const Post& post) {
    if (post.authorId == viewerId) {
        return true;
    }

    // Bài bị quản trị viên ẩn: người khác không xem được
    if (post.hidden) {
        return false;
    }

    if (isBlockedEither(viewerId, post.authorId)) {
        return false;
    }

    if (post.visibility == "PUBLIC") {
        return true;
    }

    if (post.visibility == "FRIEND") {
        return areFriends(viewerId, post.authorId);
    }

    return false;
}
// Danh sách ID đã thích bài đăng
vector<string> readLikes(const string& postId) {
    vector<string> ids;

    for (const string& line : readLines(postDir(postId) / "likes.txt")) {
        string id = trimString(line);

        if (!id.empty() && find(ids.begin(), ids.end(), id) == ids.end()) {
            ids.push_back(id);
        }
    }

    return ids;
}
// Thích / bỏ thích. Trả về true nếu sau khi bấm là đang thích
bool toggleLike(const string& postId, const string& accountId) {
    vector<string> likes = readLikes(postId);
    auto it = find(likes.begin(), likes.end(), accountId);
    bool liked;

    if (it != likes.end()) {
        likes.erase(it);
        liked = false;
    }
    else {
        likes.push_back(accountId);
        liked = true;
    }

    writeLines(postDir(postId) / "likes.txt", likes);

    // Có người thích bài thì báo cho tác giả
    if (liked) {
        Post post;

        if (readPost(postId, post)) {
            addNotification(post.authorId, NOTI_POST_LIKE, accountId, postId);
        }
    }

    return liked;
}
// Đọc các bình luận (mỗi dòng: ACCID|THỜI GIAN|NỘI DUNG)
vector<PostComment> readComments(const string& postId) {
    vector<PostComment> comments;

    for (const string& line : readLines(postDir(postId) / "comments.txt")) {
        size_t first = line.find('|');
        if (first == string::npos) continue;

        size_t second = line.find('|', first + 1);
        if (second == string::npos) continue;

        PostComment comment;
        comment.authorId = line.substr(0, first);
        comment.time = line.substr(first + 1, second - first - 1);
        comment.text = unescapeText(line.substr(second + 1));
        comments.push_back(comment);
    }

    return comments;
}
bool writeComments(const string& postId, const vector<PostComment>& comments) {
    vector<string> lines;

    for (const PostComment& comment : comments) {
        lines.push_back(comment.authorId + "|" + comment.time + "|" + escapeText(comment.text));
    }

    return writeLines(postDir(postId) / "comments.txt", lines);
}
bool addComment(const string& postId, const string& accountId, const string& text) {
    bool ok = appendLine(postDir(postId) / "comments.txt",
                         accountId + "|" + nowTimestamp() + "|" + escapeText(text));

    // Có người bình luận bài thì báo cho tác giả
    if (ok) {
        Post post;

        if (readPost(postId, post)) {
            addNotification(post.authorId, NOTI_POST_COMMENT, accountId, postId);
        }
    }

    return ok;
}
// Các bài đăng mà viewerId được xem, mới nhất ở trên.
// authorFilter không rỗng: chỉ lấy bài của người đó
vector<Post> loadPosts(const string& viewerId, const string& authorFilter) {
    vector<Post> posts;

    for (const string& id : listPostIds()) {
        Post post;

        if (!readPost(id, post)) {
            continue;
        }

        if (!authorFilter.empty() && post.authorId != authorFilter) {
            continue;
        }

        if (!canViewPost(viewerId, post)) {
            continue;
        }

        posts.push_back(post);
    }

    sort(posts.begin(), posts.end(), [](const Post& a, const Post& b) {
        if (a.createdAt != b.createdAt) return a.createdAt > b.createdAt;
        return a.id > b.id;
    });

    return posts;
}
// Nội dung xem trước (dòng đầu tiên) của bài đăng
string previewText(const string& content, int maxLen) {
    string firstLine = content.substr(0, content.find('\n'));
    return cutText(trimBlank(oneLine(firstLine)), maxLen);
}
// Tiêu đề hiển thị trong danh sách (bài cũ chưa có tiêu đề thì lấy dòng đầu của nội dung)
string displayTitle(const Post& post, int maxLen) {
    if (!post.title.empty()) {
        return cutText(oneLine(post.title), maxLen);
    }

    return previewText(post.content, maxLen);
}
// Chọn quyền riêng tư bằng menu. chosen = false nếu nhấn ESC
string chooseVisibility(const string& title, const string& footer, bool& chosen) {
    vector<string> items = {
        "Công Khai (mọi người đều xem được)",
        "Bạn Bè (chỉ bạn bè xem được)",
        "Chỉ Mình Tôi (riêng tư)"
    };

    int choice = menu(title, items, footer);
    chosen = (choice != 0);

    if (choice == 1) return "PUBLIC";
    if (choice == 2) return "FRIEND";
    return "PRIVATE";
}
// Đăng bài mới: nhập tiêu đề -> nội dung -> chọn quyền riêng tư
void createPost(const string& myId) {
    // Bước 1: tiêu đề
    InputOptions titleOptions;
    titleOptions.title = "=============== ĐĂNG BÀI MỚI ===============";
    titleOptions.prompt = "Nhập tiêu đề bài đăng (tối đa 100 ký tự)";
    titleOptions.maxLength = 100;
    titleOptions.validate = [](const string& s) -> string {
        return s.empty() ? "Tiêu đề bài đăng không được để trống!" : "";
    };

    string title;

    if (!inputText(titleOptions, title)) {
        return;
    }

    // Bước 2: nội dung
    InputOptions options;
    options.title = "=============== ĐĂNG BÀI MỚI ===============";
    options.prompt = "Tiêu đề: " + title + "\nBạn đang nghĩ gì? (tối đa 2000 ký tự)";
    options.maxLength = 2000;
    options.multiline = true;
    options.validate = [](const string& s) -> string {
        return s.empty() ? "Nội dung bài đăng không được để trống!" : "";
    };

    string content;

    if (!inputText(options, content)) {
        return;
    }

    // Bước 3: quyền riêng tư
    bool chosen = false;
    string visibility = chooseVisibility(
        "=============== AI ĐƯỢC XEM BÀI ĐĂNG NÀY? ===============",
        "Sử dụng ▲ / ▼ | ENTER: Đăng bài | ESC: Hủy đăng bài",
        chosen
    );

    if (!chosen) {
        notify("Đã hủy đăng bài!");
        return;
    }

    Post post;
    post.id = nextId("Data/Post", "POST");
    post.authorId = myId;
    post.visibility = visibility;
    post.createdAt = nowTimestamp();
    post.title = title;
    post.content = content;

    if (!writePost(post)) {
        notify("Không thể lưu bài đăng, vui lòng thử lại!");
        return;
    }

    notify("Đăng bài thành công!");
}
// Xóa hẳn 1 bài đăng
void deletePost(const string& postId) {
    error_code ec;
    fs::remove_all(postDir(postId), ec);
    removeReportsAbout(postId);     // Bài đã xóa thì các báo cáo về bài đó cũng hết ý nghĩa
}
// Đọc 1 bài đăng: tiêu đề cố định, nội dung (kèm bình luận) cuộn được, menu hướng dẫn cố định
void viewPost(const string& myId, const string& postId) {
    ReaderState state;

    while (true) {
        Post post;

        if (!readPost(postId, post) || !canViewPost(myId, post)) {
            notify("Bài đăng không còn tồn tại hoặc bạn không có quyền xem!");
            return;
        }

        bool mine = (post.authorId == myId);
        string authorName = getDisplayName(post.authorId);

        vector<string> likes = readLikes(postId);
        bool liked = find(likes.begin(), likes.end(), myId) != likes.end();
        vector<PostComment> comments = readComments(postId);

        // Nội dung cuộn được
        vector<TextBlock> blocks;

        string meta = timeAgo(post.createdAt);
        if (!post.editedAt.empty()) meta += " (đã chỉnh sửa)";
        meta += " · " + visibilityLabel(post.visibility);
        if (post.hidden) meta += " · Đã bị quản trị viên ẩn";

        blocks.push_back(makeBlock(authorName, 14));
        blocks.push_back(makeBlock(meta, 8));
        blocks.push_back(makeBlock("", 7));

        // Tiêu đề bài đăng (bài cũ chưa có tiêu đề thì bỏ qua)
        if (!post.title.empty()) {
            blocks.push_back(makeBlock(post.title, 11));
            blocks.push_back(makeBlock("", 7));
        }

        blocks.push_back(makeBlock(post.content, 15));
        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock(
            "Lượt thích: " + to_string(likes.size()) + (liked ? " (bạn đã thích)" : "")
            + "   |   Bình luận: " + to_string(comments.size()), 12));
        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock("---------- Bình luận ----------", 8, 1));
        blocks.push_back(makeBlock("", 7));

        if (comments.empty()) {
            blocks.push_back(makeBlock("Chưa có bình luận nào. Hãy là người đầu tiên!", 8, 1));
        }

        for (const PostComment& comment : comments) {
            blocks.push_back(makeBlock(
                getDisplayName(comment.authorId) + " · " + timeAgo(comment.time), 3));
            blocks.push_back(makeBlock(comment.text, 7, 0, 2));
            blocks.push_back(makeBlock("", 7));
        }

        string title = "=============== BÀI ĐĂNG CỦA " + authorName + " ===============";

        string footer = "Cuộn chuột / ▲ / ▼: Đọc | L: Thích | C: Bình luận";
        string actions = "LC";

        if (mine) {
            footer += " | E: Sửa nội dung | T: Sửa tiêu đề | V: Quyền riêng tư | D: Xóa";
            actions += "ETVD";
        }
        else {
            footer += " | P: Trang cá nhân | R: Báo cáo bài viết";
            actions += "PR";
        }

        footer += " | ESC: Quay lại";

        int action = runReader(title, blocks, state, footer, actions);

        if (action == 0) {
            return;
        }

        if (action == 'L') {
            toggleLike(postId, myId);
        }
        else if (action == 'C') {
            InputOptions options;
            options.title = "=============== BÌNH LUẬN ===============";
            options.prompt = "Viết bình luận của bạn (tối đa 300 ký tự)";
            options.maxLength = 300;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "Bình luận không được để trống!" : "";
            };

            string text;

            if (inputText(options, text)) {
                addComment(postId, myId, text);
                state.offset = INT_MAX / 2;     // Cuộn xuống để thấy bình luận mới
            }
        }
        else if (action == 'P') {
            viewUserProfile(myId, post.authorId);
        }
        else if (action == 'R') {
            reportPostFlow(myId, postId);
        }
        else if (action == 'T') {
            InputOptions options;
            options.title = "=============== SỬA TIÊU ĐỀ BÀI ĐĂNG ===============";
            options.prompt = "Chỉnh sửa tiêu đề bài đăng (tối đa 100 ký tự)";
            options.maxLength = 100;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "Tiêu đề bài đăng không được để trống!" : "";
            };

            string newTitle = post.title;

            if (inputText(options, newTitle)) {
                post.title = newTitle;
                post.editedAt = nowTimestamp();
                writePost(post);
            }
        }
        else if (action == 'E') {
            InputOptions options;
            options.title = "=============== SỬA BÀI ĐĂNG ===============";
            options.prompt = "Chỉnh sửa nội dung bài đăng (tối đa 2000 ký tự)";
            options.maxLength = 2000;
            options.multiline = true;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "Nội dung bài đăng không được để trống!" : "";
            };

            string content = post.content;

            if (inputText(options, content)) {
                post.content = content;
                post.editedAt = nowTimestamp();
                writePost(post);
            }
        }
        else if (action == 'V') {
            bool chosen = false;
            string visibility = chooseVisibility(
                "=============== ĐỔI QUYỀN RIÊNG TƯ ===============",
                "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Hủy",
                chosen
            );

            if (chosen) {
                post.visibility = visibility;
                writePost(post);
            }
        }
        else if (action == 'D') {
            if (confirmScreen("Bạn có chắc muốn xóa bài đăng này không?")) {
                deletePost(postId);
                notify("Đã xóa bài đăng!");
                return;
            }
        }
    }
}
// Danh sách bài đăng để chọn đọc. loader trả về các bài đăng cần hiển thị
const int POST_PAGE_SIZE = 5;       // Mỗi lần hiển thị / mở rộng thêm 5 bài
const int POST_CONTENT_LINES = 2;   // Nội dung mỗi bài chỉ in tối đa 2 dòng rồi "..."

// Thẻ hiển thị của 1 bài đăng: người đăng - thời gian / tiêu đề / nội dung rút gọn
Card makePostCard(const Post& post, int textWidth) {
    Card card;

    card.head = getDisplayName(post.authorId) + " · " + timeAgo(post.createdAt)
              + " · " + visibilityLabel(post.visibility);
    if (post.hidden) card.head += " · Đã bị ẩn";

    card.title = displayTitle(post, textWidth);

    // Nội dung: tối đa POST_CONTENT_LINES dòng, dài hơn thì thêm "..."
    string body = trimBlank(oneLine(post.content));
    if (body.empty()) body = "(không có nội dung)";

    vector<string> bodyLines = wrapText(body, textWidth);
    bool tooLong = (int)bodyLines.size() > POST_CONTENT_LINES;

    if (tooLong) {
        bodyLines.resize(POST_CONTENT_LINES);
        bodyLines.back() = cutChars(bodyLines.back(), textWidth - 3) + "...";
    }

    card.body = bodyLines;
    return card;
}
// Bài đăng có khớp từ khóa không (theo tên người đăng, tiêu đề, nội dung;
// không phân biệt hoa thường và dấu tiếng Việt). foldedKey đã được foldText sẵn
bool postMatchesQuery(const Post& post, const string& foldedKey) {
    return foldText(getDisplayName(post.authorId)).find(foldedKey) != string::npos ||
           foldText(post.title).find(foldedKey) != string::npos ||
           foldText(post.content).find(foldedKey) != string::npos;
}

// Danh sách bài đăng. Chỉ hiện vài bài mới nhất: M xem thêm, A xem tất cả, S tìm kiếm (gõ tới đâu lọc tới đó).
// Mỗi bài gồm 3 phần: người đăng - thời gian / tiêu đề / nội dung rút gọn, các bài cách nhau 1 dòng trống.
// Bài đang chọn được đổi màu nền. ▲ / ▼, con lăn chuột, PgUp / PgDn, Home / End để di chuyển
void browsePosts(
    const string& myId,
    const string& title,
    const function<vector<Post>()>& loader,
    bool showAuthor,
    const string& emptyMessage
) {
    (void)showAuthor;   // Luôn hiện tên người đăng ở dòng đầu của mỗi bài

    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = "Đọc bài";
    config.noun = "bài đăng";
    config.moreWord = "bài đăng cũ hơn";
    config.pageSize = POST_PAGE_SIZE;
    config.searchable = true;

    runCardList(
        config,
        [&](int textWidth) {
            vector<CardItem> items;

            for (const Post& post : loader()) {
                CardItem item;
                item.key = post.id;
                item.card = makePostCard(post, textWidth);
                // Tìm theo tên người đăng, tiêu đề, nội dung
                item.searchText = getDisplayName(post.authorId) + " " + post.title + " " + post.content;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& postId) {
            viewPost(myId, postId);
        }
    );
}
// Xem các bài đăng của 1 người mà mình được phép xem
void viewUserPosts(const string& myId, const string& otherId) {
    string title = "=============== BÀI ĐĂNG CỦA " + getDisplayName(otherId) + " ===============";

    browsePosts(
        myId,
        title,
        [&]() { return loadPosts(myId, otherId); },
        false,
        "Người này chưa có bài đăng nào mà bạn có thể xem."
    );
}


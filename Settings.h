// Module: Cài đặt tài khoản, trang cá nhân của tôi
#pragma once

/*
    CÀI ĐẶT TÀI KHOẢN
                        */

// Chỉnh sửa thông tin cá nhân: họ tên, ngày sinh, giới tính, giới thiệu, nơi ở hiện tại, quê quán,
// trường học, nơi làm việc, sở thích. Chọn ô nào thì gõ trực tiếp vào ô đó.
// Nơi ở / quê quán / trường học: ← / → để đổi nhanh, ENTER để mở danh sách (nhấn S trong danh sách để tìm kiếm).
// Sở thích: ENTER để mở danh sách và tích chọn tối đa MAX_HOBBIES sở thích. Các mục này có thể để trống
void editAccount(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    UserProfile profile;

    if (!findProfileById(myId, profile)) {
        notify("Không đọc được thông tin tài khoản!");
        return;
    }

    // Giá trị ban đầu (để biết người dùng đã sửa gì chưa)
    const string originalName = profile.fullName;
    const string originalBirthday = toDisplayDate(profile.birthday);
    const string originalGender = profile.gender;
    const string originalBio = profile.bio;
    const string originalResidence = profile.residence;
    const string originalHometown = profile.hometown;
    const string originalSchool = profile.school;
    const string originalWorkplace = profile.workplace;
    const vector<string> originalHobbies = splitList(profile.hobbies);

    RegisterForm form;
    form.fullName = originalName;
    form.birthday = originalBirthday;
    form.gender = originalGender;
    form.bio = originalBio;

    string residence = originalResidence;
    string hometown = originalHometown;
    string school = originalSchool;
    string workplace = originalWorkplace;
    vector<string> hobbies = originalHobbies;

    // 1 họ tên, 2 ngày sinh, 3 giới tính, 4 giới thiệu, 5 nơi ở, 6 quê quán, 7 trường học,
    // 8 nơi làm việc, 9 sở thích, 10 nút "Lưu Thay Đổi", 11 nút "Quay Lại"
    const int SAVE_BUTTON = 10;
    const int BACK_BUTTON = 11;
    int selected = 1;
    string errorMessage;

    // Kiểm tra 1 ô
    auto validateField = [&](int field) -> string {
        switch (field) {
            case 1: return validateFullName(form.fullName, myId);  // bỏ qua tên của chính mình
            case 2: return validateBirthday(form.birthday);
            case 3: return validateGender(form.gender);
            case 4: return validateBio(form.bio);
            case 8: return textLength(trimString(workplace)) > 100 ? "Nơi làm việc tối đa 100 ký tự!" : "";
            case 9: return (int)hobbies.size() > MAX_HOBBIES
                         ? "Chỉ được chọn tối đa " + to_string(MAX_HOBBIES) + " sở thích!" : "";
        }

        return "";
    };

    // Đổi giới tính theo vòng: (trống) -> Male -> Female -> Other -> (trống)
    auto cycleGender = [&](int step) {
        int index = 0;  // 0 = trống, 1..3 = GENDER_OPTIONS

        for (int i = 0; i < (int)GENDER_OPTIONS.size(); i++) {
            if (form.gender == GENDER_OPTIONS[i]) index = i + 1;
        }

        index = (index + step + 4) % 4;
        form.gender = (index == 0) ? "" : GENDER_OPTIONS[index - 1];
    };

    auto hasChanges = [&]() -> bool {
        return trimString(form.fullName) != originalName ||
               form.birthday != originalBirthday ||
               form.gender != originalGender ||
               trimString(form.bio) != originalBio ||
               residence != originalResidence ||
               hometown != originalHometown ||
               school != originalSchool ||
               trimString(workplace) != originalWorkplace ||
               hobbies != originalHobbies;
    };

    // Vẽ giao diện và đặt con trỏ nhập liệu
    auto drawEditUI = [&]() {
        beginFrame();

        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            hideCursor();
            endFrame();
            return;
        }

        int width = csbi.srWindow.Right
                  - csbi.srWindow.Left + 1;
        int height = csbi.srWindow.Bottom
                   - csbi.srWindow.Top + 1;
        int left = csbi.srWindow.Left;
        int top = csbi.srWindow.Top;

        int maxLen = max(10, width - 6);

        // Dòng quá dài thì giữ phần cuối (chỗ đang gõ) để con trỏ không bị lệch
        auto fit = [&](const string& label, const string& value) -> string {
            int room = maxLen - textLength(label);

            if (textLength(value) <= room) {
                return label + value;
            }

            return label + "..." + tailChars(value, max(0, room - 3));
        };

        string genderText = form.gender.empty() ? "(chưa chọn)" : form.gender;
        if (selected == 3) {
            genderText = "< " + genderText + " >";
        }

        // Ô chọn từ danh sách: đang chọn ô đó thì hiện thêm < > để biết dùng được ← / →
        auto choiceText = [&](int field, const string& value) -> string {
            string text = value.empty() ? "(chưa chọn)" : value;
            return selected == field ? "< " + text + " >" : text;
        };

        string hobbyText = hobbies.empty() ? "(chưa chọn)" : joinList(hobbies, ", ");
        hobbyText += "  [" + to_string(hobbies.size()) + "/" + to_string(MAX_HOBBIES) + "]";

        vector<string> lines = {
            fit("Họ và tên (*): ", form.fullName),
            fit("Ngày sinh: ", form.birthday),
            "Giới tính: " + genderText,
            fit("Giới thiệu: ", form.bio),
            cutText("Nơi ở hiện tại: " + choiceText(5, residence), maxLen),
            cutText("Quê quán: " + choiceText(6, hometown), maxLen),
            cutText("Trường học: " + choiceText(7, school), maxLen),
            fit("Nơi làm việc: ", workplace),
            cutText("Sở thích: " + hobbyText, maxLen),
            "Lưu Thay Đổi",
            "Quay Lại"
        };

        // Title + dòng chú thích + các ô + footer
        int totalLines = (int)lines.size() + 3;
        int startY = top + (height - totalLines) / 2;
        if (startY < top) startY = top;

        printCenter("=============== CHỈNH SỬA THÔNG TIN CÁ NHÂN ===============",
                    startY, 11, width, left);
        printCenter("(*) Thông tin bắt buộc, các mục còn lại có thể để trống", startY + 1, 8, width, left);

        for (int i = 0; i < (int)lines.size(); i++) {
            string text = lines[i];
            int y = startY + 2 + i;

            if(selected == i + 1) {
                text = (i >= SAVE_BUTTON - 1) ? "> " + text + " <"
                                              : "> " + text;
                printCenter(text, y, 14, width, left);
            }
            else {
                printCenter(text, y, 7, width, left);
            }
        }

        // Gợi ý theo ô đang chọn
        string hint;

        switch (selected) {
            case 1: hint = "Gõ để sửa họ và tên | Tối đa 50 ký tự | ENTER: Tiếp"; break;
            case 2: hint = "Gõ để sửa ngày sinh DD-MM-YYYY | Để trống nếu muốn xóa | ENTER: Tiếp"; break;
            case 3: hint = "ENTER hoặc ← / →: Đổi giới tính | Có thể bỏ trống"; break;
            case 4: hint = "Gõ để sửa giới thiệu | Tối đa 100 ký tự | Để trống nếu muốn xóa"; break;
            case 5:
            case 6:
            case 7: hint = "← / →: Đổi nhanh | ENTER: Mở danh sách (nhấn S để tìm kiếm) | Có thể bỏ trống"; break;
            case 8: hint = "Gõ để nhập nơi làm việc | Tối đa 100 ký tự | Để trống nếu muốn xóa"; break;
            case 9: hint = "ENTER: Mở danh sách sở thích (tối đa " + to_string(MAX_HOBBIES) + ", nhấn S để tìm kiếm)"; break;
            case SAVE_BUTTON: hint = "ENTER: Lưu thay đổi | ESC: Quay lại"; break;
            default: hint = "ENTER / ESC: Quay lại (không lưu)"; break;
        }

        printCenter(cutText(hint, width - 2), startY + totalLines - 1, 10, width, left);

        if(!errorMessage.empty()) {
            printCenter(cutText(errorMessage, width - 2), startY + totalLines, 12, width, left);
        }

        // Ô nhập chữ (1, 2, 4, 8) thì hiện con trỏ ở cuối dòng
        if(selected == 1 || selected == 2 || selected == 4 || selected == 8) {
            string displayLine = "> " + lines[selected - 1];

            int x = left
                  + (width - textLength(displayLine)) / 2
                  + textLength(displayLine);
            int y = startY + 1 + selected;

            if(x > csbi.srWindow.Right) {
                x = csbi.srWindow.Right;
            }

            gotoxy(x, y);
            showCursor();
        }
        else {
            hideCursor();
        }

        setColor(7);
        endFrame();
    };

    // Xử lý phím: trả về true khi cần thoát màn hình
    auto handleKey = [&](const KeyInput& key) -> bool {

        // Mũi tên lên/xuống: chuyển ô
        if(moveSelection(selected, BACK_BUTTON, key.type)) {
            errorMessage.clear();
            return false;
        }

        // ESC hoặc nút "Quay Lại": có thay đổi chưa lưu thì hỏi lại
        if(key.type == KEY_ESCAPE ||
           (selected == BACK_BUTTON && key.type == KEY_ENTER)) {

            if(hasChanges()) {
                hideCursor();

                if(!confirmScreen("Bạn có thay đổi chưa lưu. Thoát mà không lưu?")) {
                    return false;
                }
            }

            hideCursor();
            setColor(7);
            return true;
        }

        if(selected == BACK_BUTTON) {
            return false;
        }

        // Nút "Lưu Thay Đổi"
        if(selected == SAVE_BUTTON) {
            if(key.type != KEY_ENTER) {
                return false;
            }

            // Kiểm tra lại toàn bộ, ô nào sai thì nhảy tới ô đó
            for (int field = 1; field <= 9; field++) {
                string error = validateField(field);

                if(!error.empty()) {
                    selected = field;
                    errorMessage = error;
                    return false;
                }
            }

            bool ok = updateProfileField(myId, "full_name", trimString(form.fullName)) &&
                      updateProfileField(myId, "birthday", form.birthday) &&
                      updateProfileField(myId, "gender", form.gender) &&
                      updateProfileField(myId, "bio", trimString(form.bio)) &&
                      updateProfileField(myId, "residence", residence) &&
                      updateProfileField(myId, "hometown", hometown) &&
                      updateProfileField(myId, "school", school) &&
                      updateProfileField(myId, "workplace", trimString(workplace)) &&
                      updateProfileField(myId, "hobbies", joinList(hobbies, ";"));

            if(!ok) {
                errorMessage = "Không thể lưu thay đổi, vui lòng thử lại!";
                return false;
            }

            hideCursor();
            setColor(7);
            notify("Đã cập nhật thông tin!");
            return true;
        }

        // Giới tính: ENTER hoặc mũi tên trái/phải để đổi lựa chọn
        if(selected == 3) {
            if(key.type == KEY_ENTER || key.type == KEY_RIGHT) {
                cycleGender(1);
            }
            else if(key.type == KEY_LEFT) {
                cycleGender(-1);
            }

            errorMessage.clear();
            return false;
        }

        // Nơi ở hiện tại / quê quán / trường học: ← / → đổi nhanh, ENTER mở danh sách có tìm kiếm
        if(selected == 5 || selected == 6 || selected == 7) {
            string& value = (selected == 5) ? residence : (selected == 6) ? hometown : school;
            const vector<PickOption>& options = (selected == 7) ? schoolOptions() : provinceOptions();

            if(key.type == KEY_RIGHT) {
                value = cycleOption(options, value, 1);
            }
            else if(key.type == KEY_LEFT) {
                value = cycleOption(options, value, -1);
            }
            else if(key.type == KEY_ENTER) {
                hideCursor();
                string pickTitle = (selected == 5) ? "=============== CHỌN NƠI Ở HIỆN TẠI ==============="
                                 : (selected == 6) ? "=============== CHỌN QUÊ QUÁN ==============="
                                                   : "=============== CHỌN TRƯỜNG HỌC ===============";
                pickOne(pickTitle, options, value);
            }

            errorMessage.clear();
            return false;
        }

        // Sở thích: ENTER mở danh sách tích chọn
        if(selected == 9) {
            if(key.type == KEY_ENTER) {
                hideCursor();
                pickMany("=============== CHỌN SỞ THÍCH ===============", hobbyOptions(), hobbies, MAX_HOBBIES);
            }

            errorMessage.clear();
            return false;
        }

        // Các ô nhập chữ: 1 = họ tên, 2 = ngày sinh, 4 = giới thiệu, 8 = nơi làm việc
        string& input = (selected == 1) ? form.fullName
                      : (selected == 2) ? form.birthday
                      : (selected == 8) ? workplace
                      :                   form.bio;

        if(key.type == KEY_BACKSPACE) {
            popLastChar(input);

            // Ngày sinh: xóa luôn dấu '-' tự thêm vào
            if(selected == 2 && !input.empty() && input.back() == '-') {
                input.pop_back();
            }

            errorMessage.clear();
        }
        else if(key.type == KEY_ENTER) {
            // Kiểm tra ô hiện tại, hợp lệ mới cho sang ô kế tiếp
            string error = validateField(selected);

            if(!error.empty()) {
                errorMessage = error;
            }
            else {
                errorMessage.clear();
                selected++;
            }
        }
        else if(key.type == KEY_CHARACTER) {
            if(selected == 1) {
                // Họ và tên: nhận chữ có dấu, không bắt đầu bằng khoảng trắng
                bool leadingSpace = input.empty() && key.character == " ";

                if(!leadingSpace && textLength(input) < 50) {
                    input += key.character;
                }
            }
            else if(selected == 2) {
                // Ngày sinh: chỉ nhận chữ số, tự thêm '-' sau ngày và sau tháng
                if(key.asciiCode >= '0' && key.asciiCode <= '9' &&
                   input.length() < 10) {
                    if(input.length() == 2 || input.length() == 5) {
                        input += '-';
                    }

                    input += key.character;
                }
            }
            else {
                // Giới thiệu / nơi làm việc: nhận chữ có dấu, không bắt đầu bằng khoảng trắng
                bool leadingSpace = input.empty() && key.character == " ";

                if(!leadingSpace && textLength(input) < 100) {
                    input += key.character;
                }
            }

            errorMessage.clear();
        }

        return false;
    };

    runLoop(drawEditUI, handleKey);
}
// Đổi mật khẩu
void changePassword(const string& username, const string& myId) {
    InputOptions options;
    options.title = "=============== ĐỔI MẬT KHẨU ===============";
    options.maxLength = 15;
    options.mode = INPUT_ASCII;
    options.mask = true;

    string oldPassword;
    options.prompt = "Nhập mật khẩu hiện tại";
    options.validate = [&](const string& s) -> string {
        return checkAccount(username, s) ? "" : "Mật khẩu hiện tại không đúng!";
    };

    if (!inputText(options, oldPassword)) {
        return;
    }

    string newPassword;
    options.prompt = "Nhập mật khẩu mới (8-15 ký tự, có chữ hoa, chữ thường, số, ký tự đặc biệt)";
    options.validate = [&](const string& s) -> string {
        string error = validatePassword(s);

        if (!error.empty()) return error;
        if (s == oldPassword) return "Mật khẩu mới phải khác mật khẩu hiện tại!";

        return "";
    };

    if (!inputText(options, newPassword)) {
        return;
    }

    string confirmPassword;
    options.prompt = "Nhập lại mật khẩu mới";
    options.validate = [&](const string& s) -> string {
        return s == newPassword ? "" : "Mật khẩu nhập lại không khớp!";
    };

    if (!inputText(options, confirmPassword)) {
        return;
    }

    if (updateProfileField(myId, "password", stringToHex(newPassword))) {
        notify("Đổi mật khẩu thành công!");
    }
    else {
        notify("Không thể lưu mật khẩu mới, vui lòng thử lại!");
    }
}
// Xóa toàn bộ dữ liệu của 1 tài khoản (bạn bè, bài đăng, nhóm, tin nhắn, thư mục tài khoản)
void deleteAccountData(const string& accountId) {
    error_code ec;

    // 1. Gỡ khỏi danh sách quan hệ của mọi tài khoản khác
    for (const string& other : listAccountIds()) {
        if (other != accountId && !getRelation(other, accountId).empty()) {
            setRelation(other, accountId, "");
        }
    }

    // 2. Xóa bài đăng của mình; gỡ lượt thích và bình luận của mình ở các bài khác
    for (const string& postId : listPostIds()) {
        Post post;

        if (!readPost(postId, post)) {
            continue;
        }

        if (post.authorId == accountId) {
            deletePost(postId);
            continue;
        }

        vector<string> likes = readLikes(postId);
        likes.erase(remove(likes.begin(), likes.end(), accountId), likes.end());
        writeLines(postDir(postId) / "likes.txt", likes);

        vector<PostComment> comments = readComments(postId);
        comments.erase(remove_if(comments.begin(), comments.end(),
                                 [&](const PostComment& comment) {
                                     return comment.authorId == accountId;
                                 }),
                       comments.end());
        writeComments(postId, comments);
    }

    // 3. Rời khỏi các nhóm chat
    for (const string& groupId : listGroupIds()) {
        Group group;

        if (readGroup(groupId, group) && isGroupMember(group, accountId)) {
            removeGroupMember(groupId, accountId, getDisplayName(accountId) + " đã rời nhóm");
        }
    }

    // 4. Xóa các cuộc trò chuyện riêng (tên file: <ID>_<ID>.txt)
    for (const auto& entry : fs::directory_iterator("Data/Conversations/.Private", ec)) {
        string stem = entry.path().stem().string();
        size_t pos = stem.find('_');

        if (pos == string::npos) {
            continue;
        }

        if (stem.substr(0, pos) == accountId || stem.substr(pos + 1) == accountId) {
            fs::remove(entry.path(), ec);
        }
    }

    // 5. Xóa các báo cáo mà tài khoản gửi đi hoặc bị báo cáo
    removeReportsAbout(accountId);
    removeReportsBy(accountId);
    removeNotificationsFrom(accountId);
    removeAccountIssue(accountId);

    // 6. Xóa thư mục tài khoản
    fs::remove_all(fs::path("Data/Account") / accountId, ec);
}
// Xóa tài khoản. Trả về true nếu đã xóa
bool deleteAccountFlow(const string& username, const string& myId) {
    if (!confirmScreen("Xóa tài khoản sẽ mất toàn bộ dữ liệu của bạn. Bạn có chắc không?")) {
        return false;
    }

    InputOptions options;
    options.title = "=============== XÓA TÀI KHOẢN ===============";
    options.prompt = "Nhập mật khẩu để xác nhận xóa tài khoản";
    options.maxLength = 15;
    options.mode = INPUT_ASCII;
    options.mask = true;
    options.validate = [&](const string& s) -> string {
        return checkAccount(username, s) ? "" : "Mật khẩu không đúng!";
    };

    string password;

    if (!inputText(options, password)) {
        return false;
    }

    deleteAccountData(myId);
    clearLoginState();

    notify("Tài khoản đã được xóa. Hẹn gặp lại bạn!");
    return true;
}
// Menu Cài Đặt Tài Khoản. Trả về true nếu tài khoản đã bị xóa (cần quay về màn hình đăng nhập)
bool settingsMenu(const string& username) {
    string myId = getAccountId(username);

    if (myId.empty()) {
        notify("Không đọc được thông tin tài khoản!");
        return false;
    }

    vector<string> items = {
        "Chỉnh Sửa Thông Tin Cá Nhân",
        "Đổi Mật Khẩu",
        "Danh Sách Đã Chặn",
        "Xóa Tài Khoản",
        "Quay Lại"
    };

    while (true) {
        int choice = menu(
            "=============== CÀI ĐẶT TÀI KHOẢN ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                editAccount(username);
                break;

            case 2:
                changePassword(username, myId);
                break;

            case 3:
                blockedListScreen(myId);
                break;

            case 4:
                if (deleteAccountFlow(username, myId)) {
                    return true;
                }
                break;

            default:
                return false;   // "Quay Lại" (5) hoặc ESC (0)
        }
    }
}

// Xem trang cá nhân của tôi. Thoát khi chọn "Quay Lại" hoặc nhấn ESC
void viewProfile(const string& username) {
    vector<string> items = {
        "Xem Bạn Bè",
        "Xem Bài Đăng Của Tôi",
        "Chỉnh Sửa Tài Khoản",
        "Quay Lại"
    };

    while (true) {
        // Đọc lại hồ sơ mỗi lần vẽ để sau này chỉnh sửa xong sẽ thấy dữ liệu mới
        UserProfile profile;

        if (!findProfile(username, profile)) {
            printCenter("Không đọc được thông tin tài khoản!");
            sleep_for(2s);
            return;
        }

        // Khối thông tin chỉ để xem: {nhãn, giá trị}
        // Trang của tôi luôn hiện Bio, Ngày Sinh, Giới Tính (trống thì ghi "chưa có")
        vector<pair<string, string>> info = buildProfileInfo(profile, -1, true);

        int choice = menuWithInfo(
            "=============== TRANG CÁ NHÂN CỦA TÔI ===============",
            info,
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại"
        );

        switch (choice) {
            case 1:
                viewFriends(username, true);    // Từ trang cá nhân: hiện số bạn chung
                break;

            case 2:
                viewMyPosts(username);
                break;

            case 3:
                editAccount(username);
                break;

            default:
                // "Quay Lại" (4) hoặc ESC (0)
                return;
        }
    }
}

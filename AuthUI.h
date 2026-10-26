// Module: Màn hình đăng nhập và đăng ký
#pragma once

// Gửi báo cáo xin cấp lại mật khẩu cho quản trị viên (định nghĩa trong Report.h)
void forgotPasswordFlow(const string& typedUsername);

// Sai từng này lần (hoặc nhiều hơn) thì hiện thêm mục "Quên mật khẩu. Báo cáo với quản trị viên"
const int LOGIN_FAILS_BEFORE_HELP = 5;

// Màn hình đăng nhập: trả về 0 nếu nhấn ESC, 3 nếu người dùng đăng nhập thành công,
// 4 nếu quản trị viên đăng nhập thành công.
// Khi đăng nhập chỉ kiểm tra đúng / sai tài khoản và mật khẩu, không kiểm tra định dạng
int loginMenu(
    const string& title,
    const vector<string>& items,
    const string& footer,
    string& loggedUsername
) {
    // Kiểm tra menu phải có đủ 3 mục
    if(items.size() != 3) {
        hideCursor();
        return 0;
    }

    string username;
    string password;
    int selected = 1;
    string errorMessage;
    int result = 0;
    int failCount = 0;      // Số lần đăng nhập sai

    // Mục thứ 4 "Quên mật khẩu..." chỉ xuất hiện sau khi sai quá nhiều lần
    auto itemCount = [&]() -> int {
        return failCount >= LOGIN_FAILS_BEFORE_HELP ? 4 : 3;
    };

    // Vẽ giao diện và đặt con trỏ nhập liệu
    auto drawLoginUI = [&]() {
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

        vector<string> lines = {
            "Tài khoản: " + username,
            "Mật khẩu: " + string(password.length(), '*'),
            items[2]
        };

        if(itemCount() == 4) {
            lines.push_back("Quên mật khẩu. Báo cáo với quản trị viên");
        }

        int totalLines = (int)lines.size() + 2;
        int startY = top + (height - totalLines) / 2;

        printCenter(title, startY, 11, width, left);

        for (int i = 0; i < (int)lines.size(); i++) {
            string text = lines[i];

            if(selected == i + 1) {
                if(i == 0 || i == 1) {
                    text = "> " + text;
                } else {
                    text = "> " + text + " <";
                }

                printCenter(text, startY + i + 1, 14, width, left);
            }
            else {
                printCenter(text, startY + i + 1, 7, width, left);
            }
        }

        string hint = footer;

        if(selected == 1) {
            hint = "Nhập tài khoản | Backspace: Xóa";
        }
        else if(selected == 2) {
            hint = "Nhập mật khẩu | Backspace: Xóa";
        }
        else if(selected == 4) {
            hint = "ENTER: Gửi báo cáo xin cấp lại mật khẩu cho quản trị viên";
        }

        printCenter(hint, startY + totalLines - 1, 10, width, left);

        if(!errorMessage.empty()) {
            printCenter(errorMessage, startY + totalLines, 12, width, left);
        }

        if(selected == 1 || selected == 2) {
            string displayLine = "> " + lines[selected - 1];

            int x = left
                  + (width - textLength(displayLine)) / 2
                  + textLength(displayLine);
            int y = startY + selected;

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

    // Xử lý phím: trả về true khi cần thoát menu
    auto handleKey = [&](const KeyInput& key) -> bool {

        // Mũi tên lên/xuống
        if(moveSelection(selected, itemCount(), key.type)) {
            errorMessage.clear();
            return false;
        }

        // ESC: thoát menu, quay lại màn hình trước
        if(key.type == KEY_ESCAPE) {
            hideCursor();
            setColor(7);
            result = 0;
            return true;
        }

        // Nhập tài khoản hoặc mật khẩu
        if(selected == 1 || selected == 2) {
            string& input = (selected == 1) ? username : password;

            if(key.type == KEY_BACKSPACE) {
                if(!input.empty()) input.pop_back();
            }
            else if(key.type == KEY_ENTER) {
                selected++;
            }
            else if(key.type == KEY_CHARACTER &&
                    key.asciiCode >= 33 && key.asciiCode <= 126) {
                if(input.length() < 30) input += key.character;
            }

            errorMessage.clear();
            return false;
        }

        // Chỉ xử lý khi nhấn Enter
        if(key.type != KEY_ENTER) {
            return false;
        }

        // Mục "Quên mật khẩu. Báo cáo với quản trị viên"
        if(selected == 4) {
            hideCursor();
            forgotPasswordFlow(username);
            errorMessage.clear();
            return false;
        }

        // Đúng tài khoản quản trị viên: vào menu quản trị
        if(isAdminLogin(username, password)) {
            printCenter("Đã đăng nhập quản trị viên thành công!");
            sleep_for(3s);
            hideCursor();
            setColor(7);
            loggedUsername = username;
            result = 4;
            return true;
        }

        if(checkAccount(username, password)) {
            // Tài khoản bị quản trị viên khóa thì không cho đăng nhập
            if(isUsernameLocked(username)) {
                errorMessage = "Tài khoản này đã bị quản trị viên khóa!";
                return false;
            }

            printCenter("Đã đăng nhập thành công!");
            sleep_for(3s);
            hideCursor();
            setColor(7);
            loggedUsername = username;
            result = 3;
            return true;
        }

        // Chỉ báo chung "sai tài khoản hoặc mật khẩu" (không nói rõ sai cái nào)
        failCount++;
        errorMessage = "Sai tài khoản hoặc mật khẩu!";

        if(failCount >= LOGIN_FAILS_BEFORE_HELP) {
            errorMessage += " (đã sai " + to_string(failCount) + " lần)";
        }

        return false;
    };

    runLoop(drawLoginUI, handleKey);

    return result;
}
// Màn hình đăng ký: trả về true nếu tạo tài khoản thành công, false nếu nhấn ESC
bool registerMenu(const string& title, string& createdUsername) {
    RegisterForm form;

    const int BUTTON = 7;       // 6 ô nhập + 1 nút "Đăng Ký"
    int selected = 1;
    string errorMessage;
    bool success = false;

    // Đổi giới tính theo vòng: (trống) -> Male -> Female -> Other -> (trống)
    // step = +1: tiến tới, step = -1: lùi lại
    auto cycleGender = [&](int step) {

        int index = 0;  // 0 = trống, 1..3 = GENDER_OPTIONS

        for (int i = 0; i < (int)GENDER_OPTIONS.size(); i++) {
            if (form.gender == GENDER_OPTIONS[i]) index = i + 1;
        }

        index = (index + step + 4) % 4;
        form.gender = (index == 0) ? "" : GENDER_OPTIONS[index - 1];
    };

    // Vẽ giao diện và đặt con trỏ nhập liệu
    auto drawRegisterUI = [&]() {
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

        // Hiển thị giới tính: khi đang chọn thì có dấu < > hai bên
        string genderText = form.gender.empty() ? "(chưa chọn)" : form.gender;
        if (selected == 5) {
            genderText = "< " + genderText + " >";
        }

        vector<string> lines = {
            "Tài khoản (*): " + form.username,
            "Mật khẩu (*): " + string(form.password.length(), '*'),
            "Họ và tên (*): " + form.fullName,
            "Ngày sinh: " + form.birthday,
            "Giới tính: " + genderText,
            "Giới thiệu: " + form.bio,
            "Đăng Ký"
        };

        // Title + dòng chú thích + các ô + footer
        int totalLines = (int)lines.size() + 3;
        int startY = top + (height - totalLines) / 2;

        printCenter(title, startY, 11, width, left);
        printCenter("(*) Thông tin bắt buộc", startY + 1, 8, width, left);

        for (int i = 0; i < (int)lines.size(); i++) {
            string text = lines[i];
            int y = startY + 2 + i;

            if(selected == i + 1) {
                if(i == BUTTON - 1) {
                    text = "> " + text + " <";
                } else {
                    text = "> " + text;
                }

                printCenter(text, y, 14, width, left);
            }
            else {
                printCenter(text, y, 7, width, left);
            }
        }

        // Gợi ý theo ô đang chọn
        string hint;

        switch (selected) {
            case 1: hint = "Nhập tài khoản | 5-15 ký tự: chữ, số, _ | ENTER: Tiếp"; break;
            case 2: hint = "Nhập mật khẩu | 8-15 ký tự | ENTER: Tiếp"; break;
            case 3: hint = "Nhập họ và tên | Tối đa 50 ký tự | ENTER: Tiếp"; break;
            case 4: hint = "Nhập ngày sinh DD-MM-YYYY | Có thể bỏ trống | ENTER: Tiếp"; break;
            case 5: hint = "ENTER hoặc ← / →: Đổi giới tính | Có thể bỏ trống | ▼: Tiếp"; break;
            case 6: hint = "Nhập giới thiệu | Tối đa 100 ký tự | Có thể bỏ trống"; break;
            default: hint = "ENTER: Đăng ký | ESC: Quay lại"; break;
        }

        printCenter(hint, startY + totalLines - 1, 10, width, left);

        if(!errorMessage.empty()) {
            printCenter(errorMessage, startY + totalLines, 12, width, left);
        }

        // Ô nhập chữ (1, 2, 3, 4, 6) thì hiện con trỏ ở cuối dòng
        if(selected != 5 && selected != BUTTON) {
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

    // Xử lý phím: trả về true khi cần thoát màn hình đăng ký
    auto handleKey = [&](const KeyInput& key) -> bool {

        // Mũi tên lên/xuống: chuyển ô
        if(moveSelection(selected, BUTTON, key.type)) {
            errorMessage.clear();
            return false;
        }

        // ESC: quay lại menu chính
        if(key.type == KEY_ESCAPE) {
            hideCursor();
            setColor(7);
            return true;
        }

        // Nút "Đăng Ký"
        if(selected == BUTTON) {
            if(key.type != KEY_ENTER) {
                return false;
            }

            // Kiểm tra lại toàn bộ các ô, ô nào sai thì nhảy tới ô đó
            for (int field = 1; field <= 6; field++) {
                string error = validateRegisterField(field, form);

                if(!error.empty()) {
                    selected = field;
                    errorMessage = error;
                    return false;
                }
            }

            string accountId;

            if(!createAccount(form, accountId)) {
                errorMessage = "Không thể lưu tài khoản, vui lòng thử lại!";
                return false;
            }

            printCenter("Đăng ký thành công! ID của bạn: " + accountId);
            sleep_for(3s);

            hideCursor();
            setColor(7);
            createdUsername = form.username;
            success = true;
            return true;
        }

        // Giới tính: ENTER hoặc mũi tên trái/phải để đổi lựa chọn
        if(selected == 5) {
            if(key.type == KEY_ENTER || key.type == KEY_RIGHT) {
                cycleGender(1);
            }
            else if(key.type == KEY_LEFT) {
                cycleGender(-1);
            }

            errorMessage.clear();
            return false;
        }

        // Các ô nhập chữ
        string& input = (selected == 1) ? form.username
                      : (selected == 2) ? form.password
                      : (selected == 3) ? form.fullName
                      : (selected == 4) ? form.birthday
                      :                   form.bio;

        if(key.type == KEY_BACKSPACE) {
            popLastChar(input);

            // Ngày sinh: xóa luôn dấu '-' tự thêm vào
            if(selected == 4 && !input.empty() && input.back() == '-') {
                input.pop_back();
            }

            errorMessage.clear();
        }
        else if(key.type == KEY_ENTER) {
            // Kiểm tra ô hiện tại, hợp lệ mới cho sang ô kế tiếp
            string error = validateRegisterField(selected, form);

            if(!error.empty()) {
                errorMessage = error;
            }
            else {
                errorMessage.clear();
                selected++;
            }
        }
        else if(key.type == KEY_CHARACTER) {
            if(selected == 1 || selected == 2) {
                // Tài khoản, mật khẩu: chỉ nhận ký tự ASCII, không có khoảng trắng
                if(key.asciiCode >= 33 && key.asciiCode <= 126 &&
                   input.length() < 15) {
                    input += key.character;
                }
            }
            else if(selected == 3) {
                // Họ và tên: nhận chữ có dấu, không bắt đầu bằng khoảng trắng
                bool leadingSpace = input.empty() && key.character == " ";

                if(!leadingSpace && textLength(input) < 50) {
                    input += key.character;
                }
            }
            else if(selected == 4) {
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
                // Giới thiệu: nhận chữ có dấu, không bắt đầu bằng khoảng trắng
                bool leadingSpace = input.empty() && key.character == " ";

                if(!leadingSpace && textLength(input) < 100) {
                    input += key.character;
                }
            }

            errorMessage.clear();
        }

        return false;
    };

    runLoop(drawRegisterUI, handleKey);

    return success;
}

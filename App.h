// Module: Lớp App - điểm vào của chương trình
#pragma once

#include "Console.h"
#include "Text.h"
#include "VietnamData.h"
#include "AccountCore.h"
#include "UI.h"
#include "AuthUI.h"
#include "Profile.h"
#include "Utils.h"
#include "Reader.h"
#include "Cards.h"
#include "Picker.h"
#include "Notification.h"
#include "Friend.h"
#include "Report.h"
#include "Post.h"
#include "Chat.h"
#include "Group.h"
#include "ProfileScreens.h"
#include "Suggest.h"
#include "FriendMenus.h"
#include "Settings.h"
#include "NotificationScreens.h"
#include "MenuUser.h"
#include "Admin.h"

/*
        LỚP APP
                    */
class App {
    private:
        void Initialize();
        string isLoggedIn();
        void showLoginScreen();
    public:
        void run();
};

// Khởi tạo thư mục, bật hỗ trợ tiếng Việt có dấu, tạo double buffer và ẩn con trỏ
void App::Initialize() {
    // Bật hỗ trợ tiếng Việt có dấu
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    // Bật nhận sự kiện lăn chuột để cuộn nội dung
    enableMouseInput();

    // Tạo double buffer (phải gọi sau khi bật UTF-8)
    initDoubleBuffer();

    // Ẩn con trỏ
    hideCursor();

    // Danh sách các thư mục cần tạo
    const vector<fs::path> folders = {
        "Data",
        "Data/Account",
        "Data/Conversations",
        "Data/Conversations/.Private",
        "Data/Conversations/.Group",
        "Data/Post",
        "Data/Report",
        "Data/System"
    };

    // Kiểm tra và tạo thư mục nếu chưa tồn tại
    error_code ec;

    for (const auto& folder : folders) {
        ec.clear();

        if(!fs::exists(folder, ec)) {
            if(ec) {
                cerr << "Lỗi kiểm tra thư mục: "
                     << folder.string() << '\n';
                return;
            }

            if(!fs::create_directories(folder, ec) || ec) {
                cerr << "Không thể tạo thư mục: "
                     << folder.string() << '\n';
                return;
            }
        }
        else {
            ec.clear();

            if(!fs::is_directory(folder, ec) || ec) {
                cerr << "Đường dẫn không phải thư mục: "
                     << folder.string() << '\n';
                return;
            }
        }
    }

    return;
}
// Kiểm tra trạng thái đăng nhập
string App::isLoggedIn() {

    // Mở file isLogin.txt để đọc trạng thái đăng nhập
    string path = "Data/System/isLogin.txt";
    ifstream file(path);

    // Kiểm tra file có mở được không
    if(!file.is_open()) {
        return ""; // Không tìm thấy hoặc không đọc được file
    }

    string data;

    // Đọc dòng duy nhất
    if(!getline(file, data)) {
        file.close();
        return "";
    }

    file.close();

    // Bỏ '\r' nếu có, và tài khoản trong file phải còn tồn tại
    if (!data.empty() && data.back() == '\r') {
        data.pop_back();
    }

    if (data.empty() || !checkAccount(data, "", true)) {
        return "";
    }

    // Tài khoản đã bị quản trị viên khóa thì không tự đăng nhập lại nữa
    if (isUsernameLocked(data)) {
        clearLoginState();
        return "";
    }

    return data;
}
// Hiển thị màn hình đăng nhập
void App::showLoginScreen() {
    vector<string> items = {
        "Đăng Nhập",
        "Tạo Tài Khoản",
        "Thoát"
    };

    bool running = true;

    // Lặp lại menu chính cho đến khi người dùng thoát hoặc đăng nhập thành công
    while (running) {
        int choice = menu(
            "=============== CHÀO MỪNG ĐẾN VỚI MẠNG XÃ HỘI MINI ===============",
            items,
            "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Thoát"
        );

        switch (choice) {
            case 1: {
                vector<string> loginItems = {
                    "Tài Khoản",
                    "Mật Khẩu",
                    "Đăng Nhập"
                };
                string loggedUsername;
                int choiceLogin = loginMenu(
                    "=============== ĐĂNG NHẬP ===============",
                    loginItems,
                    "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Quay lại",
                    loggedUsername
                );

                // 4 = quản trị viên đăng nhập -> vào menu quản trị (không ghi nhớ phiên đăng nhập)
                if (choiceLogin == 4) {
                    // true = thoát chương trình, false = đăng xuất -> quay lại menu chính
                    if (menuAdmin(loggedUsername)) {
                        running = false;
                    }
                }
                // 0 = nhấn ESC -> quay lại menu chính
                else if (choiceLogin != 0) {
                    // Ghi nhớ phiên đăng nhập vào Data/System/isLogin.txt
                    saveLoginState(loggedUsername);

                    // true = thoát chương trình, false = đăng xuất -> quay lại menu chính
                    if (menuUser(loggedUsername)) {
                        running = false;
                    }
                }
                break;
            }
            case 2: {
                // Đăng ký thành công thì đăng nhập luôn, nhấn ESC thì quay lại menu chính
                string newUsername;

                if (registerMenu("=============== TẠO TÀI KHOẢN ===============", newUsername)) {
                    saveLoginState(newUsername);

                    if (menuUser(newUsername)) {
                        running = false;
                    }
                }
                break;
            }

            case 3:
            case 0:
                // Chọn "Thoát" hoặc nhấn ESC ở menu chính -> thoát chương trình
                printCenter("Đã thoát chương trình!");
                sleep_for(3s);
                running = false;
                break;
        }
    }

    setColor(7);
}
// Chạy ứng dụng
void App::run() {

    // Khởi tạo thư mục, bật hỗ trợ tiếng Việt có dấu, double buffer và ẩn con trỏ
    Initialize();

    // Kiểm tra trạng thái đăng nhập
    string isLogin = isLoggedIn();
    if(isLogin.empty()) {      // Nếu chưa đăng nhập, hiển thị màn hình đăng nhập
        showLoginScreen();
    } else {                    // Nếu đã đăng nhập, vào thẳng menu người dùng
        bool exitApp = menuUser(isLogin);

        if(!exitApp) {          // Đăng xuất -> về màn hình đăng nhập
            showLoginScreen();
        }
    }

    // Khôi phục buffer gốc của console trước khi thoát
    cleanupDoubleBuffer();

    // Khôi phục chế độ nhập gốc của console
    restoreInputMode();
}

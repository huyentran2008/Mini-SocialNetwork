// Module: Menu người dùng
#pragma once

/*
    MENU NGƯỜI DÙNG (SAU KHI ĐĂNG NHẬP)
                                        */

// Chọn ngẫu nhiên 1 lời chào (viết sẵn bằng chữ in hoa)
string getRandomGreeting() {
    static const vector<string> greetings = {
        "XIN CHÀO",
        "CHÀO MỪNG TRỞ LẠI",
        "RẤT VUI ĐƯỢC GẶP BẠN",
        "HELLO",
        "CHÀO BẠN",
        "MỪNG BẠN QUAY LẠI",
        "NGÀY MỚI VUI VẺ",
        "CHÚC BẠN MỘT NGÀY TỐT LÀNH",
        "HÔM NAY CỦA BẠN THẾ NÀO",
        "BẠN KHỎE KHÔNG",
        "LÂU QUÁ MỚI GẶP",
        "NHỚ BẠN QUÁ",
        "CHÀO MỘT NGÀY TRÀN ĐẦY NĂNG LƯỢNG",
        "HÔM NAY CÓ GÌ MỚI KHÔNG",
        "MỌI NGƯỜI ĐANG CHỜ BẠN ĐẤY",
        "BẠN ĐÃ SẴN SÀNG CHƯA",
        "CHÀO MỪNG ĐẾN VỚI MẠNG XÃ HỘI MINI",
        "WELCOME BACK",
        "GOOD TO SEE YOU",
        "HEY",
        "XIN CHÀO BẠN HIỀN",
        "CHÀO CẬU",
        "CHÀO NGÀY MỚI",
        "HÔM NAY BẠN CÓ VUI KHÔNG",
        "CHÀO MỪNG BẠN ĐÃ ĐẾN",
        "KẾT NỐI VÀ CHIA SẺ NÀO",
        "CÙNG TRÒ CHUYỆN NÀO",
        "ĐỪNG QUÊN NHẮN TIN CHO BẠN BÈ",
        "CHÚC BẠN MỘT NGÀY THẬT VUI",
        "HÂN HẠNH ĐÓN TIẾP BẠN",
        "ALOHA",
        "BONJOUR",
        "KONNICHIWA",
        "ANNYEONG",
        "HOLA"
    };

    static mt19937 rng(random_device{}());
    uniform_int_distribution<size_t> dist(0, greetings.size() - 1);

    return greetings[dist(rng)];
}
// Menu chính sau khi đăng nhập
bool menuUser(const string& username) {
    // Chọn lời chào 1 lần khi vào menu để không bị đổi mỗi lần vẽ lại
    string title = "================= " + getRandomGreeting()
                 + " (" + username + ") =================";

    UserProfile me;
    string myId = findProfile(username, me) ? me.id : "";

    while (true) {
        // Số thông báo chưa đọc hiện ngay trên menu (tính lại mỗi lần vẽ menu)
        int unread = myId.empty() ? 0 : countUnreadNotifications(myId);

        vector<string> items = {
            "Xem Trang Cá Nhân Của Tôi",
            "Bài Đăng",
            "Bạn Bè",
            "Nhắn Tin",
            unread > 0 ? "Thông Báo (" + to_string(unread) + " mới)" : "Thông Báo",
            "Cài Đặt Tài Khoản",
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
                viewProfile(username);
                break;

            case 2:
                postMenu(username);
                break;

            case 3:
                friendMenu(username);
                break;

            case 4:
                messageMenu(username);
                break;

            case 5:
                notificationMenu(username);
                break;

            case 6:
                // Xóa tài khoản xong thì quay lại màn hình đăng nhập
                if (settingsMenu(username)) {
                    return false;
                }
                break;

            case 7:
                // Đăng xuất: xóa phiên đăng nhập rồi quay lại màn hình đăng nhập
                clearLoginState();
                printCenter("Đã đăng xuất!");
                sleep_for(2s);
                return false;

            case 8:
                // Thoát chương trình
                printCenter("Đã thoát chương trình!");
                sleep_for(3s);
                return true;

            default:
                // Nhấn ESC: ở lại menu, không làm gì
                break;
        }
    }
}


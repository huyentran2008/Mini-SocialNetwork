#include <iostream>
#include "FileManager.h"
#include "Friend.h"
int main() {
	system("chcp 65001 > nul"); // Ép cửa sổ Console nhận bảng mã UTF-8
    // 1. Khởi tạo cấu trúc thư mục và hệ thống file mẫu
    FileManager::initializeSystemDirectories();

    // 2. Tạo sẵn dữ liệu mẫu cho các tài khoản để dễ dàng kiểm thử kết bạn, bạn chung, gợi ý
    FileManager::createAccountDataStructure("ACC000001", "nguyenvana", "Nguyen Van A", "123456");
    FileManager::createAccountDataStructure("ACC000002", "nguyenvanb", "Nguyen Van B", "123456");
    FileManager::createAccountDataStructure("ACC000003", "nguyenvanc", "Nguyen Van C", "123456");

    std::cout << "\n==================================================\n";
    std::cout << "   KIỂM THỬ MODULE FRIEND (ĐỒ THỊ & KẾT BẠN)      \n";
    std::cout << "==================================================\n";

    // Khởi tạo trình quản lý kết bạn cho tài khoản hiện tại: ACC000001
    FriendManager fm("ACC000001");

    int choice = 0;
    do {
        std::cout << "\n---------------- MENU QUẢN LÝ KẾT BẠN ----------------\n";
        std::cout << "1. Gửi lời mời kết bạn (Test gửi đến ACC000002 hoặc ACC000003)\n";
        std::cout << "2. Chấp nhận lời mời kết bạn\n";
        std::cout << "3. Tìm bạn chung (Mutual Friends)\n";
        std::cout << "4. Gợi ý bạn bè tiềm năng (Friend Suggestion)\n";
        std::cout << "0. Thoát chương trình\n";
        std::cout << "------------------------------------------------------\n";
        std::cout << "Nhập lựa chọn của bạn: ";

        if (!(std::cin >> choice)) {
            std::cout << "[LỖI] Lựa chọn không hợp lệ! Vui lòng nhập số.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                fm.sendFriendRequest();
                break;
            case 2:
                fm.acceptFriendRequest();
                break;
            case 3:
                fm.displayMutualFriendsUI();
                break;
            case 4:
                fm.suggestFriends();
                break;
            case 0:
                std::cout << "Đã thoát chương trình kiểm thử. Tạm biệt!\n";
                break;
            default:
                std::cout << "[LỖI] Lựa chọn không tồn tại, vui lòng nhập lại từ 0-4!\n";
        }
    } while (choice != 0);

    return 0;
}
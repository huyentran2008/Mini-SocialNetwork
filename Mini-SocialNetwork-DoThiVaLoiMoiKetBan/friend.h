#ifndef FRIEND_H
#ifndef FRIEND_H
#define FRIEND_H

#include <string>
#include <vector>
#include <map>

// Các trạng thái quan hệ bạn bè trong file friend.txt
enum FriendStatus {
    FRIEND,
    PENDING_SENT,
    PENDING_RECEIVED,
    BLOCK,
    NONE
};

struct FriendConnection {
    std::string targetUserId;
    FriendStatus status;
    std::string date; // Định dạng: YYYY-MM-DD
};

class FriendManager {
private:
    std::string currentUserId;
    // Đồ thị lưu danh sách kề: userId -> danh sách các bạn bè (trạng thái FRIEND)
    // Phục vụ cho thuật toán BFS, tìm bạn chung, gợi ý bạn bè
    std::map<std::string, std::vector<std::string>> adjacencyList;

    // Hàm chuyển đổi enum sang string để ghi file
    std::string statusToString(FriendStatus status);
    // Hàm chuyển đổi string từ file sang enum
    FriendStatus stringToStatus(const std::string& str);

    // Kiểm tra tính hợp lệ của ID người dùng (tránh tự kết bạn, v.v.)
    bool isValidTarget(const std::string& targetId);

public:
    FriendManager(const std::string& userId);

    // Tải dữ liệu bạn bè từ file friend.txt của người dùng
    void loadFriendsData();

    // Lưu dữ liệu bạn bè ra file friend.txt của người dùng
    void saveFriendsData();

    // --- CÁC TÁC VỤ CHÍNH ---
    
    // 1. Gửi lời mời kết bạn (Xử lý lỗi & phân quyền chi tiết)
    void sendFriendRequest();

    // 2. Chấp nhận lời mời kết bạn
    void acceptFriendRequest();

    // 3. Từ chối lời mời kết bạn
    void rejectFriendRequest();

    // 4. Hủy kết bạn hoặc hủy lời mời đã gửi / Bỏ chặn
    void removeOrCancelFriend();

    // 5. Xem danh sách bạn bè hiện tại
    void viewFriendList();

    // 6. Xem các lời mời kết bạn đang chờ (Đã gửi & Đã nhận)
    void viewPendingRequests();

    // --- TÍNH NĂNG NÂNG CAO (Bạn chung & Gợi ý bạn bè) ---

    // Tìm danh sách bạn chung giữa currentUserId và targetUserId
    std::vector<std::string> getMutualFriends(const std::string& targetUserId);

    // Hiển thị danh sách bạn chung ra màn hình
    void displayMutualFriendsUI();

    // Gợi ý bạn bè tiềm năng (Dựa trên bạn chung - BFS 2 bước)
    void suggestFriends();

    // Cập nhật lại đồ thị kề (Dùng khi có thay đổi kết bạn)
    void updateAdjacencyGraph(const std::string& userId, const std::vector<std::string>& friends);
};

#endif
#endif
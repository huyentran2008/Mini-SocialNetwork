#include "Friend.h"
#include "FileManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <set>

FriendManager::FriendManager(const std::string& userId) {
    this->currentUserId = userId;
    loadFriendsData();
}

std::string FriendManager::statusToString(FriendStatus status) {
    switch (status) {
        case FRIEND: return "FRIEND";
        case PENDING_SENT: return "PENDING_SENT";
        case PENDING_RECEIVED: return "PENDING_RECEIVED";
        case BLOCK: return "BLOCK";
        default: return "NONE";
    }
}

FriendStatus FriendManager::stringToStatus(const std::string& str) {
    if (str == "FRIEND") return FRIEND;
    if (str == "PENDING_SENT") return PENDING_SENT;
    if (str == "PENDING_RECEIVED") return PENDING_RECEIVED;
    if (str == "BLOCK") return BLOCK;
    return NONE;
}

bool FriendManager::isValidTarget(const std::string& targetId) {
    if (targetId == currentUserId) {
        std::cout << "[LỖI] Bạn không thể thao tác với chính mình!\n";
        return false;
    }
    std::string profilePath = "Data/Accounts/" + targetId + "/profile.txt";
    std::ifstream file(profilePath);
    if (!file.is_open()) {
        std::cout << "[LỖI] Mã người dùng '" << targetId << "' không tồn tại trong hệ thống!\n";
        return false;
    }
    return true;
}

void FriendManager::loadFriendsData() {
    std::string path = "Data/Accounts/" + currentUserId + "/friend.txt";
    std::ifstream file(path);
    adjacencyList[currentUserId].clear();

    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string targetId, statusStr, date;
        
        if (std::getline(ss, targetId, '|') &&
            std::getline(ss, statusStr, '|') &&
            std::getline(ss, date)) {
            
            FriendStatus status = stringToStatus(statusStr);
            if (status == FRIEND) {
                adjacencyList[currentUserId].push_back(targetId);
            }
        }
    }
    file.close();
}

void FriendManager::saveFriendsData() {
}

void FriendManager::sendFriendRequest() {
    std::string targetId;
    bool success = false;

    while (!success) {
        std::cout << "\n=== GỬI LỜI MỜI KẾT BẠN ===\n";
        std::cout << "Nhập ID người bạn muốn kết bạn (hoặc nhập 'CANCEL' để hủy): ";
        std::cin >> targetId;

        if (targetId == "CANCEL" || targetId == "cancel") {
            std::cout << "Đã hủy thao tác gửi lời mời.\n";
            return;
        }

        if (!isValidTarget(targetId)) {
            std::cout << "-> Vui lòng kiểm tra lại ID và nhập lại!\n";
            continue;
        }

        std::string path = "Data/Accounts/" + currentUserId + "/friend.txt";
        std::ifstream inFile(path);
        std::string line;
        bool alreadyExists = false;
        std::string currentStatus = "";

        while (std::getline(inFile, line)) {
            std::stringstream ss(line);
            std::string id, status, date;
            std::getline(ss, id, '|');
            std::getline(ss, status, '|');
            std::getline(ss, date, '|');
            if (id == targetId) {
                alreadyExists = true;
                currentStatus = status;
            }
        }
        inFile.close();

        if (alreadyExists) {
            if (currentStatus == "FRIEND") {
                std::cout << "[THÔNG BÁO] Hai bạn đã là bạn bè từ trước!\n";
            } else if (currentStatus == "PENDING_SENT") {
                std::cout << "[THÔNG BÁO] Bạn đã gửi lời mời kết bạn cho người này rồi!\n";
            } else if (currentStatus == "PENDING_RECEIVED") {
                std::cout << "[THÔNG BÁO] Người này đã gửi lời mời cho bạn!\n";
            } else if (currentStatus == "BLOCK") {
                std::cout << "[LỖI] Không thể gửi lời mời do quan hệ bị chặn.\n";
            }
            std::cout << "-> Bạn có muốn nhập ID khác không? (y/n): ";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y') continue;
            else return;
        }

        std::string currentDate = "2026-10-18"; 

        std::ofstream outFile(path, std::ios::app);
        outFile << targetId << "|PENDING_SENT|" << currentDate << "\n";
        outFile.close();

        std::string targetFriendPath = "Data/Accounts/" + targetId + "/friend.txt";
        std::ofstream targetOut(targetFriendPath, std::ios::app);
        targetOut << currentUserId << "|PENDING_RECEIVED|" << currentDate << "\n";
        targetOut.close();

        std::cout << "[THÀNH CÔNG] Đã gửi lời mời kết bạn đến " << targetId << " thành công!\n";
        success = true;
    }
}

void FriendManager::acceptFriendRequest() {
    std::string targetId;
    std::cout << "\n=== CHẤP NHẬN LỜI MỜI KẾT BẠN ===\n";
    std::cout << "Nhập ID người muốn kết bạn cùng: ";
    std::cin >> targetId;

    if (!isValidTarget(targetId)) return;

    std::string currentDate = "2026-10-18";
    std::string path = "Data/Accounts/" + currentUserId + "/friend.txt";
    std::ifstream inFile(path);
    std::vector<std::string> lines;
    std::string line;
    bool found = false;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string id, status, date;
        std::getline(ss, id, '|');
        std::getline(ss, status, '|');
        std::getline(ss, date, '|');

        if (id == targetId && status == "PENDING_RECEIVED") {
            lines.push_back(targetId + "|FRIEND|" + currentDate);
            found = true;
        } else {
            lines.push_back(line);
        }
    }
    inFile.close();

    if (!found) {
        std::cout << "[LỖI] Không tìm thấy lời mời kết bạn nào từ ID này!\n";
        return;
    }

    std::ofstream outFile(path, std::ofstream::trunc);
    for (const auto& l : lines) outFile << l << "\n";
    outFile.close();

    loadFriendsData();
    std::cout << "[THÀNH CÔNG] Hai bạn đã trở thành bạn bè!\n";
}

std::vector<std::string> FriendManager::getMutualFriends(const std::string& targetUserId) {
    std::vector<std::string> mutualFriends;
    std::vector<std::string> myFriends = adjacencyList[currentUserId];

    std::vector<std::string> targetFriends;
    std::string targetFriendPath = "Data/Accounts/" + targetUserId + "/friend.txt";
    std::ifstream file(targetFriendPath);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id, status, date;
        std::getline(ss, id, '|');
        std::getline(ss, status, '|');
        std::getline(ss, date, '|');
        if (status == "FRIEND") {
            targetFriends.push_back(id);
        }
    }
    file.close();

    for (const auto& myF : myFriends) {
        if (std::find(targetFriends.begin(), targetFriends.end(), myF) != targetFriends.end()) {
            mutualFriends.push_back(myF);
        }
    }
    return mutualFriends;
}

void FriendManager::displayMutualFriendsUI() {
    std::string targetId;
    std::cout << "\n=== TÌM BẠN CHUNG ===\n";
    std::cout << "Nhập ID người dùng cần kiểm tra bạn chung: ";
    std::cin >> targetId;

    if (!isValidTarget(targetId)) return;

    std::vector<std::string> mutuals = getMutualFriends(targetId);
    std::cout << "Danh sách bạn chung giữa bạn và " << targetId << " (" << mutuals.size() << " người):\n";
    for (size_t i = 0; i < mutuals.size(); ++i) {
        std::cout << i + 1 << ". ID: " << mutuals[i] << "\n";
    }
}

void FriendManager::suggestFriends() {
    std::cout << "\n=== GỢI Ý BẠN BÈ TIỀM NĂNG ===\n";
    
    std::set<std::string> directFriends(adjacencyList[currentUserId].begin(), adjacencyList[currentUserId].end());
    std::set<std::string> visited;
    visited.insert(currentUserId);
    for (const auto& f : directFriends) visited.insert(f);

    std::map<std::string, int> candidateScores;

    for (const auto& friendId : directFriends) {
        std::string friendPath = "Data/Accounts/" + friendId + "/friend.txt";
        std::ifstream file(friendPath);
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string id, status, date;
            std::getline(ss, id, '|');
            std::getline(ss, status, '|');
            std::getline(ss, date, '|');

            if (status == "FRIEND" && visited.find(id) == visited.end()) {
                candidateScores[id]++;
            }
        }
        file.close();
    }

    std::vector<std::pair<std::string, int>> sortedCandidates(candidateScores.begin(), candidateScores.end());
    std::sort(sortedCandidates.begin(), sortedCandidates.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    if (sortedCandidates.empty()) {
        std::cout << "Hiện tại chưa có gợi ý bạn bè nào cho bạn.\n";
        return;
    }

    std::cout << "Danh sách gợi ý dành cho bạn:\n";
    int count = 1;
    for (const auto& candidate : sortedCandidates) {
        std::cout << count++ << ". ID: " << candidate.first 
                  << " (Có " << candidate.second << " bạn chung)\n";
        if (count > 5) break;
    }
}
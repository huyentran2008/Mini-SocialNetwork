// Module: Hồ sơ người dùng, thông tin hiển thị (đã bỏ phần đồ thị bạn bè)
#pragma once

/*
    TRANG CÁ NHÂN
                    */

// Thông tin đọc từ profile.txt (ngày tháng giữ nguyên như trong file)
struct UserProfile {
    string id;
    string username;
    string fullName;
    string birthday;
    string gender;
    string bio;
    string createdAt;
    string status;      // ACTIVE (hoạt động) hoặc LOCKED (bị quản trị viên khóa)
    string residence;   // Nơi ở hiện tại (tỉnh thành), có thể để trống
    string hometown;    // Quê quán (tỉnh thành), có thể để trống
    string school;      // Trường học, có thể để trống
    string workplace;   // Nơi làm việc, có thể để trống
    string hobbies;     // Sở thích (tối đa 3), cách nhau bằng dấu ';', có thể để trống
};
// Tách / ghép danh sách (sở thích lưu trong profile.txt cách nhau bằng dấu ';')
vector<string> splitList(const string& text, char separator = ';') {
    vector<string> result;
    string item;

    for (char c : text) {
        if (c == separator) {
            if (!item.empty()) result.push_back(item);
            item.clear();
        }
        else {
            item += c;
        }
    }

    if (!item.empty()) result.push_back(item);
    return result;
}
string joinList(const vector<string>& items, const string& separator) {
    string result;

    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) result += separator;
        result += items[i];
    }

    return result;
}
// ID hợp lệ có dạng ACC<số>, ví dụ ACC000001
bool isValidAccountId(const string& id) {
    if (id.size() < 4 || id.substr(0, 3) != "ACC") {
        return false;
    }

    return all_of(id.begin() + 3, id.end(),
                  [](unsigned char c) { return c >= '0' && c <= '9'; });
}
// Đọc 1 file profile.txt vào UserProfile
bool readProfileFile(const fs::path& profileFile, UserProfile& profile) {
    ifstream file(profileFile);

    if (!file.is_open()) {
        return false;
    }

    UserProfile current;
    string line;

    while (getline(file, line)) {
        // Bỏ ký tự '\r' nếu file có xuống dòng kiểu Windows
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "id")                current.id = value;
        else if (key == "username")     current.username = value;
        else if (key == "full_name")    current.fullName = value;
        else if (key == "birthday")     current.birthday = value;
        else if (key == "gender")       current.gender = value;
        else if (key == "bio")          current.bio = value;
        else if (key == "created_at")   current.createdAt = value;
        else if (key == "status")       current.status = value;
        else if (key == "residence")    current.residence = value;
        else if (key == "hometown")     current.hometown = value;
        else if (key == "school")       current.school = value;
        else if (key == "workplace")    current.workplace = value;
        else if (key == "hobbies")      current.hobbies = value;
    }

    // Hồ sơ cũ chưa có dòng status thì coi là đang hoạt động
    if (current.status != "LOCKED") {
        current.status = "ACTIVE";
    }

    // Nếu file thiếu dòng id thì lấy theo tên thư mục chứa nó
    if (current.id.empty()) {
        current.id = profileFile.parent_path().filename().string();
    }

    profile = current;
    return true;
}
// Tìm tài khoản theo username
bool findProfile(const string& username, UserProfile& profile) {
    fs::path accountDir = "Data/Account";
    error_code ec;

    if (!fs::is_directory(accountDir, ec) || ec) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(accountDir, ec)) {

        if (!entry.is_directory()) {
            continue;
        }

        if (!isValidAccountId(entry.path().filename().string())) {
            continue;
        }

        UserProfile current;

        if (!readProfileFile(entry.path() / "profile.txt", current)) {
            continue;
        }

        if (current.username == username) {
            profile = current;
            return true;
        }
    }

    return false;
}
// Tìm tài khoản theo ID
bool findProfileById(const string& accountId, UserProfile& profile) {
    if (!isValidAccountId(accountId)) {
        return false;
    }

    return readProfileFile(
        fs::path("Data/Account") / accountId / "profile.txt", profile);
}
// Đổi ngày lưu trong file sang dạng DD-MM-YYYY để hiển thị
// (nhận cả YYYY-MM-DD và DD-MM-YYYY, sai dạng thì trả về chuỗi rỗng)
string toDisplayDate(const string& s) {
    if (s.length() != 10) {
        return "";
    }

    // YYYY-MM-DD -> DD-MM-YYYY
    if (s[4] == '-' && s[7] == '-') {
        return s.substr(8, 2) + "-" + s.substr(5, 2) + "-" + s.substr(0, 4);
    }

    // Đã là DD-MM-YYYY
    if (s[2] == '-' && s[5] == '-') {
        return s;
    }

    return "";
}
// Tạo khối thông tin hiển thị của 1 hồ sơ: {nhãn, giá trị}
// mutualFriends >= 0 thì thêm dòng "Số Bạn Chung" (dùng khi xem trang của người khác)
// showEmpty = true: các mục tùy chọn bỏ trống vẫn hiện với chữ "(chưa có)" (dùng cho trang của tôi)
vector<pair<string, string>> buildProfileInfo(const UserProfile& profile,
                                              int mutualFriends = -1,
                                              bool showEmpty = false) {
    vector<pair<string, string>> info;

    info.push_back({"Tên", profile.fullName.empty() ? profile.username
                                                    : profile.fullName});

    // Bio
    if (!profile.bio.empty()) {
        info.push_back({"Bio", profile.bio});
    }
    else if (showEmpty) {
        info.push_back({"Bio", "(chưa có)"});
    }

    // Ngày sinh
    string birthday = toDisplayDate(profile.birthday);
    if (!birthday.empty()) {
        info.push_back({"Ngày Sinh", birthday});
    }
    else if (showEmpty) {
        info.push_back({"Ngày Sinh", "(chưa có)"});
    }

    // Giới tính
    if (!profile.gender.empty()) {
        info.push_back({"Giới Tính", profile.gender});
    }
    else if (showEmpty) {
        info.push_back({"Giới Tính", "(chưa chọn)"});
    }

    // Nơi ở, quê quán, trường học, nơi làm việc, sở thích (người khác chưa nhập thì không hiện)
    auto addOptional = [&](const string& label, const string& value, const string& emptyText) {
        if (!value.empty()) {
            info.push_back({label, value});
        }
        else if (showEmpty) {
            info.push_back({label, emptyText});
        }
    };

    addOptional("Nơi Ở Hiện Tại", profile.residence, "(chưa chọn)");
    addOptional("Quê Quán", profile.hometown, "(chưa chọn)");
    addOptional("Trường Học", profile.school, "(chưa chọn)");
    addOptional("Nơi Làm Việc", profile.workplace, "(chưa có)");
    addOptional("Sở Thích", joinList(splitList(profile.hobbies), ", "), "(chưa chọn)");

    info.push_back({"Số Bạn Bè", to_string(countFriends(profile.id))});

    if (mutualFriends >= 0) {
        info.push_back({"Số Bạn Chung", to_string(mutualFriends)});
    }

    string joined = toDisplayDate(profile.createdAt);
    info.push_back({"Tham Gia Từ", joined.empty() ? "Không rõ" : joined});

    return info;
}
// Chọn ngẫu nhiên 1 chuỗi trong danh sách (danh sách không được rỗng)
string pickRandom(const vector<string>& list) {
    static mt19937 rng(random_device{}());
    uniform_int_distribution<size_t> dist(0, list.size() - 1);

    return list[dist(rng)];
}
// Lời nhắn ngẫu nhiên khi chưa có bạn bè
string getNoFriendMessage() {
    static const vector<string> messages = {
        "Bạn chưa có người bạn nào. Hãy đi kết bạn với mọi người nhé!",
        "Danh sách trống trơn! Đi tìm bạn mới thôi nào!",
        "Chưa có ai ở đây cả. Hãy gửi lời mời kết bạn đến ai đó đi!",
        "Một mình thì buồn lắm, hãy kết bạn với người khác nhé!",
        "Mạng xã hội vui hơn khi có bạn bè. Đi kết bạn ngay nào!",
        "Chưa có bạn bè nào. Hãy làm quen với một vài người mới!",
        "Danh sách bạn bè đang chờ bạn lấp đầy. Kết bạn thôi!",
        "Đừng ngại ngùng, hãy gửi lời mời kết bạn cho ai đó!",
        "Bạn bè chưa có, nhưng cơ hội thì đầy. Đi kết bạn nào!",
        "Hãy khám phá và kết bạn với những người thú vị xung quanh bạn!",
        "Chưa có ai trong danh sách. Bắt đầu kết bạn đầu tiên nào!",
        "Một người bạn mới đang chờ bạn ở đâu đó. Đi tìm họ nhé!"
    };

    return pickRandom(messages);
}

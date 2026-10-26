// Module: Tiện ích văn bản, thời gian, file
#pragma once

/*
    ================================================================
    PHẦN MỞ RỘNG: TIỆN ÍCH, MÀN HÌNH ĐỌC CÓ CUỘN, BẠN BÈ, BÀI ĐĂNG,
    TIN NHẮN, NHÓM CHAT
    ================================================================
                                                                        */

// Khai báo trước các màn hình gọi chéo nhau
void viewUserProfile(const string& myId, const string& otherId);
void viewUserPosts(const string& myId, const string& otherId);
void openPrivateChat(const string& myId, const string& otherId);

/*
    TIỆN ÍCH VĂN BẢN, THỜI GIAN, FILE
                                        */

// Lấy tối đa n ký tự UTF-8 đầu chuỗi
string cutChars(const string& s, int n) {
    if (n <= 0) return "";

    int count = 0;
    size_t i = 0;

    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];

        if ((c & 0xC0) != 0x80) {
            if (count == n) break;
            count++;
        }

        i++;
    }

    return s.substr(0, i);
}
// Lấy tối đa n ký tự UTF-8 cuối chuỗi
string tailChars(const string& s, int n) {
    int len = textLength(s);
    if (len <= n) return s;

    int skip = len - n;
    int count = 0;
    size_t i = 0;

    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];

        if ((c & 0xC0) != 0x80) {
            if (count == skip) break;
            count++;
        }

        i++;
    }

    return s.substr(i);
}
// Cắt chuỗi cho vừa maxLen ký tự, thêm "..." nếu bị cắt
string cutText(const string& s, int maxLen) {
    if (textLength(s) <= maxLen) return s;
    if (maxLen <= 3) return cutChars(s, maxLen);

    return cutChars(s, maxLen - 3) + "...";
}
// Cắt khoảng trắng và xuống dòng ở đầu và cuối chuỗi
string trimBlank(const string& s) {
    size_t begin = s.find_first_not_of(" \t\r\n");

    if (begin == string::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}
// Đổi xuống dòng thành khoảng trắng (dùng để xem trước nội dung trên 1 dòng)
string oneLine(const string& s) {
    string result;

    for (char c : s) {
        result += (c == '\n' || c == '\r') ? ' ' : c;
    }

    return result;
}
// Chữ thường cho các ký tự ASCII, giữ nguyên ký tự khác
string lowerAscii(const string& s) {
    string result = s;

    for (char& c : result) {
        unsigned char u = (unsigned char)c;
        if (u < 128) c = (char)tolower(u);
    }

    return result;
}
// Số byte của ký tự UTF-8 bắt đầu tại vị trí i và mã Unicode của nó
unsigned int decodeAt(const string& s, size_t i, int& length) {
    unsigned char c = (unsigned char)s[i];
    unsigned int cp = c;
    int extra = 0;

    if (c >= 0xF0)      { cp = c & 0x07; extra = 3; }
    else if (c >= 0xE0) { cp = c & 0x0F; extra = 2; }
    else if (c >= 0xC0) { cp = c & 0x1F; extra = 1; }

    if (extra > 0 && i + extra >= s.size()) {
        // Chuỗi bị cụt: coi như 1 byte
        length = 1;
        return c;
    }

    for (int k = 1; k <= extra; k++) {
        cp = (cp << 6) | ((unsigned char)s[i + k] & 0x3F);
    }

    length = extra + 1;
    return cp;
}
// Chữ thường và bỏ dấu tiếng Việt (dùng để tìm kiếm không phân biệt dấu)
string foldText(const string& s) {
    static map<unsigned int, char> table;

    if (table.empty()) {
        const vector<pair<char, string>> groups = {
            {'a', "àáạảãâầấậẩẫăằắặẳẵÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴ"},
            {'e', "èéẹẻẽêềếệểễÈÉẸẺẼÊỀẾỆỂỄ"},
            {'i', "ìíịỉĩÌÍỊỈĨ"},
            {'o', "òóọỏõôồốộổỗơờớợởỡÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠ"},
            {'u', "ùúụủũưừứựửữÙÚỤỦŨƯỪỨỰỬỮ"},
            {'y', "ỳýỵỷỹỲÝỴỶỸ"},
            {'d', "đĐ"}
        };

        for (const auto& group : groups) {
            size_t i = 0;

            while (i < group.second.size()) {
                int length = 1;
                unsigned int cp = decodeAt(group.second, i, length);
                table[cp] = group.first;
                i += length;
            }
        }
    }

    string result;
    size_t i = 0;

    while (i < s.size()) {
        int length = 1;
        unsigned int cp = decodeAt(s, i, length);

        auto it = table.find(cp);

        if (it != table.end()) {
            result += it->second;
        }
        else if (cp < 128) {
            result += (char)tolower((int)cp);
        }
        else {
            result += s.substr(i, length);
        }

        i += length;
    }

    return result;
}
// Ngắt 1 đoạn (không có xuống dòng) thành các dòng không dài quá width ký tự
vector<string> wrapParagraph(const string& p, int width) {
    vector<string> lines;
    string line;
    int lineLen = 0;
    size_t i = 0;

    while (i < p.size()) {
        // Lấy 1 từ (đến khoảng trắng kế tiếp)
        size_t j = p.find(' ', i);
        string word = (j == string::npos) ? p.substr(i) : p.substr(i, j - i);
        i = (j == string::npos) ? p.size() : j + 1;

        if (word.empty()) continue;

        int wordLen = textLength(word);

        // Từ dài hơn cả dòng: chia nhỏ
        while (wordLen > width) {
            if (lineLen > 0) {
                lines.push_back(line);
                line.clear();
                lineLen = 0;
            }

            string head = cutChars(word, width);
            lines.push_back(head);
            word = word.substr(head.size());
            wordLen -= width;
        }

        if (lineLen == 0) {
            line = word;
            lineLen = wordLen;
        }
        else if (lineLen + 1 + wordLen <= width) {
            line += " " + word;
            lineLen += 1 + wordLen;
        }
        else {
            lines.push_back(line);
            line = word;
            lineLen = wordLen;
        }
    }

    if (lineLen > 0 || lines.empty()) {
        lines.push_back(line);
    }

    return lines;
}
// Ngắt cả đoạn văn (có thể nhiều dòng) thành các dòng vừa độ rộng width
vector<string> wrapText(const string& text, int width) {
    if (width < 1) width = 1;

    vector<string> result;
    size_t start = 0;

    while (true) {
        size_t pos = text.find('\n', start);

        string paragraph = (pos == string::npos) ? text.substr(start)
                                                 : text.substr(start, pos - start);

        if (!paragraph.empty() && paragraph.back() == '\r') {
            paragraph.pop_back();
        }

        for (const string& line : wrapParagraph(paragraph, width)) {
            result.push_back(line);
        }

        if (pos == string::npos) break;
        start = pos + 1;
    }

    return result;
}
// Mã hóa nội dung nhiều dòng thành 1 dòng để lưu file (\ -> \\, xuống dòng -> \n)
string escapeText(const string& s) {
    string result;

    for (char c : s) {
        if (c == '\\')      result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') continue;
        else                result += c;
    }

    return result;
}
// Giải mã ngược lại của escapeText
string unescapeText(const string& s) {
    string result;

    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i + 1] == 'n') {
                result += '\n';
                i++;
                continue;
            }

            if (s[i + 1] == '\\') {
                result += '\\';
                i++;
                continue;
            }
        }

        result += s[i];
    }

    return result;
}
// Ngày hiện tại dạng YYYY-MM-DD
string todayIso() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay);

    return buffer;
}
// Thời điểm hiện tại dạng "YYYY-MM-DD HH:MM:SS"
string nowTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay,
             (int)st.wHour, (int)st.wMinute, (int)st.wSecond);

    return buffer;
}
// Đọc chuỗi "YYYY-MM-DD HH:MM:SS" thành time_t
bool parseTimestamp(const string& s, time_t& result) {
    if (s.size() < 19 || s[4] != '-' || s[7] != '-' || s[10] != ' ' ||
        s[13] != ':' || s[16] != ':') {
        return false;
    }

    int y, mo, d, h, mi, se;

    if (sscanf(s.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &y, &mo, &d, &h, &mi, &se) != 6) {
        return false;
    }

    tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = mo - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = mi;
    t.tm_sec = se;
    t.tm_isdst = -1;

    time_t value = mktime(&t);

    if (value == (time_t)-1) {
        return false;
    }

    result = value;
    return true;
}
// "YYYY-MM-DD..." -> "DD/MM/YYYY"
string toSlashDate(const string& iso) {
    if (iso.size() < 10) return "";

    return iso.substr(8, 2) + "/" + iso.substr(5, 2) + "/" + iso.substr(0, 4);
}
// Thời gian hiển thị: "x phút / giờ / ngày / tháng trước", quá 1 năm thì hiện dd/mm/yyyy
string timeAgo(const string& timestamp) {
    time_t then;

    if (!parseTimestamp(timestamp, then)) {
        return "";
    }

    long long diff = (long long)difftime(time(nullptr), then);

    if (diff < 60)    return "Vừa xong";
    if (diff < 3600)  return to_string(diff / 60) + " phút trước";
    if (diff < 86400) return to_string(diff / 3600) + " giờ trước";

    long long days = diff / 86400;

    if (days < 30)  return to_string(days) + " ngày trước";
    if (days < 365) return to_string(days / 30) + " tháng trước";

    return toSlashDate(timestamp);
}
// Đọc tất cả các dòng của 1 file (bỏ '\r'); file không có thì trả về danh sách rỗng
vector<string> readLines(const fs::path& path) {
    vector<string> lines;
    ifstream file(path);

    if (!file.is_open()) {
        return lines;
    }

    string line;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        lines.push_back(line);
    }

    return lines;
}
// Ghi đè toàn bộ file bằng các dòng
bool writeLines(const fs::path& path, const vector<string>& lines) {
    ofstream file(path, ios::trunc);

    if (!file.is_open()) {
        return false;
    }

    for (const string& line : lines) {
        file << line << '\n';
    }

    file.close();
    return !file.fail();
}
// Thêm 1 dòng vào cuối file (tự tạo thư mục nếu chưa có)
bool appendLine(const fs::path& path, const string& line) {
    error_code ec;
    fs::create_directories(path.parent_path(), ec);

    ofstream file(path, ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << line << '\n';
    file.close();
    return !file.fail();
}
// Tạo ID mới cho thư mục dạng <prefix><số>: lấy số lớn nhất rồi cộng 1 (đệm đủ 6 chữ số)
string nextId(const fs::path& dir, const string& prefix) {
    error_code ec;
    unsigned long long maxNumber = 0;

    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (name.size() <= prefix.size() || name.compare(0, prefix.size(), prefix) != 0) {
            continue;
        }

        string digits = name.substr(prefix.size());

        bool isValid = all_of(digits.begin(), digits.end(),
                              [](unsigned char c) { return c >= '0' && c <= '9'; });

        if (!isValid || digits.size() > 18) {
            continue;
        }

        unsigned long long number = stoull(digits);

        if (number > maxNumber) {
            maxNumber = number;
        }
    }

    string number = to_string(maxNumber + 1);

    if (number.length() < 6) {
        number = string(6 - number.length(), '0') + number;
    }

    return prefix + number;
}
// Danh sách ID của mọi tài khoản (đã sắp xếp)
vector<string> listAccountIds() {
    vector<string> ids;
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Account", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (isValidAccountId(name)) {
            ids.push_back(name);
        }
    }

    sort(ids.begin(), ids.end());
    return ids;
}
// ID tài khoản từ username ("" nếu không có)
string getAccountId(const string& username) {
    UserProfile profile;

    if (!findProfile(username, profile)) {
        return "";
    }

    return profile.id;
}
// Tên hiển thị của 1 tài khoản (họ tên, nếu trống thì dùng username)
string getDisplayName(const string& accountId) {
    UserProfile profile;

    if (!findProfileById(accountId, profile)) {
        return "Người dùng đã xóa";
    }

    return profile.fullName.empty() ? profile.username : profile.fullName;
}
// Độ rộng cửa sổ console hiện tại
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 80;
    }

    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}
// Hiện 1 thông báo giữa màn hình trong 2 giây
void notify(const string& message) {
    printCenter(message);
    sleep_for(2s);
}


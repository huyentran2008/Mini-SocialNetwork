// Module: Tiện ích chuỗi UTF-8, ngày hiện tại, mã hóa Hex
#pragma once

// Đếm số ký tự UTF-8
int textLength(const string& s) {
    int len = 0;
    for (unsigned char c : s) {
        if((c & 0xC0) != 0x80)
            len++;
    }
    return len;
}
// Xóa 1 ký tự UTF-8 cuối chuỗi (chữ có dấu chiếm nhiều byte)
void popLastChar(string& s) {
    if (s.empty()) return;

    // Lùi về byte đầu của ký tự UTF-8 cuối cùng
    size_t i = s.size() - 1;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
        i--;
    }

    s.erase(i);
}
// Cắt khoảng trắng ở đầu và cuối chuỗi
string trimString(const string& s) {
    size_t begin = s.find_first_not_of(" \t");

    if (begin == string::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t");
    return s.substr(begin, end - begin + 1);
}
// Lấy ngày hiện tại dạng DD-MM-YYYY
string todayString() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d",
             (int)st.wDay, (int)st.wMonth, (int)st.wYear);

    return buffer;
}
// Hàm chuyển đổi chuỗi thông thường sang chuỗi Hex
string stringToHex(const string& input) {
    const char HEX[] = "0123456789abcdef";
    string result;

    for (unsigned char c : input) {
        result += HEX[c >> 4];
        result += HEX[c & 0x0F];
    }

    return result;
}
// Hàm chuyển đổi chuỗi Hex thành chuỗi văn bản thông thường
string hexToString(const string& hexInput) {
    if (hexInput.length() % 2 != 0) {
        return ""; // Trả về rỗng báo hiệu chuỗi sai định dạng
    }

    string result = "";

    // Duyệt qua chuỗi Hex, mỗi bước nhảy 2 ký tự
    for (size_t i = 0; i < hexInput.length(); i += 2) {
        // Cắt cụm 2 ký tự Hex liên tiếp (ví dụ: "47", "6E")
        string byteString = hexInput.substr(i, 2);

        try {
            char ch = static_cast<char>(stoi(byteString, nullptr, 16));
            result += ch;
        } catch (...) {
            // Xử lý lỗi nếu chuỗi chứa ký tự không thuộc hệ Hex (không nằm trong 0-9, A-F)
            return "";
        }
    }

    return result;
}

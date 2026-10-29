// Module: Console, nhận phím, double buffer
#pragma once

// Thư viện
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <clocale>
#include <cctype>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <map>
#include <functional>
#include <random>
#include <ctime>
#include <climits>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono_literals;
namespace fs = filesystem;

/*
    BIẾN TOÀN CỤC CHO DOUBLE BUFFERING
                                        */
HANDLE hOriginal = GetStdHandle(STD_OUTPUT_HANDLE); // Buffer gốc của console
HANDLE hConsole  = hOriginal;                       // Buffer đang dùng để VẼ (back buffer)
HANDLE hFront    = nullptr;                         // Buffer đang HIỂN THỊ (front buffer)
bool   bufferReady = false;                         // Đã tạo double buffer thành công chưa

HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);     // Luồng nhập của console

DWORD originalInputMode = 0;                        // Chế độ nhập gốc của console
bool  inputModeSaved = false;

// Danh sách giới tính được phép chọn
const vector<string> GENDER_OPTIONS = {"Male", "Female", "Other"};

/*
    CHỨC NĂNG NHẬN PHÍM TỪ NGƯỜI DÙNG
                                        */
// Mã phím
enum KeyType {
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_NEWLINE,        // Shift+ENTER hoặc Ctrl+ENTER (xuống dòng khi soạn nội dung)
    KEY_SCROLL_UP,      // Lăn chuột lên
    KEY_SCROLL_DOWN,    // Lăn chuột xuống
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CHARACTER,
    KEY_TICK,           // Hết thời gian chờ mà chưa có phím nào (dùng để tự cập nhật màn hình)
    KEY_TAB,
    KEY_DELETE,
    KEY_F2
};
// Thông tin phím vừa nhận
struct KeyInput {
    KeyType type;
    int asciiCode;      // Mã ký tự (>= 128 nếu là ký tự Unicode, ví dụ chữ có dấu)
    string character;   // Ký tự dạng UTF-8
};
// Chuyển chuỗi UTF-16 sang UTF-8
string wideToUtf8(const wstring& w) {
    if (w.empty()) return "";

    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                                nullptr, 0, nullptr, nullptr);
    if (n <= 0) return "";

    string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                        &s[0], n, nullptr, nullptr);
    return s;
}
// Hàm nhận 1 phím từ bàn phím (hỗ trợ cả chữ tiếng Việt có dấu)
// timeoutMs < 0: chờ mãi đến khi có phím. timeoutMs >= 0: sau chừng đó mili giây mà chưa có phím
// thì trả về KEY_TICK để màn hình có dịp tự cập nhật (ví dụ tin nhắn mới)
KeyInput readKey(int timeoutMs = -1) {
    KeyInput result = {KEY_NONE, 0, ""};
    auto startTime = chrono::steady_clock::now();
    static wchar_t highSurrogate = 0;   // Nửa đầu của ký tự 2 đơn vị (emoji...)

    INPUT_RECORD record;
    DWORD count = 0;

    // Chờ đến khi có sự kiện nhấn phím (bỏ qua chuột, đổi kích thước, nhả phím...)
    while (true) {
        if (timeoutMs >= 0) {
            long long elapsed = chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - startTime).count();
            long long remaining = timeoutMs - elapsed;

            // Hết giờ, hoặc chờ đến hết giờ mà vẫn chưa có sự kiện nào
            if (remaining <= 0 ||
                WaitForSingleObject(hInput, (DWORD)remaining) == WAIT_TIMEOUT) {
                result.type = KEY_TICK;
                return result;
            }
        }

        if (!ReadConsoleInputW(hInput, &record, 1, &count) || count == 0) {
            return result;
        }

        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            break;
        }

        // Lăn chuột (các sự kiện chuột khác được bỏ qua)
        if (record.EventType == MOUSE_EVENT) {
            const MOUSE_EVENT_RECORD& me = record.Event.MouseEvent;

            if (me.dwEventFlags & MOUSE_WHEELED) {
                short delta = (short)HIWORD(me.dwButtonState);
                result.type = (delta > 0) ? KEY_SCROLL_UP : KEY_SCROLL_DOWN;
                return result;
            }
        }
    }

    const KEY_EVENT_RECORD& ke = record.Event.KeyEvent;

    // Các phím điều khiển
    switch (ke.wVirtualKeyCode) {
        case VK_TAB:
            result.type = KEY_TAB;
            return result;
        case VK_DELETE:
            result.type = KEY_DELETE;
            return result;
        case VK_F2:
            result.type = KEY_F2;
            return result;
        case VK_UP:
            result.type = KEY_UP;
            return result;

        case VK_DOWN:
            result.type = KEY_DOWN;
            return result;

        case VK_LEFT:
            result.type = KEY_LEFT;
            return result;

        case VK_RIGHT:
            result.type = KEY_RIGHT;
            return result;

        case VK_RETURN:
            // Shift+ENTER hoặc Ctrl+ENTER: xuống dòng
            if (ke.dwControlKeyState & (SHIFT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                result.type = KEY_NEWLINE;
            }
            else {
                result.type = KEY_ENTER;
            }
            return result;

        case VK_ESCAPE:
            result.type = KEY_ESCAPE;
            return result;

        case VK_BACK:
            result.type = KEY_BACKSPACE;
            return result;

        case VK_PRIOR:
            result.type = KEY_PAGE_UP;
            return result;

        case VK_NEXT:
            result.type = KEY_PAGE_DOWN;
            return result;

        case VK_HOME:
            result.type = KEY_HOME;
            return result;

        case VK_END:
            result.type = KEY_END;
            return result;
    }

    // Ký tự thông thường
    wchar_t ch = ke.uChar.UnicodeChar;

    // Nửa đầu của cặp surrogate: lưu lại, chờ nửa sau
    if (ch >= 0xD800 && ch <= 0xDBFF) {
        highSurrogate = ch;
        return result;
    }

    wstring wide;

    if (ch >= 0xDC00 && ch <= 0xDFFF) {
        if (highSurrogate == 0) return result;

        wide = {highSurrogate, ch};
        highSurrogate = 0;
    }
    else {
        highSurrogate = 0;

        // Bỏ qua ký tự điều khiển (Tab, Ctrl+...)
        if (ch < 32 || ch == 127) return result;

        wide = wstring(1, ch);
    }

    result.character = wideToUtf8(wide);

    if (!result.character.empty()) {
        result.type = KEY_CHARACTER;
        result.asciiCode = (wide.size() == 1) ? (int)ch : 0x10000;
    }

    return result;
}

// Bật nhận sự kiện lăn chuột (tắt Quick Edit vì nó chặn sự kiện chuột)
void enableMouseInput() {
    DWORD mode = 0;

    if (!GetConsoleMode(hInput, &mode)) {
        return;
    }

    originalInputMode = mode;
    inputModeSaved = true;

    mode |= ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE;

    SetConsoleMode(hInput, mode);
}
// Khôi phục chế độ nhập gốc của console khi thoát
void restoreInputMode() {
    if (inputModeSaved) {
        SetConsoleMode(hInput, originalInputMode);
        inputModeSaved = false;
    }
}

/*
    CÁC HÀM HỖ TRỢ
                    */

// Ẩn con trỏ console
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;

    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // Ẩn luôn ở buffer đang hiển thị để không bị sót con trỏ ở khung hình kế tiếp
    if (bufferReady && hFront != nullptr) {
        SetConsoleCursorInfo(hFront, &cursorInfo);
    }
}
// Hiện con trỏ console
void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;

    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
// Thay đổi màu sắc chữ
void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}
// Di chuyển con trỏ đến vị trí (x, y)
void gotoxy(int x, int y) {
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, pos);
}
// Xóa màn hình bằng Windows API (nhanh hơn system("cls"))
void clearScreen() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written;
    COORD home = {0, 0};

    // Ghi khoảng trắng và đặt lại màu mặc định (7) cho toàn bộ buffer
    FillConsoleOutputCharacterA(hConsole, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(hConsole, 7, cellCount, home, &written);

    SetConsoleTextAttribute(hConsole, 7);
    SetConsoleCursorPosition(hConsole, home);
}
// Ghi chuỗi UTF-8 vào buffer đang vẽ (thay cho cout)
void writeText(const string& s) {
    if(s.empty()) return;

    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if(n <= 0) return;

    wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);

    DWORD written;
    WriteConsoleW(hConsole, w.data(), (DWORD)w.size(), &written, nullptr);
}

/*
    DOUBLE BUFFERING
                        */

// Tạo 2 screen buffer. Nếu lỗi thì giữ nguyên buffer gốc (không double buffer)
void initDoubleBuffer() {
    HANDLE a = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);

    HANDLE b = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);

    if(a == INVALID_HANDLE_VALUE || b == INVALID_HANDLE_VALUE) {
        if(a != INVALID_HANDLE_VALUE) CloseHandle(a);
        if(b != INVALID_HANDLE_VALUE) CloseHandle(b);
        return;
    }

    // Ẩn con trỏ ở cả 2 buffer (con trỏ là thuộc tính riêng của từng buffer)
    hConsole = a;
    hideCursor();
    hConsole = b;
    hideCursor();

    // Vẽ vào b trước, a là buffer sẽ bị hoán đổi sau khung hình đầu tiên.
    // Chưa kích hoạt buffer nào ở đây, nên nếu chưa vẽ gì thì người dùng
    // vẫn thấy buffer gốc (cout vẫn hoạt động bình thường).
    hFront = a;
    hConsole = b;
    bufferReady = true;
}

// Bắt đầu 1 khung hình: xóa back buffer (người dùng không thấy)
void beginFrame() {
    clearScreen();
}

// Kết thúc khung hình: hiển thị back buffer rồi hoán đổi vai trò 2 buffer
void endFrame() {
    if(!bufferReady) return;

    SetConsoleActiveScreenBuffer(hConsole); // Hiện buffer vừa vẽ xong
    swap(hConsole, hFront);                 // Buffer cũ trở thành back buffer
}

// Khôi phục buffer gốc khi thoát chương trình
void cleanupDoubleBuffer() {
    if(!bufferReady) return;

    SetConsoleActiveScreenBuffer(hOriginal);
    CloseHandle(hConsole);
    CloseHandle(hFront);

    hConsole = hOriginal;
    hFront = nullptr;
    bufferReady = false;

    // Dọn buffer gốc và đưa con trỏ về như bình thường
    clearScreen();
    showCursor();
}


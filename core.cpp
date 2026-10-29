#include "core.h"

// ===== Biáº¿n toÃ n cá»¥c cá»§a console =====
HANDLE hOriginal = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE hConsole  = hOriginal;   // buffer Ä‘ang Ä‘Æ°á»£c váº½
HANDLE hFront    = nullptr;     // buffer hiá»ƒn thá»‹ (double buffer)
bool   bufferReady = false;
HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);   // Luá»“ng nháº­p cá»§a console
DWORD  originalInputMode = 0;                     // Cháº¿ Ä‘á»™ nháº­p gá»‘c cá»§a console
bool   inputModeSaved = false;

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

KeyInput readKey(int timeoutMs) {
    KeyInput result = {KEY_NONE, 0, ""};
    auto startTime = chrono::steady_clock::now();
    static wchar_t highSurrogate = 0;

    INPUT_RECORD record;
    DWORD count = 0;

    while (true) {
        if (timeoutMs >= 0) {
            long long elapsed = chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - startTime).count();
            long long remaining = timeoutMs - elapsed;

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

    wchar_t ch = ke.uChar.UnicodeChar;

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

// Báº­t nháº­n sá»± kiá»‡n lÄƒn chuá»™t (táº¯t Quick Edit vÃ¬ nÃ³ cháº·n sá»± kiá»‡n chuá»™t)
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

// KhÃ´i phá»¥c cháº¿ Ä‘á»™ nháº­p gá»‘c cá»§a console khi thoÃ¡t
void restoreInputMode() {
    if (inputModeSaved) {
        SetConsoleMode(hInput, originalInputMode);
        inputModeSaved = false;
    }
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;

    SetConsoleCursorInfo(hConsole, &cursorInfo);

    if (bufferReady && hFront != nullptr) {
        SetConsoleCursorInfo(hFront, &cursorInfo);
    }
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;

    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void gotoxy(int x, int y) {
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, pos);
}

void clearScreen() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written;
    COORD home = {0, 0};

    FillConsoleOutputCharacterA(hConsole, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(hConsole, 7, cellCount, home, &written);

    SetConsoleTextAttribute(hConsole, 7);
    SetConsoleCursorPosition(hConsole, home);
}

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

// Táº¡o 2 screen buffer. Náº¿u lá»—i thÃ¬ giá»¯ nguyÃªn buffer gá»‘c (khÃ´ng double buffer)
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

    // áº¨n con trá» á»Ÿ cáº£ 2 buffer (con trá» lÃ  thuá»™c tÃ­nh riÃªng cá»§a tá»«ng buffer)
    hConsole = a;
    hideCursor();
    hConsole = b;
    hideCursor();

    // Váº½ vÃ o b trÆ°á»›c, a lÃ  buffer sáº½ bá»‹ hoÃ¡n Ä‘á»•i sau khung hÃ¬nh Ä‘áº§u tiÃªn.
    // ChÆ°a kÃ­ch hoáº¡t buffer nÃ o á»Ÿ Ä‘Ã¢y, nÃªn náº¿u chÆ°a váº½ gÃ¬ thÃ¬ ngÆ°á»i dÃ¹ng
    // váº«n tháº¥y buffer gá»‘c (cout váº«n hoáº¡t Ä‘á»™ng bÃ¬nh thÆ°á»ng).
    hFront = a;
    hConsole = b;
    bufferReady = true;
}

void beginFrame() {
    clearScreen();
}

void endFrame() {
    if(!bufferReady) return;

    SetConsoleActiveScreenBuffer(hConsole);
    swap(hConsole, hFront);
}

// KhÃ´i phá»¥c buffer gá»‘c khi thoÃ¡t chÆ°Æ¡ng trÃ¬nh
void cleanupDoubleBuffer() {
    if(!bufferReady) return;

    SetConsoleActiveScreenBuffer(hOriginal);
    CloseHandle(hConsole);
    CloseHandle(hFront);

    hConsole = hOriginal;
    hFront = nullptr;
    bufferReady = false;

    // Dá»n buffer gá»‘c vÃ  Ä‘Æ°a con trá» vá» nhÆ° bÃ¬nh thÆ°á»ng
    clearScreen();
    showCursor();
}

int textLength(const string& s) {
    int len = 0;
    for (unsigned char c : s) {
        if((c & 0xC0) != 0x80)
            len++;
    }
    return len;
}

void popLastChar(string& s) {
    if (s.empty()) return;

    size_t i = s.size() - 1;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
        i--;
    }

    s.erase(i);
}

string trimString(const string& s) {
    size_t begin = s.find_first_not_of(" \t");

    if (begin == string::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t");
    return s.substr(begin, end - begin + 1);
}

void runLoop(
    const function<void()>& draw,
    const function<bool(const KeyInput&)>& handleKey,
    int tickMs,
    const function<bool()>& onTick
) {
    bool shouldExit = false;
    bool needRedraw = true;

    while (!shouldExit) {
        if (needRedraw) {
            draw();
        }

        KeyInput key = readKey(onTick ? tickMs : -1);

        if (key.type == KEY_TICK) {
            needRedraw = onTick();
            continue;
        }

        needRedraw = (key.type != KEY_NONE);

        shouldExit = handleKey(key);
    }
}

bool moveSelection(int& selected, int itemCount, KeyType type) {
    if(type == KEY_UP) {
        selected--;
        if(selected < 1) selected = itemCount;
        return true;
    }

    if(type == KEY_DOWN) {
        selected++;
        if(selected > itemCount) selected = 1;
        return true;
    }

    return false;
}

void printCenter(const string& s, int y, int color,
                 int width, int left) {
    int x = left + (width - textLength(s)) / 2;
    if(x < left) x = left;

    gotoxy(x, y);
    setColor(color);
    writeText(s);
}

void printCenter(const string& s) {
    beginFrame();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int width = csbi.srWindow.Right
              - csbi.srWindow.Left + 1;

    int height = csbi.srWindow.Bottom
               - csbi.srWindow.Top + 1;

    int x = csbi.srWindow.Left
          + (width - textLength(s)) / 2;

    int y = csbi.srWindow.Top
          + (height - 1) / 2;

    if(x < csbi.srWindow.Left)
        x = csbi.srWindow.Left;

    gotoxy(x, y);
    setColor(2);
    writeText(s);
    setColor(7);

    endFrame();
}

void drawMenu(
    int selected,
    const string& title,
    const vector<string>& items,
    const string& footer
) {
    beginFrame();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int width = csbi.srWindow.Right
              - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom
               - csbi.srWindow.Top + 1;

    int left = csbi.srWindow.Left;
    int top = csbi.srWindow.Top;

    int totalLines = (int)items.size() + 2;
    int startY = top + (height - totalLines) / 2;

    // Title
    printCenter(title, startY, 11, width, left);

    // Menu items
    for (int i = 0; i < (int)items.size(); i++) {
        string text = items[i];

        if(selected == i + 1) {
            text = "> " + text + " <";
            printCenter(text, startY + i + 1, 14,
                        width, left);
        } else {
            printCenter(text, startY + i + 1, 7,
                        width, left);
        }
    }

    // Footer
    printCenter(footer, startY + totalLines - 1,
                10, width, left);

    setColor(7);
    endFrame();
}

// Váº½ menu cÃ³ thÃªm khá»‘i thÃ´ng tin chá»‰ Ä‘á»ƒ xem (nhÃ£n, giÃ¡ trá»‹) á»Ÿ phÃ­a trÃªn
void drawMenuWithInfo(
    int selected,
    const string& title,
    const vector<pair<string, string>>& info,
    const vector<string>& items,
    const string& footer
) {
    beginFrame();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int width = csbi.srWindow.Right
              - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom
               - csbi.srWindow.Top + 1;

    int left = csbi.srWindow.Left;
    int top = csbi.srWindow.Top;

    // Äá»™ rá»™ng lá»›n nháº¥t cá»§a nhÃ£n Ä‘á»ƒ cÄƒn tháº³ng cá»™t dáº¥u ':'
    int maxLabel = 0;
    for (const auto& line : info) {
        int len = textLength(line.first);
        if (len > maxLabel) maxLabel = len;
    }

    // Äá»™ rá»™ng cá»§a cáº£ khá»‘i thÃ´ng tin
    int blockWidth = 0;
    for (const auto& line : info) {
        int len = maxLabel + 3 + textLength(line.second);
        if (len > blockWidth) blockWidth = len;
    }

    // TiÃªu Ä‘á» + dÃ²ng trá»‘ng + thÃ´ng tin + Ä‘Æ°á»ng káº» + cÃ¡c má»¥c + dÃ²ng trá»‘ng + footer
    int totalLines = 1 + 1 + (int)info.size() + 1
                   + (int)items.size() + 1 + 1;
    int startY = top + (height - totalLines) / 2;
    if (startY < top) startY = top;

    int y = startY;

    // TiÃªu Ä‘á»
    printCenter(title, y++, 11, width, left);
    y++;

    // Khá»‘i thÃ´ng tin: cáº£ khá»‘i Ä‘Æ°á»£c cÄƒn giá»¯a, cÃ¡c dÃ²ng cÄƒn trÃ¡i theo khá»‘i
    int infoX = left + (width - blockWidth) / 2;
    if (infoX < left) infoX = left;

    for (const auto& line : info) {
        string label = line.first
                     + string(maxLabel - textLength(line.first), ' ')
                     + " : ";

        gotoxy(infoX, y++);
        setColor(3);            // NhÃ£n: mÃ u xanh lÆ¡
        writeText(label);
        setColor(15);           // GiÃ¡ trá»‹: mÃ u tráº¯ng sÃ¡ng
        writeText(line.second);
    }

    // ÄÆ°á»ng káº» ngÄƒn cÃ¡ch pháº§n thÃ´ng tin vÃ  pháº§n lá»±a chá»n
    int separatorLength = textLength(title);
    if (blockWidth > separatorLength) separatorLength = blockWidth;

    printCenter(string(separatorLength, '-'), y++, 8, width, left);

    // CÃ¡c má»¥c chá»n Ä‘Æ°á»£c
    for (int i = 0; i < (int)items.size(); i++) {
        if(selected == i + 1) {
            printCenter("> " + items[i] + " <", y++, 14, width, left);
        } else {
            printCenter(items[i], y++, 7, width, left);
        }
    }

    // Footer
    y++;
    printCenter(footer, y, 10, width, left);

    setColor(7);
    endFrame();
}

// Hiá»ƒn thá»‹ menu cÃ³ khá»‘i thÃ´ng tin vÃ  nháº­n lá»±a chá»n
int menuWithInfo(
    const string& title,
    const vector<pair<string, string>>& info,
    const vector<string>& items,
    const string& footer
) {
    if(items.empty()) return 0;

    int selected = 1;
    int result = 0;

    runLoop(
        // Váº½ giao diá»‡n
        [&]() {
            drawMenuWithInfo(selected, title, info, items, footer);
        },
        // Xá»­ lÃ½ phÃ­m
        [&](const KeyInput& key) -> bool {
            if(moveSelection(selected, (int)items.size(), key.type)) {
                return false;
            }

            if(key.type == KEY_ENTER) {
                result = selected;
                return true;
            }

            if(key.type == KEY_ESCAPE) {
                result = 0;
                return true;
            }

            return false;
        }
    );

    return result;
}

void showMessageScreen(
    const string& title,
    const string& message,
    const string& footer
) {
    runLoop(
        [&]() {
            beginFrame();

            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);

            int width = csbi.srWindow.Right
                      - csbi.srWindow.Left + 1;
            int height = csbi.srWindow.Bottom
                       - csbi.srWindow.Top + 1;

            int left = csbi.srWindow.Left;
            int top = csbi.srWindow.Top;

            int startY = top + (height - 5) / 2;
            if (startY < top) startY = top;

            printCenter(title, startY, 11, width, left);
            printCenter(message, startY + 2, 14, width, left);
            printCenter(footer, startY + 4, 10, width, left);

            setColor(7);
            endFrame();
        },
        [&](const KeyInput& key) -> bool {
            return key.type == KEY_ENTER || key.type == KEY_ESCAPE;
        }
    );
}

int menu(
    const string& title,
    const vector<string>& items,
    const string& footer
) {
    if(items.empty()) return 0;

    int selected = 1;
    int result = 0;

    runLoop(
        [&]() {
            drawMenu(selected, title, items, footer);
        },
        [&](const KeyInput& key) -> bool {
            if(moveSelection(selected, (int)items.size(), key.type)) {
                return false;
            }

            if(key.type == KEY_ENTER) {
                result = selected;
                return true;
            }

            if(key.type == KEY_ESCAPE) {
                result = 0;
                return true;
            }

            return false;
        }
    );

    return result;
}

vector<string> splitList(const string& text, char separator) {
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

bool isValidAccountId(const string& id) {
    if (id.size() < 4 || id.substr(0, 3) != "ACC") {
        return false;
    }

    return all_of(id.begin() + 3, id.end(),
                  [](unsigned char c) { return c >= '0' && c <= '9'; });
}

bool readProfileFile(const fs::path& profileFile, UserProfile& profile) {
    ifstream file(profileFile);

    if (!file.is_open()) {
        return false;
    }

    UserProfile current;
    string line;

    while (getline(file, line)) {
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

    if (current.status != "LOCKED") {
        current.status = "ACTIVE";
    }

    if (current.id.empty()) {
        current.id = profileFile.parent_path().filename().string();
    }

    profile = current;
    return true;
}

bool findProfileById(const string& accountId, UserProfile& profile) {
    if (!isValidAccountId(accountId)) {
        return false;
    }

    return readProfileFile(
        fs::path("Data/Account") / accountId / "profile.txt", profile);
}

// Äá»•i ngÃ y lÆ°u trong file sang dáº¡ng DD-MM-YYYY Ä‘á»ƒ hiá»ƒn thá»‹
// (nháº­n cáº£ YYYY-MM-DD vÃ  DD-MM-YYYY, sai dáº¡ng thÃ¬ tráº£ vá» chuá»—i rá»—ng)
string toDisplayDate(const string& s) {
    if (s.length() != 10) {
        return "";
    }

    // YYYY-MM-DD -> DD-MM-YYYY
    if (s[4] == '-' && s[7] == '-') {
        return s.substr(8, 2) + "-" + s.substr(5, 2) + "-" + s.substr(0, 4);
    }

    // ÄÃ£ lÃ  DD-MM-YYYY
    if (s[2] == '-' && s[5] == '-') {
        return s;
    }

    return "";
}

vector<FriendEntry> readFriendList(const string& accountId) {
    vector<FriendEntry> list;

    if (!isValidAccountId(accountId)) {
        return list;
    }

    ifstream file(fs::path("Data/Account") / accountId / "friend.txt");

    if (!file.is_open()) {
        return list;
    }

    string line;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t first = line.find('|');

        if (first == string::npos) {
            continue;
        }

        size_t second = line.find('|', first + 1);

        FriendEntry entry;
        entry.id = trimString(line.substr(0, first));

        if (second == string::npos) {
            entry.status = trimString(line.substr(first + 1));
        }
        else {
            entry.status = trimString(line.substr(first + 1, second - first - 1));
            entry.date = trimString(line.substr(second + 1));
        }

        if (!entry.id.empty()) {
            list.push_back(entry);
        }
    }

    return list;
}

vector<string> getFriendIds(const string& accountId) {
    vector<string> ids;

    for (const auto& entry : readFriendList(accountId)) {
        if (entry.status != "FRIEND") {
            continue;
        }

        if (find(ids.begin(), ids.end(), entry.id) == ids.end()) {
            ids.push_back(entry.id);
        }
    }

    return ids;
}

int countFriends(const string& accountId) {
    return (int)getFriendIds(accountId).size();
}

int countMutualFriends(const string& idA, const string& idB) {
    vector<string> friendsA = getFriendIds(idA);
    vector<string> friendsB = getFriendIds(idB);

    int count = 0;

    for (const string& id : friendsA) {
        if (find(friendsB.begin(), friendsB.end(), id) != friendsB.end()) {
            count++;
        }
    }

    return count;
}

vector<string> getMutualFriendIds(const string& idA, const string& idB) {
    vector<string> friendsB = getFriendIds(idB);
    vector<string> result;

    for (const string& id : getFriendIds(idA)) {
        if (find(friendsB.begin(), friendsB.end(), id) != friendsB.end()) {
            result.push_back(id);
        }
    }

    return result;
}

// Táº¡o khá»‘i thÃ´ng tin hiá»ƒn thá»‹ cá»§a 1 há»“ sÆ¡: {nhÃ£n, giÃ¡ trá»‹}
// mutualFriends >= 0 thÃ¬ thÃªm dÃ²ng "Sá»‘ Báº¡n Chung" (dÃ¹ng khi xem trang cá»§a ngÆ°á»i khÃ¡c)
// showEmpty = true: cÃ¡c má»¥c tÃ¹y chá»n bá» trá»‘ng váº«n hiá»‡n vá»›i chá»¯ "(chÆ°a cÃ³)" (dÃ¹ng cho trang cá»§a tÃ´i)
vector<pair<string, string>> buildProfileInfo(const UserProfile& profile,
                                              int mutualFriends,
                                              bool showEmpty) {
    vector<pair<string, string>> info;

    info.push_back({"TÃªn", profile.fullName.empty() ? profile.username
                                                    : profile.fullName});

    // Bio
    if (!profile.bio.empty()) {
        info.push_back({"Bio", profile.bio});
    }
    else if (showEmpty) {
        info.push_back({"Bio", "(chÆ°a cÃ³)"});
    }

    // NgÃ y sinh
    string birthday = toDisplayDate(profile.birthday);
    if (!birthday.empty()) {
        info.push_back({"NgÃ y Sinh", birthday});
    }
    else if (showEmpty) {
        info.push_back({"NgÃ y Sinh", "(chÆ°a cÃ³)"});
    }

    // Giá»›i tÃ­nh
    if (!profile.gender.empty()) {
        info.push_back({"Giá»›i TÃ­nh", profile.gender});
    }
    else if (showEmpty) {
        info.push_back({"Giá»›i TÃ­nh", "(chÆ°a chá»n)"});
    }

    // NÆ¡i á»Ÿ, quÃª quÃ¡n, trÆ°á»ng há»c, nÆ¡i lÃ m viá»‡c, sá»Ÿ thÃ­ch (ngÆ°á»i khÃ¡c chÆ°a nháº­p thÃ¬ khÃ´ng hiá»‡n)
    auto addOptional = [&](const string& label, const string& value, const string& emptyText) {
        if (!value.empty()) {
            info.push_back({label, value});
        }
        else if (showEmpty) {
            info.push_back({label, emptyText});
        }
    };

    addOptional("NÆ¡i á»ž Hiá»‡n Táº¡i", profile.residence, "(chÆ°a chá»n)");
    addOptional("QuÃª QuÃ¡n", profile.hometown, "(chÆ°a chá»n)");
    addOptional("TrÆ°á»ng Há»c", profile.school, "(chÆ°a chá»n)");
    addOptional("NÆ¡i LÃ m Viá»‡c", profile.workplace, "(chÆ°a cÃ³)");
    addOptional("Sá»Ÿ ThÃ­ch", joinList(splitList(profile.hobbies), ", "), "(chÆ°a chá»n)");

    info.push_back({"Sá»‘ Báº¡n BÃ¨", to_string(countFriends(profile.id))});

    if (mutualFriends >= 0) {
        info.push_back({"Sá»‘ Báº¡n Chung", to_string(mutualFriends)});
    }

    string joined = toDisplayDate(profile.createdAt);
    info.push_back({"Tham Gia Tá»«", joined.empty() ? "KhÃ´ng rÃµ" : joined});

    return info;
}

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

string cutText(const string& s, int maxLen) {
    if (textLength(s) <= maxLen) return s;
    if (maxLen <= 3) return cutChars(s, maxLen);

    return cutChars(s, maxLen - 3) + "...";
}

// Cáº¯t khoáº£ng tráº¯ng vÃ  xuá»‘ng dÃ²ng á»Ÿ Ä‘áº§u vÃ  cuá»‘i chuá»—i
string trimBlank(const string& s) {
    size_t begin = s.find_first_not_of(" \t\r\n");

    if (begin == string::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

string oneLine(const string& s) {
    string result;

    for (char c : s) {
        result += (c == '\n' || c == '\r') ? ' ' : c;
    }

    return result;
}

unsigned int decodeAt(const string& s, size_t i, int& length) {
    unsigned char c = (unsigned char)s[i];
    unsigned int cp = c;
    int extra = 0;

    if (c >= 0xF0)      { cp = c & 0x07; extra = 3; }
    else if (c >= 0xE0) { cp = c & 0x0F; extra = 2; }
    else if (c >= 0xC0) { cp = c & 0x1F; extra = 1; }

    if (extra > 0 && i + extra >= s.size()) {
        length = 1;
        return c;
    }

    for (int k = 1; k <= extra; k++) {
        cp = (cp << 6) | ((unsigned char)s[i + k] & 0x3F);
    }

    length = extra + 1;
    return cp;
}

string foldText(const string& s) {
    static map<unsigned int, char> table;

    if (table.empty()) {
        const vector<pair<char, string>> groups = {
            {'a', "Ã Ã¡áº¡áº£Ã£Ã¢áº§áº¥áº­áº©áº«Äƒáº±áº¯áº·áº³áºµÃ€Ãáº áº¢ÃƒÃ‚áº¦áº¤áº¬áº¨áºªÄ‚áº°áº®áº¶áº²áº´"},
            {'e', "Ã¨Ã©áº¹áº»áº½Ãªá»áº¿á»‡á»ƒá»…ÃˆÃ‰áº¸áººáº¼ÃŠá»€áº¾á»†á»‚á»„"},
            {'i', "Ã¬Ã­á»‹á»‰Ä©ÃŒÃá»Šá»ˆÄ¨"},
            {'o', "Ã²Ã³á»á»ÃµÃ´á»“á»‘á»™á»•á»—Æ¡á»á»›á»£á»Ÿá»¡Ã’Ã“á»Œá»ŽÃ•Ã”á»’á»á»˜á»”á»–Æ á»œá»šá»¢á»žá» "},
            {'u', "Ã¹Ãºá»¥á»§Å©Æ°á»«á»©á»±á»­á»¯Ã™Ãšá»¤á»¦Å¨Æ¯á»ªá»¨á»°á»¬á»®"},
            {'y', "á»³Ã½á»µá»·á»¹á»²Ãá»´á»¶á»¸"},
            {'d', "Ä‘Ä"}
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

vector<string> wrapParagraph(const string& p, int width) {
    vector<string> lines;
    string line;
    int lineLen = 0;
    size_t i = 0;

    while (i < p.size()) {
        size_t j = p.find(' ', i);
        string word = (j == string::npos) ? p.substr(i) : p.substr(i, j - i);
        i = (j == string::npos) ? p.size() : j + 1;

        if (word.empty()) continue;

        int wordLen = textLength(word);

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

// MÃ£ hÃ³a ná»™i dung nhiá»u dÃ²ng thÃ nh 1 dÃ²ng Ä‘á»ƒ lÆ°u file (\ -> \\, xuá»‘ng dÃ²ng -> \n)
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

// NgÃ y hiá»‡n táº¡i dáº¡ng YYYY-MM-DD
string todayIso() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay);

    return buffer;
}

// Thá»i Ä‘iá»ƒm hiá»‡n táº¡i dáº¡ng "YYYY-MM-DD HH:MM:SS"
string nowTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay,
             (int)st.wHour, (int)st.wMinute, (int)st.wSecond);

    return buffer;
}

// Äá»c chuá»—i "YYYY-MM-DD HH:MM:SS" thÃ nh time_t
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

// Thá»i gian hiá»ƒn thá»‹: "x phÃºt / giá» / ngÃ y / thÃ¡ng trÆ°á»›c", quÃ¡ 1 nÄƒm thÃ¬ hiá»‡n dd/mm/yyyy
string timeAgo(const string& timestamp) {
    time_t then;

    if (!parseTimestamp(timestamp, then)) {
        return "";
    }

    long long diff = (long long)difftime(time(nullptr), then);

    if (diff < 60)    return "Vá»«a xong";
    if (diff < 3600)  return to_string(diff / 60) + " phÃºt trÆ°á»›c";
    if (diff < 86400) return to_string(diff / 3600) + " giá» trÆ°á»›c";

    long long days = diff / 86400;

    if (days < 30)  return to_string(days) + " ngÃ y trÆ°á»›c";
    if (days < 365) return to_string(days / 30) + " thÃ¡ng trÆ°á»›c";

    return toSlashDate(timestamp);
}

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

// Ghi Ä‘Ã¨ toÃ n bá»™ file báº±ng cÃ¡c dÃ²ng
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

// ThÃªm 1 dÃ²ng vÃ o cuá»‘i file (tá»± táº¡o thÆ° má»¥c náº¿u chÆ°a cÃ³)
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

string getDisplayName(const string& accountId) {
    UserProfile profile;

    if (!findProfileById(accountId, profile)) {
        return "NgÆ°á»i dÃ¹ng Ä‘Ã£ xÃ³a";
    }

    return profile.fullName.empty() ? profile.username : profile.fullName;
}

int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 80;
    }

    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void notify(const string& message) {
    printCenter(message);
    sleep_for(2s);
}

TextBlock makeBlock(const string& text, int color, int align,
                    int indent, int widthPercent) {
    TextBlock block;
    block.text = text;
    block.color = color;
    block.align = align;
    block.indent = indent;
    block.widthPercent = widthPercent;
    return block;
}

vector<string> wrapKeepSpaces(const string& text, int width) {
    vector<string> lines;
    size_t start = 0;

    while (true) {
        size_t newline = text.find('\n', start);
        string paragraph = (newline == string::npos) ? text.substr(start)
                                                     : text.substr(start, newline - start);

        string current;
        int currentLen = 0;
        int lastSpace = -1;

        size_t i = 0;

        while (i < paragraph.size()) {
            unsigned char lead = (unsigned char)paragraph[i];
            size_t len = (lead >= 0xF0) ? 4 : (lead >= 0xE0) ? 3 : (lead >= 0xC0) ? 2 : 1;
            string ch = paragraph.substr(i, len);
            i += len;

            current += ch;
            currentLen += textLength(ch);

            if (ch == " ") {
                lastSpace = (int)current.size();
            }

            if (currentLen > width) {
                if (lastSpace > 0 && lastSpace < (int)current.size()) {
                    lines.push_back(current.substr(0, lastSpace));
                    current = current.substr(lastSpace);
                    currentLen = textLength(current);
                    lastSpace = -1;
                }
                else {

                    current.erase(current.size() - ch.size());
                    lines.push_back(current);
                    current = ch;
                    currentLen = textLength(ch);
                    lastSpace = (ch == " ") ? 1 : -1;
                }
            }
        }

        lines.push_back(current);

        if (newline == string::npos) {
            break;
        }

        start = newline + 1;
    }

    return lines;
}

vector<DisplayLine> layoutBlocks(const vector<TextBlock>& blocks, int blockWidth) {
    vector<DisplayLine> lines;

    for (const TextBlock& block : blocks) {
        if (block.raw) {
            DisplayLine line;
            line.text = block.text;
            line.color = block.color;
            line.offset = block.indent;
            lines.push_back(line);
            continue;
        }

        int width = (blockWidth - block.indent) * block.widthPercent / 100;
        if (width < 1) width = 1;

        vector<string> wrapped = block.keepSpaces ? wrapKeepSpaces(block.text, width)
                                                  : wrapText(block.text, width);

        for (const string& text : wrapped) {
            int len = textLength(text);
            int offset = block.indent;

            if (block.align == 1)      offset = (blockWidth - len) / 2;
            else if (block.align == 2) offset = blockWidth - len;

            if (offset < 0) offset = 0;

            DisplayLine line;
            line.text = text;
            line.color = block.color;
            line.offset = offset;
            lines.push_back(line);
        }
    }

    return lines;
}

vector<string> splitFooter(const string& footer, int maxLen) {
    if (maxLen < 10) maxLen = 10;

    vector<string> parts;
    size_t start = 0;

    while (true) {
        size_t pos = footer.find(" | ", start);

        if (pos == string::npos) {
            parts.push_back(footer.substr(start));
            break;
        }

        parts.push_back(footer.substr(start, pos - start));
        start = pos + 3;
    }

    vector<string> lines;
    string line;

    for (string part : parts) {
        if (textLength(part) > maxLen) {
            part = cutText(part, maxLen);
        }

        if (line.empty()) {
            line = part;
        }
        else if (textLength(line) + 3 + textLength(part) <= maxLen) {
            line += " | " + part;
        }
        else {
            lines.push_back(line);
            line = part;
        }
    }

    if (!line.empty() || lines.empty()) {
        lines.push_back(line);
    }

    if (lines.size() > 3) {
        lines.resize(3);
    }

    return lines;
}

void printAt(int x, int y, const string& s, int color) {
    gotoxy(x, y);
    setColor(color);
    writeText(s);
}

int computeReaderBlockWidth(int width) {
    int margin = max(2, width / 8);
    int blockWidth = min(100, width - 2 * margin);
    if (blockWidth < 10) blockWidth = max(1, width - 2);
    return blockWidth;
}

void drawReaderFrame(
    const string& title,
    const vector<TextBlock>& blocks,
    ReaderState& state,
    const vector<BottomLine>& bottom,
    const string& footer
) {
    beginFrame();

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        endFrame();
        return;
    }

    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int left = csbi.srWindow.Left;
    int top = csbi.srWindow.Top;
    int rows = height - 1;

    int blockWidth = computeReaderBlockWidth(width);
    int blockX = left + (width - blockWidth) / 2;

    int maxLen = width - 2;

    vector<DisplayLine> lines = layoutBlocks(blocks, blockWidth);
    int total = (int)lines.size();

    vector<string> footerLines = splitFooter(footer + " | 100%", maxLen);
    int contentHeight = rows - 3 - (int)footerLines.size() - (int)bottom.size();
    if (contentHeight < 3) contentHeight = 3;

    state.viewHeight = contentHeight;
    state.maxOffset = max(0, total - contentHeight);
    if (state.offset > state.maxOffset) state.offset = state.maxOffset;
    if (state.offset < 0) state.offset = 0;

    bool scrollable = total > contentHeight;

    if (scrollable) {
        int percent = (state.maxOffset == 0) ? 100 : state.offset * 100 / state.maxOffset;
        footerLines = splitFooter(footer + " | " + to_string(percent) + "%", maxLen);
    }
    else {
        footerLines = splitFooter(footer, maxLen);
    }

    printCenter(cutText(title, maxLen), top, 11, width, left);
    printCenter(string(blockWidth, '-'), top + 1, 8, width, left);

    state.endVisible = false;

    for (int i = 0; i < contentHeight; i++) {
        int index = state.offset + i;
        if (index >= total) break;

        const DisplayLine& line = lines[index];
        int y = top + 2 + i;
        string shown = cutChars(line.text, blockWidth - line.offset);

        printAt(blockX + line.offset, y, shown, line.color);

        if (index == total - 1) {
            state.endVisible = true;
            state.endX = blockX + line.offset + textLength(shown);
            state.endY = y;
        }
    }

    if (scrollable) {
        int barX = blockX + blockWidth + 1;
        if (barX > left + width - 1) barX = left + width - 1;

        int thumb = max(1, contentHeight * contentHeight / total);
        int thumbPos = (state.maxOffset == 0)
                     ? 0
                     : state.offset * (contentHeight - thumb) / state.maxOffset;

        for (int i = 0; i < contentHeight; i++) {
            bool inThumb = (i >= thumbPos && i < thumbPos + thumb);
            printAt(barX, top + 2 + i, inThumb ? "â–ˆ" : "â”‚", inThumb ? 11 : 8);
        }
    }
    bool cursorSet = false;
    int cursorX = 0;
    int cursorY = 0;

    for (size_t k = 0; k < bottom.size(); k++) {
        const BottomLine& line = bottom[k];
        int y = top + 2 + contentHeight + (int)k;
        int limit = line.leftAlign ? blockWidth : maxLen;

        string text = line.keepTail ? tailChars(line.text, limit)
                                    : cutChars(line.text, limit);

        int x = line.leftAlign ? blockX : left + (width - textLength(text)) / 2;
        if (x < left) x = left;

        printAt(x, y, text, line.color);

        if (line.cursor) {
            cursorSet = true;
            cursorX = x + textLength(text);
            cursorY = y;
        }
    }

    int separatorY = top + 2 + contentHeight + (int)bottom.size();
    printCenter(string(blockWidth, '-'), separatorY, 8, width, left);

    for (size_t k = 0; k < footerLines.size(); k++) {
        printCenter(footerLines[k], separatorY + 1 + (int)k, 10, width, left);
    }

    if (cursorSet) {
        gotoxy(cursorX, cursorY);
        showCursor();
    }
    else if (state.wantEndCursor && state.endVisible) {
        gotoxy(state.endX, state.endY);
        showCursor();
    }
    else {
        hideCursor();
    }

    setColor(7);
    endFrame();
}

// Xá»­ lÃ½ phÃ­m cuá»™n. Tráº£ vá» true náº¿u phÃ­m vá»«a nháº¥n lÃ  phÃ­m cuá»™n
bool applyScrollKey(const KeyInput& key, ReaderState& state) {
    switch (key.type) {
        case KEY_UP:          state.offset -= 1;                      break;
        case KEY_DOWN:        state.offset += 1;                      break;
        case KEY_SCROLL_UP:   state.offset -= 3;                      break;
        case KEY_SCROLL_DOWN: state.offset += 3;                      break;
        case KEY_PAGE_UP:     state.offset -= max(1, state.viewHeight - 1); break;
        case KEY_PAGE_DOWN:   state.offset += max(1, state.viewHeight - 1); break;
        case KEY_HOME:        state.offset = 0;                       break;
        case KEY_END:         state.offset = state.maxOffset;         break;
        default:
            return false;
    }

    if (state.offset > state.maxOffset) state.offset = state.maxOffset;
    if (state.offset < 0) state.offset = 0;

    return true;
}

// MÃ n hÃ¬nh Ä‘á»c: tráº£ vá» 0 khi nháº¥n ESC, ngÆ°á»£c láº¡i tráº£ vá» mÃ£ phÃ­m hÃ nh Ä‘á»™ng
// (chá»¯ HOA náº±m trong actionKeys, hoáº·c '\n' náº¿u actionKeys cÃ³ '\n' vÃ  nháº¥n ENTER)
int runReader(
    const string& title,
    const vector<TextBlock>& blocks,
    ReaderState& state,
    const string& footer,
    const string& actionKeys
) {
    int result = 0;
    vector<BottomLine> noBottom;

    runLoop(
        // Váº½ giao diá»‡n
        [&]() {
            drawReaderFrame(title, blocks, state, noBottom, footer);
        },
        // Xá»­ lÃ½ phÃ­m
        [&](const KeyInput& key) -> bool {
            if (applyScrollKey(key, state)) {
                return false;
            }

            if (key.type == KEY_ESCAPE) {
                result = 0;
                return true;
            }

            if (key.type == KEY_ENTER && actionKeys.find('\n') != string::npos) {
                result = '\n';
                return true;
            }

            if (key.type == KEY_CHARACTER && key.asciiCode > 0 && key.asciiCode < 128) {
                int upper = toupper(key.asciiCode);

                if (actionKeys.find((char)upper) != string::npos) {
                    result = upper;
                    return true;
                }
            }

            return false;
        }
    );

    return result;
}

// Há»i xÃ¡c nháº­n báº±ng menu KhÃ´ng / CÃ³ (máº·c Ä‘á»‹nh chá»n "KhÃ´ng"). Tráº£ vá» true náº¿u chá»n "CÃ³"
bool confirmScreen(const string& question) {
    vector<string> items = {"KhÃ´ng", "CÃ³"};

    int choice = menu(
        cutText(question, getConsoleWidth() - 4),
        items,
        "Sá»­ dá»¥ng â–² / â–¼ | ENTER: Chá»n | ESC: Há»§y"
    );

    return choice == 2;
}

// MÃ n hÃ¬nh nháº­p vÄƒn báº£n. Tráº£ vá» true náº¿u xÃ¡c nháº­n (ENTER), false náº¿u nháº¥n ESC.
// value vá»«a lÃ  giÃ¡ trá»‹ ban Ä‘áº§u vá»«a lÃ  káº¿t quáº£
bool inputText(const InputOptions& options, string& value) {
    string text = value;
    string errorMessage;
    bool accepted = false;

    ReaderState state;
    state.wantEndCursor = true;

    string footer = options.footer;

    if (footer.empty()) {
        footer = options.multiline
               ? "ENTER: XÃ¡c nháº­n | Shift+ENTER: Xuá»‘ng dÃ²ng | Backspace: XÃ³a | ESC: Há»§y"
               : "ENTER: XÃ¡c nháº­n | Backspace: XÃ³a | ESC: Há»§y";
    }

    runLoop(
        // Váº½ giao diá»‡n
        [&]() {
            vector<TextBlock> blocks;

            if (!options.prompt.empty()) {
                blocks.push_back(makeBlock(options.prompt, 8, 1));
                blocks.push_back(makeBlock("", 7));
            }

            // Ã” Ä‘ang soáº¡n tháº£o: giá»¯ nguyÃªn dáº¥u cÃ¡ch Ä‘á»ƒ con trá» luÃ´n Ä‘á»©ng Ä‘Ãºng chá»—
            TextBlock editBlock = makeBlock(options.mask ? string(textLength(text), '*') : text,
                                            15, 0);
            editBlock.keepSpaces = true;
            blocks.push_back(editBlock);

            vector<BottomLine> bottom;

            if (!errorMessage.empty()) {
                BottomLine line;
                line.text = errorMessage;
                line.color = 12;
                bottom.push_back(line);
            }

            // LuÃ´n cuá»™n xuá»‘ng cuá»‘i Ä‘á»ƒ tháº¥y chá»— Ä‘ang gÃµ
            state.offset = INT_MAX / 2;

            drawReaderFrame(options.title, blocks, state, bottom, footer);
        },
        // Xá»­ lÃ½ phÃ­m
        [&](const KeyInput& key) -> bool {
            if (key.type == KEY_ESCAPE) {
                accepted = false;
                return true;
            }

            if (key.type == KEY_ENTER) {
                string result = trimBlank(text);

                if (options.validate) {
                    errorMessage = options.validate(result);

                    if (!errorMessage.empty()) {
                        return false;
                    }
                }

                value = result;
                accepted = true;
                return true;
            }

            if (key.type == KEY_NEWLINE) {
                if (options.multiline && textLength(text) < options.maxLength) {
                    text += '\n';
                    errorMessage.clear();
                }

                return false;
            }

            if (key.type == KEY_BACKSPACE) {
                popLastChar(text);

                // NgÃ y sinh: xÃ³a luÃ´n dáº¥u '-' tá»± thÃªm vÃ o
                if (options.mode == INPUT_DATE && !text.empty() && text.back() == '-') {
                    text.pop_back();
                }

                errorMessage.clear();
                return false;
            }

            if (key.type == KEY_CHARACTER) {
                errorMessage.clear();

                if (textLength(text) >= options.maxLength) {
                    return false;
                }

                if (options.mode == INPUT_ASCII) {
                    if (key.asciiCode >= 33 && key.asciiCode <= 126) {
                        text += key.character;
                    }
                }
                else if (options.mode == INPUT_DATE) {
                    if (key.asciiCode >= '0' && key.asciiCode <= '9') {
                        if (text.length() == 2 || text.length() == 5) {
                            text += '-';
                        }

                        text += key.character;
                    }
                }
                else {
                    // KhÃ´ng báº¯t Ä‘áº§u báº±ng khoáº£ng tráº¯ng
                    bool leadingSpace = text.empty() && key.character == " ";

                    if (!leadingSpace) {
                        text += key.character;
                    }
                }
            }

            return false;
        }
    );

    hideCursor();
    setColor(7);

    return accepted;
}

bool editTextKey(const KeyInput& key, string& text, int maxChars) {
    if (key.type == KEY_BACKSPACE) {
        if (!text.empty()) {
            popLastChar(text);
            return true;
        }

        return false;
    }

    if (key.type == KEY_CHARACTER && key.asciiCode >= 32 && key.asciiCode != 127) {
        bool leadingSpace = text.empty() && key.character == " ";

        if (!leadingSpace && textLength(text) < maxChars) {
            text += key.character;
            return true;
        }
    }

    return false;
}

void buildCardBlocks(
    const vector<Card>& cards,
    int selected,
    int blockWidth,
    vector<TextBlock>& blocks,
    vector<int>& startLine,
    vector<int>& endLine
) {
    int textWidth = max(5, blockWidth - 4);

    startLine.assign(cards.size(), 0);
    endLine.assign(cards.size(), 0);
    auto keepColor = [](int color, int fallback) -> int {
        return (color == 10 || color == 12 || color == 14) ? (0x10 | color) : fallback;
    };

    auto addLine = [&](const string& text, int normalColor, int selectedColor, bool isSelected) {
        TextBlock block;
        block.raw = true;

        if (isSelected) {
            string line = text;
            int len = textLength(line);

            if (len < blockWidth) {
                line += string(blockWidth - len, ' ');
            }

            block.text = cutChars(line, blockWidth);
            block.color = selectedColor;
        }
        else {
            block.text = text;
            block.color = normalColor;
        }

        blocks.push_back(block);
    };

    for (int i = 0; i < (int)cards.size(); i++) {
        const Card& card = cards[i];
        bool isSel = (i == selected);

        startLine[i] = (int)blocks.size();

        addLine((isSel ? " â–¶ " : "   ") + cutText(card.head, textWidth), card.headColor,
                keepColor(card.headColor, 0x1E), isSel);
        addLine("   " + cutText(card.title, textWidth), card.titleColor,
                keepColor(card.titleColor, 0x1F), isSel);

        for (const string& line : card.body) {
            addLine("   " + cutText(line, textWidth), card.bodyColor,
                    keepColor(card.bodyColor, 0x17), isSel);
        }

        endLine[i] = (int)blocks.size() - 1;

        addLine("", 7, 7, false);
    }
}

void ensureLinesVisible(ReaderState& state, int startL, int endL) {
    if (startL < state.offset) {
        state.offset = startL;
    }
    else if (endL >= state.offset + state.viewHeight) {
        state.offset = endL - state.viewHeight + 1;
    }
}

TextBlock makeCenteredNotice(const string& text, int color, int blockWidth) {
    TextBlock block;
    block.raw = true;
    block.text = cutText(text, blockWidth);
    block.color = color;
    block.indent = max(0, (blockWidth - textLength(block.text)) / 2);
    return block;
}

BottomLine makeSearchLine(const string& query) {
    BottomLine line;
    line.text = "TÃ¬m kiáº¿m: " + query;
    line.color = 14;
    line.leftAlign = true;
    line.cursor = true;
    line.keepTail = true;
    return line;
}

void runCardList(
    const CardListConfig& config,
    const function<vector<CardItem>(int textWidth)>& source,
    const function<void(const string& key)>& onSelect
) {
    bool paged = config.pageSize > 0;
    int shown = paged ? config.pageSize : INT_MAX;
    int selected = 0;
    string selectedKey;
    bool searching = false;
    string query;
    ReaderState state;
    state.viewHeight = 20;

    while (true) {
        int textWidth = max(5, computeReaderBlockWidth(getConsoleWidth()) - 4);
        vector<CardItem> all = source(textWidth);

        if (all.empty()) {
            showMessageScreen(config.title, config.emptyMessage, "ENTER / ESC: Quay láº¡i");
            return;
        }

        int total = (int)all.size();

        vector<string> folded;

        for (const CardItem& item : all) {
            folded.push_back(foldText(item.searchText));
        }

        if (!selectedKey.empty() && !searching) {
            for (int i = 0; i < total; i++) {
                if (all[i].key == selectedKey) {
                    shown = max(shown, i + 1);
                }
            }
        }

        shown = max(1, min(shown, total));

        auto visibleList = [&]() -> vector<int> {
            vector<int> list;
            string key = foldText(trimString(query));

            if (searching && !key.empty()) {
                for (int i = 0; i < total; i++) {
                    if (folded[i].find(key) != string::npos) {
                        list.push_back(i);
                    }
                }
            }
            else {
                for (int i = 0; i < shown; i++) {
                    list.push_back(i);
                }
            }

            return list;
        };

        if (!selectedKey.empty()) {
            vector<int> list = visibleList();

            for (int k = 0; k < (int)list.size(); k++) {
                if (all[list[k]].key == selectedKey) {
                    selected = k;
                }
            }
        }

        bool openItem = false;

        auto drawAll = [&]() {
            vector<int> list = visibleList();
            int count = (int)list.size();

            if (selected >= count) selected = count - 1;
            if (selected < 0) selected = 0;

            int blockWidth = computeReaderBlockWidth(getConsoleWidth());

            vector<Card> cards;

            for (int index : list) {
                cards.push_back(all[index].card);
            }

            vector<TextBlock> blocks;
            vector<int> startLine, endLine;
            buildCardBlocks(cards, selected, blockWidth, blocks, startLine, endLine);

            bool hasQuery = searching && !trimString(query).empty();
            bool canExpand = paged && !hasQuery && shown < total;

            string notice;
            int noticeColor = 8;

            if (hasQuery) {
                notice = count > 0 ? "â”€â”€ TÃ¬m tháº¥y " + to_string(count) + " " + config.noun + " â”€â”€"
                                   : "KhÃ´ng tÃ¬m tháº¥y " + config.noun + " nÃ o phÃ¹ há»£p";
                noticeColor = count > 0 ? 8 : 12;
            }
            else if (canExpand) {
                notice = "â”€â”€ CÃ²n " + to_string(total - shown) + " " + config.moreWord
                       + " - nháº¥n M Ä‘á»ƒ xem thÃªm, A Ä‘á»ƒ xem táº¥t cáº£ â”€â”€";
                noticeColor = 14;
            }
            else {
                notice = "â”€â”€ ÄÃ£ hiá»ƒn thá»‹ táº¥t cáº£ " + to_string(total) + " " + config.noun + " â”€â”€";
            }

            blocks.push_back(makeCenteredNotice(notice, noticeColor, blockWidth));
            int lastLine = (int)blocks.size() - 1;

            vector<BottomLine> bottom;
            string footer;

            if (searching) {
                bottom.push_back(makeSearchLine(query));
                footer = "GÃµ Ä‘á»ƒ tÃ¬m | â–² / â–¼ / Cuá»™n chuá»™t: Chá»n | ENTER: " + config.selectHint
                       + " | ESC: ThoÃ¡t tÃ¬m kiáº¿m";
            }
            else {
                footer = "â–² / â–¼ / Cuá»™n chuá»™t: Chá»n | ENTER: " + config.selectHint + " | ";
                if (config.searchable) footer += "S: TÃ¬m kiáº¿m | ";
                if (canExpand)         footer += "M: Xem thÃªm | A: Xem táº¥t cáº£ | ";
                footer += "ESC: Quay láº¡i";
            }

            auto ensureVisible = [&]() {
                if (count == 0) {
                    state.offset = 0;
                    return;
                }

                int e = (selected == count - 1) ? lastLine : endLine[selected];
                ensureLinesVisible(state, startLine[selected], e);
            };

            ensureVisible();
            drawReaderFrame(config.title, blocks, state, bottom, footer);

            int oldOffset = state.offset;
            ensureVisible();

            if (state.offset != oldOffset) {
                drawReaderFrame(config.title, blocks, state, bottom, footer);
            }
        };

        int pageStep = paged ? config.pageSize : 5;

        runLoop(
            drawAll,
            [&](const KeyInput& key) -> bool {
                vector<int> list = visibleList();
                int count = (int)list.size();

                switch (key.type) {
                    case KEY_UP:          selected -= 1;  break;
                    case KEY_DOWN:        selected += 1;  break;
                    case KEY_SCROLL_UP:   selected -= 2;  break;
                    case KEY_SCROLL_DOWN: selected += 2;  break;
                    case KEY_PAGE_UP:     selected -= pageStep; break;
                    case KEY_PAGE_DOWN:   selected += pageStep; break;
                    case KEY_HOME:        selected = 0;   break;
                    case KEY_END:         selected = count - 1; break;

                    case KEY_ESCAPE:
                        if (searching) {
                            string currentKey = (selected >= 0 && selected < count) ? all[list[selected]].key : "";

                            searching = false;
                            query.clear();
                            selected = 0;

                            for (int i = 0; i < total; i++) {
                                if (all[i].key == currentKey) {
                                    shown = max(shown, i + 1);
                                    selected = i;
                                }
                            }

                            return false;
                        }

                        return true;

                    case KEY_ENTER:
                        if (count == 0) {
                            return false;
                        }

                        openItem = true;
                        return true;

                    case KEY_BACKSPACE:
                        if (searching && editTextKey(key, query, 60)) {
                            selected = 0;
                        }

                        return false;

                    case KEY_CHARACTER:
                        if (searching) {
                            if (editTextKey(key, query, 60)) {
                                selected = 0;
                            }
                        }
                        else if (config.searchable && (key.asciiCode == 's' || key.asciiCode == 'S')) {
                            searching = true;
                            query.clear();
                            selected = 0;
                        }

                        else if (paged && (key.asciiCode == 'm' || key.asciiCode == 'M') && shown < total) {
                            int oldShown = shown;
                            shown = min(total, shown + config.pageSize);
                            selected = oldShown;
                        }

                        else if (paged && (key.asciiCode == 'a' || key.asciiCode == 'A') && shown < total) {
                            shown = total;
                        }

                        return false;

                    default:
                        return false;
                }

                if (selected >= count) selected = count - 1;
                if (selected < 0) selected = 0;

                return false;
            }
        );

        if (!openItem) {
            return;
        }

        vector<int> list = visibleList();
        selectedKey = all[list[max(0, min(selected, (int)list.size() - 1))]].key;
        onSelect(selectedKey);
    }
}

fs::path notificationFile(const string& accountId) {
    return fs::path("Data/Account") / accountId / "notifications.txt";
}

// Äá»c thÃ´ng bÃ¡o cá»§a 1 tÃ i khoáº£n (cÅ© nháº¥t á»Ÿ Ä‘áº§u)
vector<Notification> readNotifications(const string& accountId) {
    vector<Notification> list;

    for (const string& line : readLines(notificationFile(accountId))) {
        vector<string> parts;
        size_t start = 0;

        while (parts.size() < 4) {
            size_t pos = line.find('|', start);

            if (pos == string::npos) {
                break;
            }

            parts.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }

        if (parts.size() < 4) {
            continue;
        }

        Notification n;
        n.type = parts[0];
        n.fromId = parts[1];
        n.target = parts[2];
        n.time = parts[3];
        n.read = (line.substr(start) == "1");
        list.push_back(n);
    }

    return list;
}

bool writeNotifications(const string& accountId, vector<Notification> list) {
    error_code ec;

    if (!fs::is_directory(fs::path("Data/Account") / accountId, ec)) {
        return false;
    }

    if (list.size() > NOTI_MAX_KEEP) {
        list.erase(list.begin(), list.end() - NOTI_MAX_KEEP);
    }

    vector<string> lines;

    for (const Notification& n : list) {
        lines.push_back(n.type + "|" + n.fromId + "|" + n.target + "|" + n.time + "|" + (n.read ? "1" : "0"));
    }

    return writeLines(notificationFile(accountId), lines);
}

// Gá»­i thÃ´ng bÃ¡o cho toId. KhÃ´ng bÃ¡o cho chÃ­nh mÃ¬nh, khÃ´ng bÃ¡o trÃ¹ng thÃ´ng bÃ¡o chÆ°a Ä‘á»c giá»‘ng há»‡t
// (thÃ­ch bÃ i thÃ¬ má»—i ngÆ°á»i chá»‰ bÃ¡o 1 láº§n dÃ¹ báº¥m thÃ­ch / bá» thÃ­ch nhiá»u láº§n)
void addNotification(const string& toId, const string& type, const string& fromId,
                     const string& target) {
    if (toId == fromId || toId.empty()) {
        return;
    }

    error_code ec;

    if (!fs::is_directory(fs::path("Data/Account") / toId, ec)) {
        return;
    }

    vector<Notification> list = readNotifications(toId);

    for (const Notification& n : list) {
        if (n.type == type && n.fromId == fromId && n.target == target &&
            (!n.read || type == NOTI_POST_LIKE)) {
            return;
        }
    }

    Notification n;
    n.type = type;
    n.fromId = fromId;
    n.target = target;
    n.time = nowTimestamp();
    list.push_back(n);

    writeNotifications(toId, list);
}

// Gá»¡ thÃ´ng bÃ¡o "lá»i má»i káº¿t báº¡n" cá»§a fromId khá»i há»™p thÃ´ng bÃ¡o cá»§a owner
// (lá»i má»i Ä‘Ã£ Ä‘Æ°á»£c cháº¥p nháº­n, tá»« chá»‘i hoáº·c há»§y thÃ¬ thÃ´ng bÃ¡o khÃ´ng cÃ²n Ã½ nghÄ©a)
void dropFriendRequestNotifications(const string& ownerId, const string& fromId) {
    vector<Notification> list = readNotifications(ownerId);
    size_t before = list.size();

    list.erase(remove_if(list.begin(), list.end(), [&](const Notification& n) {
        return n.type == NOTI_FRIEND_REQUEST && n.fromId == fromId;
    }), list.end());

    if (list.size() != before) {
        writeNotifications(ownerId, list);
    }
}

// Tin nháº¯n má»›i cÃ³ cáº§n bÃ¡o cho ngÆ°á»i kia khÃ´ng: nháº¯n láº§n Ä‘áº§u (lastMessageTime rá»—ng)
// hoáº·c Ä‘Ã£ lÃ¢u (>= NOTI_MESSAGE_GAP_SECONDS) chÆ°a ai nháº¯n trong cuá»™c trÃ² chuyá»‡n
bool shouldNotifyMessage(const string& lastMessageTime) {
    if (lastMessageTime.empty()) {
        return true;
    }

    time_t last;

    if (!parseTimestamp(lastMessageTime, last)) {
        return true;
    }

    return difftime(time(nullptr), last) >= (double)NOTI_MESSAGE_GAP_SECONDS;
}

/*
    QUAN Há»† Báº N BÃˆ (friend.txt: ACCID|STATUS|YYYY-MM-DD)
                                                            */

// Ghi láº¡i toÃ n bá»™ friend.txt cá»§a 1 tÃ i khoáº£n
bool writeFriendList(const string& accountId, const vector<FriendEntry>& list) {
    if (!isValidAccountId(accountId)) {
        return false;
    }

    vector<string> lines;

    for (const FriendEntry& entry : list) {
        lines.push_back(entry.id + "|" + entry.status + "|" + entry.date);
    }

    return writeLines(fs::path("Data/Account") / accountId / "friend.txt", lines);
}

string getRelation(const string& owner, const string& other) {
    for (const FriendEntry& entry : readFriendList(owner)) {
        if (entry.id == other) {
            return entry.status;
        }
    }

    return "";
}

// Äáº·t tráº¡ng thÃ¡i owner -> other (status rá»—ng = xÃ³a quan há»‡)
bool setRelation(const string& owner, const string& other, const string& status) {
    vector<FriendEntry> list = readFriendList(owner);

    list.erase(remove_if(list.begin(), list.end(),
                         [&](const FriendEntry& entry) { return entry.id == other; }),
               list.end());

    if (!status.empty()) {
        FriendEntry entry;
        entry.id = other;
        entry.status = status;
        entry.date = todayIso();
        list.push_back(entry);
    }

    return writeFriendList(owner, list);
}

// Hai tÃ i khoáº£n cÃ³ lÃ  báº¡n bÃ¨ khÃ´ng
bool areFriends(const string& a, const string& b) {
    return getRelation(a, b) == "FRIEND" || getRelation(b, a) == "FRIEND";
}

// CÃ³ ai cháº·n ai giá»¯a hai tÃ i khoáº£n khÃ´ng
bool isBlockedEither(const string& a, const string& b) {
    return getRelation(a, b) == "BLOCK" || getRelation(b, a) == "BLOCK";
}

// Äá»c toÃ n bá»™ bÃ¡o cÃ¡o (theo thá»© tá»± Ä‘Ã£ gá»­i, cÅ© nháº¥t á»Ÿ Ä‘áº§u)
vector<Report> readReports() {
    vector<Report> reports;

    for (const string& line : readLines(REPORT_FILE)) {
        // Cáº¯t theo 4 dáº¥u '|' Ä‘áº§u tiÃªn, pháº§n cÃ²n láº¡i (lÃ½ do) cÃ³ thá»ƒ chá»©a '|'
        vector<size_t> bars;
        size_t pos = 0;

        while (bars.size() < 4) {
            pos = line.find('|', pos);

            if (pos == string::npos) {
                break;
            }

            bars.push_back(pos);
            pos++;
        }

        if (bars.size() < 4) {
            continue;
        }

        Report report;
        report.type = line.substr(0, bars[0]);
        report.reporterId = line.substr(bars[0] + 1, bars[1] - bars[0] - 1);
        report.targetId = line.substr(bars[1] + 1, bars[2] - bars[1] - 1);
        report.time = line.substr(bars[2] + 1, bars[3] - bars[2] - 1);

        // Cá»™t tráº¡ng thÃ¡i (file cÅ© khÃ´ng cÃ³ thÃ¬ pháº§n cÃ²n láº¡i chÃ­nh lÃ  lÃ½ do)
        string rest = line.substr(bars[3] + 1);
        size_t statusEnd = rest.find('|');

        if (statusEnd != string::npos) {
            string status = rest.substr(0, statusEnd);

            if (status == "PENDING" || status == "DONE") {
                report.handled = (status == "DONE");
                rest = rest.substr(statusEnd + 1);
            }
        }

        report.reason = unescapeText(rest);

        if (report.type != REPORT_USER && report.type != REPORT_POST) {
            continue;
        }

        reports.push_back(report);
    }

    return reports;
}

// Ghi Ä‘Ã¨ toÃ n bá»™ bÃ¡o cÃ¡o
bool writeReports(const vector<Report>& reports) {
    error_code ec;
    fs::create_directories(REPORT_FILE.parent_path(), ec);

    vector<string> lines;

    for (const Report& report : reports) {
        lines.push_back(report.type + "|" + report.reporterId + "|" + report.targetId + "|"
                        + report.time + "|" + (report.handled ? "DONE" : "PENDING") + "|"
                        + escapeText(report.reason));
    }

    return writeLines(REPORT_FILE, lines);
}

// NgÆ°á»i nÃ y Ä‘Ã£ bÃ¡o cÃ¡o Ä‘á»‘i tÆ°á»£ng nÃ y chÆ°a
bool hasReported(const string& type, const string& reporterId, const string& targetId) {
    for (const Report& report : readReports()) {
        if (report.type == type && report.reporterId == reporterId && report.targetId == targetId) {
            return true;
        }
    }

    return false;
}

// Gá»­i 1 bÃ¡o cÃ¡o má»›i
bool addReport(const string& type, const string& reporterId, const string& targetId,
               const string& reason) {
    return appendLine(REPORT_FILE,
                      type + "|" + reporterId + "|" + targetId + "|"
                      + nowTimestamp() + "|PENDING|" + escapeText(reason));
}

// XÃ³a má»i bÃ¡o cÃ¡o vá» 1 Ä‘á»‘i tÆ°á»£ng (tÃ i khoáº£n hoáº·c bÃ i viáº¿t Ä‘Ã£ bá»‹ xÃ³a)
void removeReportsAbout(const string& targetId) {
    vector<Report> reports = readReports();
    size_t before = reports.size();

    reports.erase(remove_if(reports.begin(), reports.end(),
                            [&](const Report& report) { return report.targetId == targetId; }),
                  reports.end());

    if (reports.size() != before) {
        writeReports(reports);
    }
}

/*
    NGÆ¯á»œI DÃ™NG Gá»¬I BÃO CÃO
                            */

// Nháº­p lÃ½ do bÃ¡o cÃ¡o. Tráº£ vá» false náº¿u nháº¥n ESC
bool inputReportReason(const string& title, const string& prompt, string& reason) {
    InputOptions options;
    options.title = title;
    options.prompt = prompt;
    options.maxLength = 200;
    options.validate = [](const string& s) -> string {
        return s.empty() ? "LÃ½ do bÃ¡o cÃ¡o khÃ´ng Ä‘Æ°á»£c Ä‘á»ƒ trá»‘ng!" : "";
    };

    return inputText(options, reason);
}

// BÃ¡o cÃ¡o 1 ngÆ°á»i dÃ¹ng vá»›i lÃ½ do do ngÆ°á»i bÃ¡o cÃ¡o nháº­p
void reportUserFlow(const string& myId, const string& targetId) {
    if (myId == targetId) {
        notify("Báº¡n khÃ´ng thá»ƒ tá»± bÃ¡o cÃ¡o chÃ­nh mÃ¬nh!");
        return;
    }

    if (hasReported(REPORT_USER, myId, targetId)) {
        notify("Báº¡n Ä‘Ã£ bÃ¡o cÃ¡o ngÆ°á»i nÃ y rá»“i, quáº£n trá»‹ viÃªn sáº½ xem xÃ©t!");
        return;
    }

    string reason;

    if (!inputReportReason("=============== BÃO CÃO NGÆ¯á»œI DÃ™NG ===============",
                           "Nháº­p lÃ½ do bÃ¡o cÃ¡o " + getDisplayName(targetId) + " (tá»‘i Ä‘a 200 kÃ½ tá»±)",
                           reason)) {
        return;
    }

    if (addReport(REPORT_USER, myId, targetId, reason)) {
        notify("ÄÃ£ gá»­i bÃ¡o cÃ¡o Ä‘áº¿n quáº£n trá»‹ viÃªn!");
    }
    else {
        notify("KhÃ´ng thá»ƒ gá»­i bÃ¡o cÃ¡o, vui lÃ²ng thá»­ láº¡i!");
    }
}

// BÃ¡o cÃ¡o 1 bÃ i viáº¿t vá»›i lÃ½ do do ngÆ°á»i bÃ¡o cÃ¡o nháº­p
void reportPostFlow(const string& myId, const string& postId) {
    if (hasReported(REPORT_POST, myId, postId)) {
        notify("Báº¡n Ä‘Ã£ bÃ¡o cÃ¡o bÃ i viáº¿t nÃ y rá»“i, quáº£n trá»‹ viÃªn sáº½ xem xÃ©t!");
        return;
    }

    string reason;

    if (!inputReportReason("=============== BÃO CÃO BÃ€I VIáº¾T ===============",
                           "Nháº­p lÃ½ do bÃ¡o cÃ¡o bÃ i viáº¿t nÃ y (tá»‘i Ä‘a 200 kÃ½ tá»±)",
                           reason)) {
        return;
    }

    if (addReport(REPORT_POST, myId, postId, reason)) {
        notify("ÄÃ£ gá»­i bÃ¡o cÃ¡o Ä‘áº¿n quáº£n trá»‹ viÃªn!");
    }
    else {
        notify("KhÃ´ng thá»ƒ gá»­i bÃ¡o cÃ¡o, vui lÃ²ng thá»­ láº¡i!");
    }
}

fs::path postDir(const string& postId) {
    return fs::path("Data/Post") / postId;
}

string visibilityLabel(const string& visibility) {
    if (visibility == "PUBLIC") return "CÃ´ng Khai";
    if (visibility == "FRIEND") return "Báº¡n BÃ¨";
    return "Chá»‰ MÃ¬nh TÃ´i";
}

// Äá»c 1 bÃ i Ä‘Äƒng
bool readPost(const string& postId, Post& post) {
    error_code ec;
    fs::path file = postDir(postId) / "post.txt";

    if (!fs::is_regular_file(file, ec)) {
        return false;
    }

    Post current;
    current.id = postId;

    for (const string& line : readLines(file)) {
        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "author")            current.authorId = value;
        else if (key == "visibility")   current.visibility = value;
        else if (key == "created_at")   current.createdAt = value;
        else if (key == "edited_at")    current.editedAt = value;
        else if (key == "status")       current.hidden = (value == "HIDDEN");
        else if (key == "title")        current.title = unescapeText(value);
        else if (key == "content")      current.content = unescapeText(value);
    }

    if (current.authorId.empty()) {
        return false;
    }

    if (current.visibility.empty()) {
        current.visibility = "PRIVATE";
    }

    post = current;
    return true;
}

// Ghi 1 bÃ i Ä‘Äƒng
bool writePost(const Post& post) {
    error_code ec;
    fs::create_directories(postDir(post.id), ec);

    vector<string> lines = {
        "id=" + post.id,
        "author=" + post.authorId,
        "visibility=" + post.visibility,
        "created_at=" + post.createdAt,
        "edited_at=" + post.editedAt,
        "status=" + string(post.hidden ? "HIDDEN" : "VISIBLE"),
        "title=" + escapeText(post.title),
        "content=" + escapeText(post.content)
    };

    return writeLines(postDir(post.id) / "post.txt", lines);
}

// ID cá»§a má»i bÃ i Ä‘Äƒng
vector<string> listPostIds() {
    vector<string> ids;
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Post", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (name.size() > 4 && name.compare(0, 4, "POST") == 0) {
            ids.push_back(name);
        }
    }

    sort(ids.begin(), ids.end());
    return ids;
}

// NgÆ°á»i xem cÃ³ Ä‘Æ°á»£c xem bÃ i Ä‘Äƒng nÃ y khÃ´ng
bool canViewPost(const string& viewerId, const Post& post) {
    if (post.authorId == viewerId) {
        return true;
    }

    // BÃ i bá»‹ quáº£n trá»‹ viÃªn áº©n: ngÆ°á»i khÃ¡c khÃ´ng xem Ä‘Æ°á»£c
    if (post.hidden) {
        return false;
    }

    if (isBlockedEither(viewerId, post.authorId)) {
        return false;
    }

    if (post.visibility == "PUBLIC") {
        return true;
    }

    if (post.visibility == "FRIEND") {
        return areFriends(viewerId, post.authorId);
    }

    return false;
}

// Danh sÃ¡ch ID Ä‘Ã£ thÃ­ch bÃ i Ä‘Äƒng
vector<string> readLikes(const string& postId) {
    vector<string> ids;

    for (const string& line : readLines(postDir(postId) / "likes.txt")) {
        string id = trimString(line);

        if (!id.empty() && find(ids.begin(), ids.end(), id) == ids.end()) {
            ids.push_back(id);
        }
    }

    return ids;
}

// ThÃ­ch / bá» thÃ­ch. Tráº£ vá» true náº¿u sau khi báº¥m lÃ  Ä‘ang thÃ­ch
bool toggleLike(const string& postId, const string& accountId) {
    vector<string> likes = readLikes(postId);
    auto it = find(likes.begin(), likes.end(), accountId);
    bool liked;

    if (it != likes.end()) {
        likes.erase(it);
        liked = false;
    }
    else {
        likes.push_back(accountId);
        liked = true;
    }

    writeLines(postDir(postId) / "likes.txt", likes);

    // CÃ³ ngÆ°á»i thÃ­ch bÃ i thÃ¬ bÃ¡o cho tÃ¡c giáº£
    if (liked) {
        Post post;

        if (readPost(postId, post)) {
            addNotification(post.authorId, NOTI_POST_LIKE, accountId, postId);
        }
    }

    return liked;
}

// Äá»c cÃ¡c bÃ¬nh luáº­n (má»—i dÃ²ng: ACCID|THá»œI GIAN|Ná»˜I DUNG)
vector<PostComment> readComments(const string& postId) {
    vector<PostComment> comments;

    for (const string& line : readLines(postDir(postId) / "comments.txt")) {
        size_t first = line.find('|');
        if (first == string::npos) continue;

        size_t second = line.find('|', first + 1);
        if (second == string::npos) continue;

        PostComment comment;
        comment.authorId = line.substr(0, first);
        comment.time = line.substr(first + 1, second - first - 1);
        comment.text = unescapeText(line.substr(second + 1));
        comments.push_back(comment);
    }

    return comments;
}

bool addComment(const string& postId, const string& accountId, const string& text) {
    bool ok = appendLine(postDir(postId) / "comments.txt",
                         accountId + "|" + nowTimestamp() + "|" + escapeText(text));

    // CÃ³ ngÆ°á»i bÃ¬nh luáº­n bÃ i thÃ¬ bÃ¡o cho tÃ¡c giáº£
    if (ok) {
        Post post;

        if (readPost(postId, post)) {
            addNotification(post.authorId, NOTI_POST_COMMENT, accountId, postId);
        }
    }

    return ok;
}

// CÃ¡c bÃ i Ä‘Äƒng mÃ  viewerId Ä‘Æ°á»£c xem, má»›i nháº¥t á»Ÿ trÃªn.
// authorFilter khÃ´ng rá»—ng: chá»‰ láº¥y bÃ i cá»§a ngÆ°á»i Ä‘Ã³
vector<Post> loadPosts(const string& viewerId, const string& authorFilter) {
    vector<Post> posts;

    for (const string& id : listPostIds()) {
        Post post;

        if (!readPost(id, post)) {
            continue;
        }

        if (!authorFilter.empty() && post.authorId != authorFilter) {
            continue;
        }

        if (!canViewPost(viewerId, post)) {
            continue;
        }

        posts.push_back(post);
    }

    sort(posts.begin(), posts.end(), [](const Post& a, const Post& b) {
        if (a.createdAt != b.createdAt) return a.createdAt > b.createdAt;
        return a.id > b.id;
    });

    return posts;
}

// Ná»™i dung xem trÆ°á»›c (dÃ²ng Ä‘áº§u tiÃªn) cá»§a bÃ i Ä‘Äƒng
string previewText(const string& content, int maxLen) {
    string firstLine = content.substr(0, content.find('\n'));
    return cutText(trimBlank(oneLine(firstLine)), maxLen);
}

// TiÃªu Ä‘á» hiá»ƒn thá»‹ trong danh sÃ¡ch (bÃ i cÅ© chÆ°a cÃ³ tiÃªu Ä‘á» thÃ¬ láº¥y dÃ²ng Ä‘áº§u cá»§a ná»™i dung)
string displayTitle(const Post& post, int maxLen) {
    if (!post.title.empty()) {
        return cutText(oneLine(post.title), maxLen);
    }

    return previewText(post.content, maxLen);
}

// Chá»n quyá»n riÃªng tÆ° báº±ng menu. chosen = false náº¿u nháº¥n ESC
string chooseVisibility(const string& title, const string& footer, bool& chosen) {
    vector<string> items = {
        "CÃ´ng Khai (má»i ngÆ°á»i Ä‘á»u xem Ä‘Æ°á»£c)",
        "Báº¡n BÃ¨ (chá»‰ báº¡n bÃ¨ xem Ä‘Æ°á»£c)",
        "Chá»‰ MÃ¬nh TÃ´i (riÃªng tÆ°)"
    };

    int choice = menu(title, items, footer);
    chosen = (choice != 0);

    if (choice == 1) return "PUBLIC";
    if (choice == 2) return "FRIEND";
    return "PRIVATE";
}

// XÃ³a háº³n 1 bÃ i Ä‘Äƒng
void deletePost(const string& postId) {
    error_code ec;
    fs::remove_all(postDir(postId), ec);
    removeReportsAbout(postId);     // BÃ i Ä‘Ã£ xÃ³a thÃ¬ cÃ¡c bÃ¡o cÃ¡o vá» bÃ i Ä‘Ã³ cÅ©ng háº¿t Ã½ nghÄ©a
}

// Äá»c 1 bÃ i Ä‘Äƒng: tiÃªu Ä‘á» cá»‘ Ä‘á»‹nh, ná»™i dung (kÃ¨m bÃ¬nh luáº­n) cuá»™n Ä‘Æ°á»£c, menu hÆ°á»›ng dáº«n cá»‘ Ä‘á»‹nh
void viewPost(const string& myId, const string& postId) {
    ReaderState state;

    while (true) {
        Post post;

        if (!readPost(postId, post) || !canViewPost(myId, post)) {
            notify("BÃ i Ä‘Äƒng khÃ´ng cÃ²n tá»“n táº¡i hoáº·c báº¡n khÃ´ng cÃ³ quyá»n xem!");
            return;
        }

        bool mine = (post.authorId == myId);
        string authorName = getDisplayName(post.authorId);

        vector<string> likes = readLikes(postId);
        bool liked = find(likes.begin(), likes.end(), myId) != likes.end();
        vector<PostComment> comments = readComments(postId);

        // Ná»™i dung cuá»™n Ä‘Æ°á»£c
        vector<TextBlock> blocks;

        string meta = timeAgo(post.createdAt);
        if (!post.editedAt.empty()) meta += " (Ä‘Ã£ chá»‰nh sá»­a)";
        meta += " Â· " + visibilityLabel(post.visibility);
        if (post.hidden) meta += " Â· ÄÃ£ bá»‹ quáº£n trá»‹ viÃªn áº©n";

        blocks.push_back(makeBlock(authorName, 14));
        blocks.push_back(makeBlock(meta, 8));
        blocks.push_back(makeBlock("", 7));

        // TiÃªu Ä‘á» bÃ i Ä‘Äƒng (bÃ i cÅ© chÆ°a cÃ³ tiÃªu Ä‘á» thÃ¬ bá» qua)
        if (!post.title.empty()) {
            blocks.push_back(makeBlock(post.title, 11));
            blocks.push_back(makeBlock("", 7));
        }

        blocks.push_back(makeBlock(post.content, 15));
        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock(
            "LÆ°á»£t thÃ­ch: " + to_string(likes.size()) + (liked ? " (báº¡n Ä‘Ã£ thÃ­ch)" : "")
            + "   |   BÃ¬nh luáº­n: " + to_string(comments.size()), 12));
        blocks.push_back(makeBlock("", 7));
        blocks.push_back(makeBlock("---------- BÃ¬nh luáº­n ----------", 8, 1));
        blocks.push_back(makeBlock("", 7));

        if (comments.empty()) {
            blocks.push_back(makeBlock("ChÆ°a cÃ³ bÃ¬nh luáº­n nÃ o. HÃ£y lÃ  ngÆ°á»i Ä‘áº§u tiÃªn!", 8, 1));
        }

        for (const PostComment& comment : comments) {
            blocks.push_back(makeBlock(
                getDisplayName(comment.authorId) + " Â· " + timeAgo(comment.time), 3));
            blocks.push_back(makeBlock(comment.text, 7, 0, 2));
            blocks.push_back(makeBlock("", 7));
        }

        string title = "=============== BÃ€I ÄÄ‚NG Cá»¦A " + authorName + " ===============";

        string footer = "Cuá»™n chuá»™t / â–² / â–¼: Äá»c | L: ThÃ­ch | C: BÃ¬nh luáº­n";
        string actions = "LC";

        if (mine) {
            footer += " | E: Sá»­a ná»™i dung | T: Sá»­a tiÃªu Ä‘á» | V: Quyá»n riÃªng tÆ° | D: XÃ³a";
            actions += "ETVD";
        }
        else {
            footer += " | P: Trang cÃ¡ nhÃ¢n | R: BÃ¡o cÃ¡o bÃ i viáº¿t";
            actions += "PR";
        }

        footer += " | ESC: Quay láº¡i";

        int action = runReader(title, blocks, state, footer, actions);

        if (action == 0) {
            return;
        }

        if (action == 'L') {
            toggleLike(postId, myId);
        }
        else if (action == 'C') {
            InputOptions options;
            options.title = "=============== BÃŒNH LUáº¬N ===============";
            options.prompt = "Viáº¿t bÃ¬nh luáº­n cá»§a báº¡n (tá»‘i Ä‘a 300 kÃ½ tá»±)";
            options.maxLength = 300;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "BÃ¬nh luáº­n khÃ´ng Ä‘Æ°á»£c Ä‘á»ƒ trá»‘ng!" : "";
            };

            string text;

            if (inputText(options, text)) {
                addComment(postId, myId, text);
                state.offset = INT_MAX / 2;     // Cuá»™n xuá»‘ng Ä‘á»ƒ tháº¥y bÃ¬nh luáº­n má»›i
            }
        }
        else if (action == 'P') {
            viewUserProfile(myId, post.authorId);
        }
        else if (action == 'R') {
            reportPostFlow(myId, postId);
        }
        else if (action == 'T') {
            InputOptions options;
            options.title = "=============== Sá»¬A TIÃŠU Äá»€ BÃ€I ÄÄ‚NG ===============";
            options.prompt = "Chá»‰nh sá»­a tiÃªu Ä‘á» bÃ i Ä‘Äƒng (tá»‘i Ä‘a 100 kÃ½ tá»±)";
            options.maxLength = 100;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "TiÃªu Ä‘á» bÃ i Ä‘Äƒng khÃ´ng Ä‘Æ°á»£c Ä‘á»ƒ trá»‘ng!" : "";
            };

            string newTitle = post.title;

            if (inputText(options, newTitle)) {
                post.title = newTitle;
                post.editedAt = nowTimestamp();
                writePost(post);
            }
        }
        else if (action == 'E') {
            InputOptions options;
            options.title = "=============== Sá»¬A BÃ€I ÄÄ‚NG ===============";
            options.prompt = "Chá»‰nh sá»­a ná»™i dung bÃ i Ä‘Äƒng (tá»‘i Ä‘a 2000 kÃ½ tá»±)";
            options.maxLength = 2000;
            options.multiline = true;
            options.validate = [](const string& s) -> string {
                return s.empty() ? "Ná»™i dung bÃ i Ä‘Äƒng khÃ´ng Ä‘Æ°á»£c Ä‘á»ƒ trá»‘ng!" : "";
            };

            string content = post.content;

            if (inputText(options, content)) {
                post.content = content;
                post.editedAt = nowTimestamp();
                writePost(post);
            }
        }
        else if (action == 'V') {
            bool chosen = false;
            string visibility = chooseVisibility(
                "=============== Äá»”I QUYá»€N RIÃŠNG TÆ¯ ===============",
                "Sá»­ dá»¥ng â–² / â–¼ | ENTER: Chá»n | ESC: Há»§y",
                chosen
            );

            if (chosen) {
                post.visibility = visibility;
                writePost(post);
            }
        }
        else if (action == 'D') {
            if (confirmScreen("Báº¡n cÃ³ cháº¯c muá»‘n xÃ³a bÃ i Ä‘Äƒng nÃ y khÃ´ng?")) {
                deletePost(postId);
                notify("ÄÃ£ xÃ³a bÃ i Ä‘Äƒng!");
                return;
            }
        }
    }
}

// Tháº» hiá»ƒn thá»‹ cá»§a 1 bÃ i Ä‘Äƒng: ngÆ°á»i Ä‘Äƒng - thá»i gian / tiÃªu Ä‘á» / ná»™i dung rÃºt gá»n
Card makePostCard(const Post& post, int textWidth) {
    Card card;

    card.head = getDisplayName(post.authorId) + " Â· " + timeAgo(post.createdAt)
              + " Â· " + visibilityLabel(post.visibility);
    if (post.hidden) card.head += " Â· ÄÃ£ bá»‹ áº©n";

    card.title = displayTitle(post, textWidth);

    // Ná»™i dung: tá»‘i Ä‘a POST_CONTENT_LINES dÃ²ng, dÃ i hÆ¡n thÃ¬ thÃªm "..."
    string body = trimBlank(oneLine(post.content));
    if (body.empty()) body = "(khÃ´ng cÃ³ ná»™i dung)";

    vector<string> bodyLines = wrapText(body, textWidth);
    bool tooLong = (int)bodyLines.size() > POST_CONTENT_LINES;

    if (tooLong) {
        bodyLines.resize(POST_CONTENT_LINES);
        bodyLines.back() = cutChars(bodyLines.back(), textWidth - 3) + "...";
    }

    card.body = bodyLines;
    return card;
}

// Danh sÃ¡ch bÃ i Ä‘Äƒng. Chá»‰ hiá»‡n vÃ i bÃ i má»›i nháº¥t: M xem thÃªm, A xem táº¥t cáº£, S tÃ¬m kiáº¿m (gÃµ tá»›i Ä‘Ã¢u lá»c tá»›i Ä‘Ã³).
// Má»—i bÃ i gá»“m 3 pháº§n: ngÆ°á»i Ä‘Äƒng - thá»i gian / tiÃªu Ä‘á» / ná»™i dung rÃºt gá»n, cÃ¡c bÃ i cÃ¡ch nhau 1 dÃ²ng trá»‘ng.
// BÃ i Ä‘ang chá»n Ä‘Æ°á»£c Ä‘á»•i mÃ u ná»n. â–² / â–¼, con lÄƒn chuá»™t, PgUp / PgDn, Home / End Ä‘á»ƒ di chuyá»ƒn
void browsePosts(
    const string& myId,
    const string& title,
    const function<vector<Post>()>& loader,
    bool showAuthor,
    const string& emptyMessage
) {
    (void)showAuthor;   // LuÃ´n hiá»‡n tÃªn ngÆ°á»i Ä‘Äƒng á»Ÿ dÃ²ng Ä‘áº§u cá»§a má»—i bÃ i

    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = "Äá»c bÃ i";
    config.noun = "bÃ i Ä‘Äƒng";
    config.moreWord = "bÃ i Ä‘Äƒng cÅ© hÆ¡n";
    config.pageSize = POST_PAGE_SIZE;
    config.searchable = true;

    runCardList(
        config,
        [&](int textWidth) {
            vector<CardItem> items;

            for (const Post& post : loader()) {
                CardItem item;
                item.key = post.id;
                item.card = makePostCard(post, textWidth);
                // TÃ¬m theo tÃªn ngÆ°á»i Ä‘Äƒng, tiÃªu Ä‘á», ná»™i dung
                item.searchText = getDisplayName(post.authorId) + " " + post.title + " " + post.content;
                items.push_back(item);
            }

            return items;
        },
        [&](const string& postId) {
            viewPost(myId, postId);
        }
    );
}

// Xem cÃ¡c bÃ i Ä‘Äƒng cá»§a 1 ngÆ°á»i mÃ  mÃ¬nh Ä‘Æ°á»£c phÃ©p xem
void viewUserPosts(const string& myId, const string& otherId) {
    string title = "=============== BÃ€I ÄÄ‚NG Cá»¦A " + getDisplayName(otherId) + " ===============";

    browsePosts(
        myId,
        title,
        [&]() { return loadPosts(myId, otherId); },
        false,
        "NgÆ°á»i nÃ y chÆ°a cÃ³ bÃ i Ä‘Äƒng nÃ o mÃ  báº¡n cÃ³ thá»ƒ xem."
    );
}

vector<ChatMessage> readMessages(const fs::path& file) {
    vector<ChatMessage> messages;

    for (const string& line : readLines(file)) {
        size_t first = line.find('|');
        if (first == string::npos) continue;

        size_t second = line.find('|', first + 1);
        if (second == string::npos) continue;

        ChatMessage message;
        message.sender = line.substr(0, first);
        message.time = line.substr(first + 1, second - first - 1);
        message.text = unescapeText(line.substr(second + 1));
        messages.push_back(message);
    }

    return messages;
}

// Ghi thÃªm 1 tin nháº¯n vÃ o cuá»‘i file há»™i thoáº¡i
bool appendMessage(const fs::path& file, const string& sender, const string& text,
                   string* timeOut) {
    string time = nowTimestamp();

    if (timeOut != nullptr) {
        *timeOut = time;
    }

    return appendLine(file, sender + "|" + time + "|" + escapeText(text));
}

fs::path privateChatFile(const string& a, const string& b) {
    string low = min(a, b);
    string high = max(a, b);

    return fs::path("Data/Conversations/.Private") / (low + "_" + high + ".txt");
}

string messageKey(const ChatMessage& m) { return m.sender + "\t" + m.time; }

fs::path hiddenMessagesFile(const string& myId) {
    return fs::path("Data/Conversations/.Hidden") / (myId + ".txt");
}

set<string> readHiddenMessageKeys(const string& myId, const fs::path& conversationFile) {
    set<string> result;
    for (const string& line : readLines(hiddenMessagesFile(myId))) {
        size_t p1 = line.find('|'), p2 = line.find('|', p1 == string::npos ? 0 : p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        string conversationKey = conversationFile.lexically_relative("Data/Conversations").generic_string();
        if (line.substr(0, p1) == conversationKey)
            result.insert(line.substr(p1 + 1, p2 - p1 - 1) + "\t" + line.substr(p2 + 1));
    }
    return result;
}

bool hideMessageLocally(const string& myId, const fs::path& conversationFile, const ChatMessage& m) {
    error_code ec;
    fs::create_directories(hiddenMessagesFile(myId).parent_path(), ec);
    string conversationKey = conversationFile.lexically_relative("Data/Conversations").generic_string();
    string line = conversationKey + "|" + m.sender + "|" + m.time;
    vector<string> lines = readLines(hiddenMessagesFile(myId));
    if (find(lines.begin(), lines.end(), line) == lines.end()) lines.push_back(line);
    return writeLines(hiddenMessagesFile(myId), lines);
}

bool rewriteMessages(const fs::path& file, const vector<ChatMessage>& messages) {
    vector<string> lines;
    for (const ChatMessage& m : messages)
        lines.push_back(m.sender + "|" + m.time + "|" + escapeText(m.text));
    return writeLines(file, lines);
}

fs::path nicknamesFile(const string& myId) {
    return fs::path("Data/Account") / myId / "nicknames.txt";
}

string getNickname(const string& myId, const string& otherId) {
    for (const string& line : readLines(nicknamesFile(myId))) {
        size_t pos = line.find('=');
        if (pos != string::npos && line.substr(0, pos) == otherId)
            return unescapeText(line.substr(pos + 1));
    }
    return "";
}

bool setNickname(const string& myId, const string& otherId, const string& nickname) {
    vector<string> lines = readLines(nicknamesFile(myId));
    lines.erase(remove_if(lines.begin(), lines.end(), [&](const string& line) {
        size_t pos = line.find('=');
        return pos != string::npos && line.substr(0, pos) == otherId;
    }), lines.end());
    if (!trimString(nickname).empty()) lines.push_back(otherId + "=" + escapeText(trimString(nickname)));
    return writeLines(nicknamesFile(myId), lines);
}

string chatDisplayName(const string& myId, const string& otherId) {
    string nickname = getNickname(myId, otherId);
    return nickname.empty() ? getDisplayName(otherId) : nickname;
}

// MÃ n hÃ¬nh trÃ² chuyá»‡n: tiÃªu Ä‘á» cá»‘ Ä‘á»‹nh - lá»‹ch sá»­ tin nháº¯n (cuá»™n báº±ng chuá»™t) - Ã´ nháº­p - menu hÆ°á»›ng dáº«n.
// blockReason(): tráº£ vá» rá»—ng náº¿u Ä‘Æ°á»£c phÃ©p gá»­i, ngÆ°á»£c láº¡i lÃ  lÃ½ do khÃ´ng gá»­i Ä‘Æ°á»£c
void runChat(
    const string& myId,
    const string& title,
    const fs::path& file,
    const function<string()>& blockReason,
    const string& notifyUserId     // Chat riÃªng: ID ngÆ°á»i kia Ä‘á»ƒ gá»­i thÃ´ng bÃ¡o tin nháº¯n (nhÃ³m thÃ¬ Ä‘á»ƒ rá»—ng)
) {
    vector<ChatMessage> messages = readMessages(file);
    string displayTitle = title;
    set<string> hiddenKeys = readHiddenMessageKeys(myId, file);
    map<string, string> names;
    int selectedMessage = 0;
    int selectionMode = 0; // 0: bÃ¬nh thÆ°á»ng, 1: xÃ³a phÃ­a mÃ¬nh, 2: thu há»“i
    set<string> markedMessages;

    auto nameOf = [&](const string& id) -> string {
        auto it = names.find(id);

        if (it != names.end()) {
            return it->second;
        }

        string name = chatDisplayName(myId, id);
        names[id] = name;
        return name;
    };

    string input;
    string errorMessage;

    ReaderState state;
    state.offset = INT_MAX / 2;     // Má»Ÿ lÃªn lÃ  tháº¥y tin nháº¯n má»›i nháº¥t

    // Dáº¥u hiá»‡u thay Ä‘á»•i cá»§a file há»™i thoáº¡i (kÃ­ch thÆ°á»›c + thá»i Ä‘iá»ƒm ghi) Ä‘á»ƒ biáº¿t cÃ³ tin nháº¯n má»›i
    auto fileStamp = [&]() -> pair<uintmax_t, long long> {
        error_code ec;
        uintmax_t size = fs::file_size(file, ec);

        if (ec) {
            size = 0;
        }

        auto time = fs::last_write_time(file, ec);
        return {size, ec ? 0LL : (long long)time.time_since_epoch().count()};
    };

    pair<uintmax_t, long long> lastStamp = fileStamp();
    string lastReason = blockReason();

    runLoop(
        // Váº½ giao diá»‡n
        [&]() {
            vector<TextBlock> blocks;

            if (messages.empty()) {
                blocks.push_back(makeBlock("ChÆ°a cÃ³ tin nháº¯n nÃ o. HÃ£y báº¯t Ä‘áº§u cuá»™c trÃ² chuyá»‡n!", 8, 1));
            }

            string lastDate;

            int visibleIndex = 0;
            for (const ChatMessage& message : messages) {
                if (hiddenKeys.count(messageKey(message))) continue;
                string date = (message.time.size() >= 10) ? message.time.substr(0, 10) : "";

                // DÃ²ng ngÄƒn cÃ¡ch khi sang ngÃ y má»›i
                if (date != lastDate) {
                    lastDate = date;

                    if (!date.empty()) {
                        blocks.push_back(makeBlock(
                            "---------- " + toSlashDate(date) + " ----------", 8, 1));
                    }
                }

                // ThÃ´ng bÃ¡o cá»§a há»‡ thá»‘ng (nhÃ³m)
                if (message.sender == "SYSTEM") {
                    blocks.push_back(makeBlock(message.text, 8, 1));
                    continue;
                }

                bool withdrawn = message.text == WITHDRAWN_MESSAGE_TEXT;
                string shownText = withdrawn ? "Báº¡n Ä‘Ã£ thu há»“i tin nháº¯n" : message.text;
                string selector;
                if (selectionMode != 0 && markedMessages.count(messageKey(message))) selector = "[x] ";
                else if (selectionMode != 0 && visibleIndex == selectedMessage) selector = "[>] ";
                else selector = "    ";
                string hhmm = (message.time.size() >= 16) ? message.time.substr(11, 5) : "";

                if (message.sender == myId) {
                    blocks.push_back(makeBlock(selector + hhmm, 8, 2));
                    blocks.push_back(makeBlock(shownText, withdrawn ? 8 : 10, 2, 0, 70));
                }
                else {
                    // Hiá»‡n cÃ¹ng dáº¥u chá»n á»Ÿ tin nháº¯n Ä‘á»‘i phÆ°Æ¡ng Ä‘á»ƒ dá»… phÃ¢n biá»‡t má»¥c Ä‘ang thao tÃ¡c.
                    blocks.push_back(makeBlock(selector + nameOf(message.sender) + " Â· " + hhmm, 3, 0));
                    blocks.push_back(makeBlock(shownText, withdrawn ? 8 : 15, 0, 0, 70));
                }
                ++visibleIndex;
                blocks.push_back(makeBlock("", 7));
            }

            vector<BottomLine> bottom;

            if (!errorMessage.empty()) {
                BottomLine line;
                line.text = errorMessage;
                line.color = 12;
                bottom.push_back(line);
            }

            string reason = blockReason();
            BottomLine inputLine;

            if (!reason.empty()) {
                inputLine.text = reason;
                inputLine.color = 12;
            }
            else {
                inputLine.text = "> " + input;
                inputLine.color = 14;
                inputLine.leftAlign = true;
                inputLine.cursor = true;
                inputLine.keepTail = true;
            }

            bottom.push_back(inputLine);

            drawReaderFrame(
                displayTitle, blocks, state, bottom,
                selectionMode == 1
                    ? "CHáº¾ Äá»˜ XÃ“A: ENTER chá»n/bá» chá»n | DELETE láº§n ná»¯a: XÃ³a phÃ­a báº¡n | /: Há»§y | ESC: ThoÃ¡t cháº¿ Ä‘á»™"
                    : selectionMode == 2
                    ? "CHáº¾ Äá»˜ THU Há»’I: ENTER chá»n/bá» chá»n | / láº§n ná»¯a: Thu há»“i | DELETE: Há»§y | ESC: ThoÃ¡t cháº¿ Ä‘á»™"
                    : "GÃµ Ä‘á»ƒ soáº¡n tin | ENTER: Gá»­i | DELETE: Chá»n tin Ä‘á»ƒ xÃ³a | /: Chá»n tin Ä‘á»ƒ thu há»“i | F2: Äáº·t biá»‡t danh | Cuá»™n/â–²/â–¼: Xem tin"
            );
        },
        // Xá»­ lÃ½ phÃ­m
        [&](const KeyInput& key) -> bool {
            if (selectionMode == 0 && applyScrollKey(key, state)) {
                return false;
            }

            if (key.type == KEY_ESCAPE) {
                if (selectionMode != 0) {
                    selectionMode = 0;
                    markedMessages.clear();
                    return false;
                }
                return true;
            }

            auto visibleMessageIndices = [&]() {
                vector<int> ids;
                for (int i = 0; i < (int)messages.size(); ++i) {
                    if (messages[i].sender != "SYSTEM" && !hiddenKeys.count(messageKey(messages[i])))
                        ids.push_back(i);
                }
                return ids;
            };
            vector<int> visible = visibleMessageIndices();
            if (selectedMessage >= (int)visible.size()) selectedMessage = max(0, (int)visible.size() - 1);
            if (selectedMessage < 0) selectedMessage = 0;

            auto applyMarkedAction = [&](int mode) {
                bool changed = false;
                vector<ChatMessage> beforeAction = messages;
                for (ChatMessage& m : messages) {
                    if (!markedMessages.count(messageKey(m))) continue;
                    if (mode == 1) {
                        if (hideMessageLocally(myId, file, m)) {
                            hiddenKeys.insert(messageKey(m));
                            changed = true;
                        }
                    } else if (mode == 2 && m.sender == myId && m.text != WITHDRAWN_MESSAGE_TEXT) {
                        time_t sentAt, nowAt;
                        if (parseTimestamp(m.time, sentAt) &&
                            parseTimestamp(nowTimestamp(), nowAt) &&
                            nowAt >= sentAt && nowAt - sentAt <= 600) {
                            m.text = WITHDRAWN_MESSAGE_TEXT;
                            changed = true;
                        }
                    }
                }
                if (mode == 2 && changed && !rewriteMessages(file, messages)) {
                    messages = beforeAction;
                    changed = false;
                    errorMessage = "KhÃ´ng thá»ƒ lÆ°u tráº¡ng thÃ¡i thu há»“i tin nháº¯n.";
                }
                if (!changed && mode == 2 && errorMessage.empty())
                    errorMessage = "Chá»‰ thu há»“i Ä‘Æ°á»£c tin nháº¯n cá»§a báº¡n trong vÃ²ng 10 phÃºt.";
                markedMessages.clear();
                selectionMode = 0;
                lastStamp = fileStamp();
            };

            if (selectionMode != 0) {
                if (key.type == KEY_UP || key.type == KEY_SCROLL_UP) {
                    selectedMessage = max(0, selectedMessage - 1);
                    state.offset = max(0, state.offset - 3);
                }
                else if (key.type == KEY_DOWN || key.type == KEY_SCROLL_DOWN) {
                    selectedMessage = min(max(0, (int)visible.size() - 1), selectedMessage + 1);
                    state.offset = min(state.maxOffset, state.offset + 3);
                }
                else if (key.type == KEY_ENTER && !visible.empty()) {
                    const ChatMessage& m = messages[visible[selectedMessage]];
                    if (selectionMode == 1 || (selectionMode == 2 && m.sender == myId &&
                        m.text != WITHDRAWN_MESSAGE_TEXT)) {
                        string k = messageKey(m);
                        if (markedMessages.count(k)) markedMessages.erase(k); else markedMessages.insert(k);
                    } else if (selectionMode == 2) {
                        errorMessage = "Báº¡n chá»‰ cÃ³ thá»ƒ thu há»“i tin nháº¯n do chÃ­nh mÃ¬nh gá»­i.";
                    }
                }
                else if (key.type == KEY_DELETE) {
                    if (selectionMode == 1) applyMarkedAction(1);
                    else { selectionMode = 0; markedMessages.clear(); }
                }
                else if (key.type == KEY_CHARACTER && key.character == "/") {
                    if (selectionMode == 2) applyMarkedAction(2);
                    else { selectionMode = 0; markedMessages.clear(); }
                }
                return false;
            }

            if (key.type == KEY_DELETE) {
                selectionMode = 1; markedMessages.clear();
                selectedMessage = max(0, (int)visible.size() - 1);
                return false;
            }
            if (key.type == KEY_CHARACTER && key.character == "/" && input.empty()) {
                selectionMode = 2; markedMessages.clear();
                selectedMessage = max(0, (int)visible.size() - 1);
                return false;
            }
            if (key.type == KEY_F2 && !notifyUserId.empty()) {
                string oldName = getNickname(myId, notifyUserId);
                string newName = oldName;
                InputOptions nicknameOptions;
                nicknameOptions.title = "Äáº¶T BIá»†T DANH";
                nicknameOptions.prompt = "Nháº­p biá»‡t danh; Ä‘á»ƒ trá»‘ng Ä‘á»ƒ bá» biá»‡t danh.";
                nicknameOptions.maxLength = 40;
                if (!inputText(nicknameOptions, newName)) return false;
                if (setNickname(myId, notifyUserId, newName)) {
                    names.erase(notifyUserId);
                    displayTitle = "=============== TRÃ’ CHUYá»†N Vá»šI " + chatDisplayName(myId, notifyUserId) + " ===============";
                    string notice = trimString(newName).empty()
                        ? "Báº¡n Ä‘Ã£ bá» biá»‡t danh cá»§a " + getDisplayName(notifyUserId)
                        : "Báº¡n Ä‘Ã£ Ä‘áº·t biá»‡t danh cho " + getDisplayName(notifyUserId) + " lÃ  \"" + trimString(newName) + "\"";
                    if (appendMessage(file, "SYSTEM", notice)) {
                        ChatMessage systemMessage;
                        systemMessage.sender = "SYSTEM";
                        systemMessage.time = nowTimestamp();
                        systemMessage.text = notice;
                        messages.push_back(systemMessage);
                        lastStamp = fileStamp();
                        state.offset = INT_MAX / 2;
                    }
                    errorMessage = trimString(newName).empty() ? "ÄÃ£ bá» biá»‡t danh." : "ÄÃ£ cáº­p nháº­t biá»‡t danh.";
                } else errorMessage = "KhÃ´ng lÆ°u Ä‘Æ°á»£c biá»‡t danh.";
                return false;
            }

            errorMessage.clear();

            // KhÃ´ng Ä‘Æ°á»£c phÃ©p gá»­i (bá»‹ cháº·n, khÃ´ng cÃ²n lÃ  báº¡n bÃ¨...)
            if (!blockReason().empty()) {
                return false;
            }

            if (key.type == KEY_BACKSPACE) {
                popLastChar(input);
            }
            else if (key.type == KEY_ENTER) {
                string text = trimBlank(input);

                if (text.empty()) {
                    return false;
                }

                string time;

                // Nháº¯n láº§n Ä‘áº§u hoáº·c nháº¯n láº¡i sau nhiá»u ngÃ y thÃ¬ bÃ¡o cho ngÆ°á»i kia (khÃ´ng bÃ¡o má»—i tin)
                bool notifyOther = !notifyUserId.empty() &&
                                   shouldNotifyMessage(messages.empty() ? "" : messages.back().time);

                if (!appendMessage(file, myId, text, &time)) {
                    errorMessage = "KhÃ´ng gá»­i Ä‘Æ°á»£c tin nháº¯n, vui lÃ²ng thá»­ láº¡i!";
                    return false;
                }

                if (notifyOther) {
                    addNotification(notifyUserId, NOTI_MESSAGE, myId);
                }

                ChatMessage message;
                message.sender = myId;
                message.time = time;
                message.text = text;
                messages.push_back(message);
                lastStamp = fileStamp();    // Tin vá»«a gá»­i Ä‘Ã£ náº±m trong danh sÃ¡ch, khÃ´ng cáº§n Ä‘á»c láº¡i

                input.clear();
                state.offset = INT_MAX / 2;     // Cuá»™n xuá»‘ng tin má»›i nháº¥t
            }
            else if (key.type == KEY_CHARACTER) {
                bool leadingSpace = input.empty() && key.character == " ";

                if (!leadingSpace && textLength(input) < 500) {
                    input += key.character;
                }
            }

            return false;
        },
        // Má»—i 0,7 giÃ¢y khÃ´ng cÃ³ phÃ­m: kiá»ƒm tra tin nháº¯n má»›i cá»§a ngÆ°á»i kia vÃ  tá»± cáº­p nháº­t
        700,
        [&]() -> bool {
            bool changed = false;
            auto stamp = fileStamp();

            if (stamp != lastStamp) {
                lastStamp = stamp;

                // Äang xem cuá»‘i cuá»™c trÃ² chuyá»‡n thÃ¬ tá»± cuá»™n xuá»‘ng tin má»›i, Ä‘ang xem tin cÅ© thÃ¬ giá»¯ nguyÃªn chá»— Ä‘á»c
                bool atBottom = state.offset >= state.maxOffset;

                messages = readMessages(file);
                hiddenKeys = readHiddenMessageKeys(myId, file);

                if (atBottom) {
                    state.offset = INT_MAX / 2;
                }

                changed = true;
            }

            // Quan há»‡ thay Ä‘á»•i (bá»‹ cháº·n, bá»‹ xÃ³a khá»i nhÃ³m...) thÃ¬ cáº­p nháº­t dÃ²ng nháº­p tin
            string reason = blockReason();

            if (reason != lastReason) {
                lastReason = reason;
                changed = true;
            }

            return changed;
        }
    );

    hideCursor();
    setColor(7);
}

// Má»Ÿ cuá»™c trÃ² chuyá»‡n riÃªng vá»›i 1 ngÆ°á»i
void openPrivateChat(const string& myId, const string& otherId) {
    string title = "=============== TRÃ’ CHUYá»†N Vá»šI " + chatDisplayName(myId, otherId) + " ===============";

    runChat(myId, title, privateChatFile(myId, otherId), [&]() -> string {
        if (isBlockedEither(myId, otherId)) {
            return "Báº¡n khÃ´ng thá»ƒ nháº¯n tin cho ngÆ°á»i nÃ y!";
        }

        if (!areFriends(myId, otherId)) {
            return "Hai báº¡n khÃ´ng cÃ²n lÃ  báº¡n bÃ¨ nÃªn khÃ´ng thá»ƒ nháº¯n tin!";
        }

        return "";
    }, otherId);
}

fs::path groupDir(const string& groupId) {
    return fs::path("Data/Conversations/.Group") / groupId;
}

bool readGroup(const string& groupId, Group& group) {
    fs::path dir = groupDir(groupId);
    vector<string> info = readLines(dir / "info.txt");

    if (info.empty()) {
        return false;
    }

    Group current;
    current.id = groupId;

    for (const string& line : info) {
        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "name")              current.name = value;
        else if (key == "owner")        current.ownerId = value;
        else if (key == "created_at")   current.createdAt = value;
    }

    for (const string& line : readLines(dir / "members.txt")) {
        size_t first = line.find('|');
        GroupMember member;

        if (first == string::npos) {
            member.id = trimString(line);
        }
        else {
            member.id = trimString(line.substr(0, first));
            size_t second = line.find('|', first + 1);

            if (second == string::npos) {
                member.addedBy = trimString(line.substr(first + 1));
            }
            else {
                member.addedBy = trimString(line.substr(first + 1, second - first - 1));
                member.date = trimString(line.substr(second + 1));
            }
        }

        if (!member.id.empty()) {
            current.members.push_back(member);
        }
    }

    if (current.name.empty()) {
        return false;
    }

    group = current;
    return true;
}

vector<string> listGroupIds() {
    vector<string> ids;
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Conversations/.Group", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        string name = entry.path().filename().string();

        if (name.size() > 3 && name.compare(0, 3, "GRP") == 0) {
            ids.push_back(name);
        }
    }

    sort(ids.begin(), ids.end());
    return ids;
}

bool isGroupMember(const Group& group, const string& accountId) {
    for (const GroupMember& member : group.members) {
        if (member.id == accountId) {
            return true;
        }
    }

    return false;
}

string groupLastActivity(const Group& group) {
    vector<ChatMessage> messages = readMessages(groupDir(group.id) / "messages.txt");

    if (!messages.empty()) {
        return messages.back().time;
    }

    return group.createdAt;
}

vector<Group> getGroupsOf(const string& accountId) {
    vector<Group> groups;

    for (const string& id : listGroupIds()) {
        Group group;

        if (readGroup(id, group) && isGroupMember(group, accountId)) {
            groups.push_back(group);
        }
    }

    sort(groups.begin(), groups.end(), [](const Group& a, const Group& b) {
        string timeA = groupLastActivity(a);
        string timeB = groupLastActivity(b);

        if (timeA != timeB) return timeA > timeB;
        return a.id > b.id;
    });

    return groups;
}

/*
    XEM TRANG CÃ NHÃ‚N Cá»¦A NGÆ¯á»œI KHÃC VÃ€ CÃC THAO TÃC Báº N BÃˆ
                                                                */

string relationLabel(const string& status) {
    if (status == "FRIEND")             return "Báº¡n bÃ¨";
    if (status == "PENDING_SENT")       return "Báº¡n Ä‘Ã£ gá»­i lá»i má»i káº¿t báº¡n";
    if (status == "PENDING_RECEIVED")   return "NgÆ°á»i nÃ y Ä‘Ã£ gá»­i lá»i má»i cho báº¡n";
    if (status == "BLOCK")              return "Báº¡n Ä‘Ã£ cháº·n ngÆ°á»i nÃ y";
    return "ChÆ°a káº¿t báº¡n";
}

CardItem makePersonItem(const string& myId, const string& id, bool showMutual, const string& extra) {
    CardItem item;
    UserProfile profile;

    if (!findProfileById(id, profile)) {
        return item;
    }

    string name = profile.fullName.empty() ? profile.username : profile.fullName;

    item.key = id;
    item.card.head = name + (extra.empty() ? "" : " Â· " + extra);
    item.card.title = profile.bio.empty() ? "(chÆ°a cÃ³ giá»›i thiá»‡u)" : oneLine(profile.bio);
    item.card.titleColor = profile.bio.empty() ? 8 : 7;
    item.card.body = {showMutual ? to_string(countMutualFriends(myId, id)) + " báº¡n chung"
                                 : to_string(countFriends(id)) + " báº¡n bÃ¨"};
    item.card.bodyColor = 3;
    item.searchText = name + " " + profile.username;
    return item;
}

void personListScreen(
    const string& myId,
    const string& title,
    const string& emptyMessage,
    const string& selectHint,
    const function<vector<string>()>& idsOf,
    bool showMutual
) {
    CardListConfig config;
    config.title = title;
    config.emptyMessage = emptyMessage;
    config.selectHint = selectHint;
    config.noun = "ngÆ°á»i";
    config.moreWord = "ngÆ°á»i ná»¯a";
    config.pageSize = 5;

    runCardList(
        config,
        [&](int) {
            vector<CardItem> items;

            for (const string& id : idsOf()) {
                CardItem item = makePersonItem(myId, id, showMutual);

                if (!item.key.empty()) {
                    items.push_back(item);
                }
            }

            return items;
        },
        [&](const string& id) {
            viewUserProfile(myId, id);
        }
    );
}

void mutualFriendsScreen(const string& myId, const string& otherId) {
    personListScreen(
        myId,
        "=============== Báº N CHUNG Vá»šI " + getDisplayName(otherId) + " ===============",
        "Hai báº¡n khÃ´ng cÃ³ báº¡n chung nÃ o.",
        "Xem trang cÃ¡ nhÃ¢n",
        [&]() {
            vector<string> ids;

            for (const string& id : getMutualFriendIds(myId, otherId)) {
                UserProfile profile;

                if (findProfileById(id, profile) && profile.status != "LOCKED" &&
                    getRelation(id, myId) != "BLOCK") {
                    ids.push_back(id);
                }
            }

            return ids;
        }
    );
}

// Xem trang cÃ¡ nhÃ¢n cá»§a 1 ngÆ°á»i (myId: ID cá»§a tÃ´i). CÃ¡c nÃºt thay Ä‘á»•i theo quan há»‡ hiá»‡n táº¡i
void viewUserProfile(const string& myId, const string& otherId) {
    while (true) {
        UserProfile profile;

        if (!findProfileById(otherId, profile)) {
            notify("KhÃ´ng Ä‘á»c Ä‘Æ°á»£c thÃ´ng tin ngÆ°á»i nÃ y!");
            return;
        }

        string status = getRelation(myId, otherId);

        // NgÆ°á»i kia Ä‘Ã£ cháº·n mÃ¬nh thÃ¬ khÃ´ng Ä‘Æ°á»£c xem
        if (status != "BLOCK" && getRelation(otherId, myId) == "BLOCK") {
            notify("Báº¡n khÃ´ng thá»ƒ xem trang cÃ¡ nhÃ¢n cá»§a ngÆ°á»i nÃ y!");
            return;
        }

        int mutualCount = countMutualFriends(myId, otherId);

        vector<pair<string, string>> info =
            buildProfileInfo(profile, mutualCount);
        info.push_back({"Quan Há»‡", relationLabel(status)});

        vector<string> items, actions;
        auto add = [&](const string& label, const string& action) {
            items.push_back(label);
            actions.push_back(action);
        };

        if (status != "BLOCK") {
            if (status == "FRIEND") {
                add("Nháº¯n Tin", "chat");
            }

            add("Xem BÃ i ÄÄƒng", "posts");

            if (mutualCount > 0) {
                add("Xem Báº¡n Chung (" + to_string(mutualCount) + ")", "mutual");
            }

            add("BÃ¡o CÃ¡o NgÆ°á»i DÃ¹ng", "report");
        }

        add("Quay Láº¡i", "back");

        int choice = menuWithInfo(
            "=============== TRANG CÃ NHÃ‚N ===============",
            info,
            items,
            "Sá»­ dá»¥ng â–² / â–¼ | ENTER: Chá»n | ESC: Quay láº¡i"
        );

        if (choice == 0) {
            return;
        }

        const string& action = actions[choice - 1];

        if (action == "back") {
            return;
        }
        else if (action == "chat") {
            openPrivateChat(myId, otherId);
        }
        else if (action == "posts") {
            viewUserPosts(myId, otherId);
        }
        else if (action == "mutual") {
            mutualFriendsScreen(myId, otherId);
        }
        else if (action == "report") {
            reportUserFlow(myId, otherId);
        }
    }
}

bool canSuggestTo(const string& myId, const string& id, UserProfile& profile) {
    if (id == myId) {
        return false;
    }

    if (!getRelation(myId, id).empty()) {
        return false;
    }

    if (getRelation(id, myId) == "BLOCK") {
        return false;
    }

    if (!findProfileById(id, profile)) {
        return false;
    }

    if (profile.status == "LOCKED") {
        return false;
    }

    return true;
}

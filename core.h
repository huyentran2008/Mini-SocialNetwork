#ifndef CORE_H
#define CORE_H

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
#include <set>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono_literals;
namespace fs = filesystem;

// ===== Biáº¿n toÃ n cá»¥c cá»§a console (Ä‘á»‹nh nghÄ©a trong core.cpp) =====
extern HANDLE hOriginal;
extern HANDLE hConsole;
extern HANDLE hFront;
extern bool   bufferReady;
extern HANDLE hInput;
extern DWORD  originalInputMode;
extern bool   inputModeSaved;

// ===== Kiá»ƒu dá»¯ liá»‡u vÃ  háº±ng sá»‘ =====

enum KeyType {
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_NEWLINE,
    KEY_SCROLL_UP,
    KEY_SCROLL_DOWN,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CHARACTER,
    KEY_TICK,
    KEY_TAB,
    KEY_DELETE,
    KEY_F2
};

struct KeyInput {
    KeyType type;
    int asciiCode;
    string character;
};

struct UserProfile {
    string id;
    string username;
    string fullName;
    string birthday;
    string gender;
    string bio;
    string createdAt;
    string status;
    string residence;
    string hometown;
    string school;
    string workplace;
    string hobbies;
};

struct FriendEntry {
    string id;
    string status;
    string date;
};

struct TextBlock {
    string text;
    int color = 7;
    int align = 0;
    int indent = 0;
    int widthPercent = 100;
    bool raw = false;
    bool keepSpaces = false;
};

struct DisplayLine {
    string text;
    int color;
    int offset;
};

struct BottomLine {
    string text;
    int color = 7;
    bool leftAlign = false;
    bool cursor = false;
    bool keepTail = false;
};

struct ReaderState {
    int offset = 0;
    int maxOffset = 0;
    int viewHeight = 1;
    bool wantEndCursor = false;
    bool endVisible = false;
    int endX = 0;
    int endY = 0;
};

/*
    NHáº¬P VÄ‚N Báº¢N (1 DÃ’NG HOáº¶C NHIá»€U DÃ’NG)
                                            */

enum InputMode {
    INPUT_FREE = 0,     // Nháº­n má»i kÃ½ tá»± (ká»ƒ cáº£ chá»¯ cÃ³ dáº¥u)
    INPUT_ASCII = 1,    // Chá»‰ nháº­n kÃ½ tá»± ASCII khÃ´ng khoáº£ng tráº¯ng (máº­t kháº©u)
    INPUT_DATE = 2      // Chá»‰ nháº­n chá»¯ sá»‘, tá»± thÃªm '-' (DD-MM-YYYY)
};

struct InputOptions {
    string title;
    string prompt;                  // DÃ²ng gá»£i Ã½ phÃ­a trÃªn Ã´ nháº­p
    string footer;                  // Rá»—ng: dÃ¹ng menu hÆ°á»›ng dáº«n máº·c Ä‘á»‹nh
    int maxLength = 100;
    int mode = INPUT_FREE;
    bool multiline = false;         // true: Shift+ENTER hoáº·c Ctrl+ENTER Ä‘á»ƒ xuá»‘ng dÃ²ng
    bool mask = false;              // true: hiá»ƒn thá»‹ dáº¥u '*'
    function<string(const string&)> validate;   // Tráº£ vá» thÃ´ng bÃ¡o lá»—i, rá»—ng náº¿u há»£p lá»‡
};

struct Card {
    string head;
    string title;
    vector<string> body;
    int headColor = 14;
    int titleColor = 11;
    int bodyColor = 7;
};

struct CardItem {
    string key;
    Card card;
    string searchText;
};

struct CardListConfig {
    string title;
    string emptyMessage;
    string selectHint = "Xem";
    string noun = "má»¥c";
    string moreWord = "má»¥c ná»¯a";
    int pageSize = 5;
    bool searchable = true;
};

/*
    THÃ”NG BÃO Cá»¦A NGÆ¯á»œI DÃ™NG (Data/Account/ACCxxxxxx/notifications.txt)
    Má»—i dÃ²ng: LOáº I|NGÆ¯á»œI GÃ‚Y RA|Äá»I TÆ¯á»¢NG|THá»œI GIAN|ÄÃƒ Äá»ŒC (0/1)
        FRIEND_REQUEST : cÃ³ ngÆ°á»i gá»­i lá»i má»i káº¿t báº¡n cho mÃ¬nh
        FRIEND_ACCEPT  : cÃ³ ngÆ°á»i cháº¥p nháº­n lá»i má»i káº¿t báº¡n cá»§a mÃ¬nh
        POST_LIKE      : cÃ³ ngÆ°á»i thÃ­ch bÃ i Ä‘Äƒng cá»§a mÃ¬nh  (Äá»I TÆ¯á»¢NG = ID bÃ i Ä‘Äƒng)
        POST_COMMENT   : cÃ³ ngÆ°á»i bÃ¬nh luáº­n bÃ i Ä‘Äƒng cá»§a mÃ¬nh (Äá»I TÆ¯á»¢NG = ID bÃ i Ä‘Äƒng)
        MESSAGE        : cÃ³ ngÆ°á»i nháº¯n tin (chá»‰ bÃ¡o khi nháº¯n láº§n Ä‘áº§u, hoáº·c nháº¯n láº¡i sau
                         NOTI_MESSAGE_GAP_SECONDS khÃ´ng ai nháº¯n - khÃ´ng bÃ¡o má»—i tin nháº¯n)
    Ná»™i dung chá»¯ cá»§a thÃ´ng bÃ¡o Ä‘Æ°á»£c táº¡o lÃºc hiá»ƒn thá»‹ (theo tÃªn hiá»‡n táº¡i cá»§a ngÆ°á»i gá»­i).
                                                                                        */

const string NOTI_FRIEND_REQUEST = "FRIEND_REQUEST";

const string NOTI_FRIEND_ACCEPT = "FRIEND_ACCEPT";

const string NOTI_POST_LIKE = "POST_LIKE";

const string NOTI_POST_COMMENT = "POST_COMMENT";

const string NOTI_MESSAGE = "MESSAGE";

const long long NOTI_MESSAGE_GAP_SECONDS = 3LL * 24 * 3600;

   // 3 ngÃ y khÃ´ng nháº¯n thÃ¬ nháº¯n láº¡i sáº½ bÃ¡o

   // 3 ngÃ y khÃ´ng nháº¯n thÃ¬ nháº¯n láº¡i sáº½ bÃ¡o
const size_t NOTI_MAX_KEEP = 200;

                             // Má»—i ngÆ°á»i giá»¯ tá»‘i Ä‘a 200 thÃ´ng bÃ¡o gáº§n nháº¥t

struct Notification {
    string type;
    string fromId;
    string target;
    string time;
    bool read = false;
};

/*
    BÃO CÃO VI PHáº M (Data/Report/reports.txt)
    Má»—i dÃ²ng: LOáº I|NGÆ¯á»œI BÃO CÃO|Äá»I TÆ¯á»¢NG|THá»œI GIAN|TRáº NG THÃI|LÃ DO
        TRáº NG THÃI = PENDING (chÆ°a xá»­ lÃ½) hoáº·c DONE (Ä‘Ã£ xá»­ lÃ½). File cÅ© khÃ´ng cÃ³ cá»™t nÃ y váº«n Ä‘á»c Ä‘Æ°á»£c (coi lÃ  chÆ°a xá»­ lÃ½)
        LOáº I = USER: bÃ¡o cÃ¡o ngÆ°á»i dÃ¹ng (Äá»I TÆ¯á»¢NG lÃ  ID tÃ i khoáº£n, vÃ­ dá»¥ ACC000002)
        LOáº I = POST: bÃ¡o cÃ¡o bÃ i viáº¿t   (Äá»I TÆ¯á»¢NG lÃ  ID bÃ i viáº¿t, vÃ­ dá»¥ POST000002)
    Quáº£n trá»‹ viÃªn xem cÃ¡c bÃ¡o cÃ¡o nÃ y trong menu quáº£n trá»‹.
                                                                                    */

const fs::path REPORT_FILE = "Data/Report/reports.txt";

const string REPORT_USER = "USER";

const string REPORT_POST = "POST";

struct Report {
    string type;        // REPORT_USER hoáº·c REPORT_POST
    string reporterId;  // NgÆ°á»i gá»­i bÃ¡o cÃ¡o
    string targetId;    // NgÆ°á»i dÃ¹ng / bÃ i viáº¿t bá»‹ bÃ¡o cÃ¡o
    string time;        // YYYY-MM-DD HH:MM:SS
    string reason;      // LÃ½ do
    bool handled = false;   // true: quáº£n trá»‹ viÃªn Ä‘Ã£ xá»­ lÃ½ (DONE), false: chÆ°a xá»­ lÃ½ (PENDING)
};

/*
    BÃ€I ÄÄ‚NG (Data/Post/POST<sá»‘>/ post.txt, likes.txt, comments.txt)
                                                                        */

struct Post {
    string id;
    string authorId;
    string visibility;      // PUBLIC, FRIEND, PRIVATE
    string createdAt;       // YYYY-MM-DD HH:MM:SS
    string editedAt;        // Rá»—ng náº¿u chÆ°a chá»‰nh sá»­a
    string title;           // TiÃªu Ä‘á» (bÃ i cÅ© cÃ³ thá»ƒ Ä‘á»ƒ trá»‘ng)
    string content;
    bool hidden = false;    // true: bá»‹ quáº£n trá»‹ viÃªn áº©n (chá»‰ tÃ¡c giáº£ vÃ  quáº£n trá»‹ viÃªn xem Ä‘Æ°á»£c)
};

struct PostComment {
    string authorId;
    string time;
    string text;
};

// Danh sÃ¡ch bÃ i Ä‘Äƒng Ä‘á»ƒ chá»n Ä‘á»c. loader tráº£ vá» cÃ¡c bÃ i Ä‘Äƒng cáº§n hiá»ƒn thá»‹
const int POST_PAGE_SIZE = 5;

       // Má»—i láº§n hiá»ƒn thá»‹ / má»Ÿ rá»™ng thÃªm 5 bÃ i

       // Má»—i láº§n hiá»ƒn thá»‹ / má»Ÿ rá»™ng thÃªm 5 bÃ i
const int POST_CONTENT_LINES = 2;

   // Ná»™i dung má»—i bÃ i chá»‰ in tá»‘i Ä‘a 2 dÃ²ng rá»“i "..."

struct ChatMessage {
    string sender;
    string time;
    string text;
};

// Tin nháº¯n bá»‹ thu há»“i Ä‘Æ°á»£c Ä‘Ã¡nh dáº¥u ngay trong file há»™i thoáº¡i Ä‘á»ƒ cáº£ hai phÃ­a cÃ¹ng tháº¥y.
const string WITHDRAWN_MESSAGE_TEXT = "__MINISOCIAL_WITHDRAWN_MESSAGE__";

struct GroupMember {
    string id;
    string addedBy;
    string date;
};

struct Group {
    string id;
    string name;
    string ownerId;
    string createdAt;
    vector<GroupMember> members;
};

const int SUGGEST_MAX_DEPTH = 3;

const int SUGGEST_MAX_RESULT = 10;

const int SUGGEST_MAX_FALLBACK = 5;

struct Suggestion {
    string id;
    string name;
    int distance = 0;
    int mutual = 0;
    int friendCount = 0;
    string reason;
    int score = 0;
    int commonGroups = 0;
};

// ===== Khai bÃ¡o hÃ m =====

// ---------- Console, nháº­p phÃ­m, mÃ n hÃ¬nh Ä‘á»‡m Ä‘Ã´i (double buffer), menu ----------
string wideToUtf8(const wstring& w);

KeyInput readKey(int timeoutMs = -1);

// Báº­t nháº­n sá»± kiá»‡n lÄƒn chuá»™t (táº¯t Quick Edit vÃ¬ nÃ³ cháº·n sá»± kiá»‡n chuá»™t)
void enableMouseInput();

// KhÃ´i phá»¥c cháº¿ Ä‘á»™ nháº­p gá»‘c cá»§a console khi thoÃ¡t
void restoreInputMode();

void hideCursor();

void showCursor();

void setColor(int color);

void gotoxy(int x, int y);

void clearScreen();

void writeText(const string& s);

// Táº¡o 2 screen buffer. Náº¿u lá»—i thÃ¬ giá»¯ nguyÃªn buffer gá»‘c (khÃ´ng double buffer)
void initDoubleBuffer();

void beginFrame();

void endFrame();

// KhÃ´i phá»¥c buffer gá»‘c khi thoÃ¡t chÆ°Æ¡ng trÃ¬nh
void cleanupDoubleBuffer();

int textLength(const string& s);

void popLastChar(string& s);

string trimString(const string& s);

void runLoop(
    const function<void()>& draw,
    const function<bool(const KeyInput&)>& handleKey,
    int tickMs = 0,
    const function<bool()>& onTick = nullptr
);

bool moveSelection(int& selected, int itemCount, KeyType type);

void printCenter(const string& s, int y, int color,
                 int width, int left);

void printCenter(const string& s);

void drawMenu(
    int selected,
    const string& title,
    const vector<string>& items,
    const string& footer
);

// Váº½ menu cÃ³ thÃªm khá»‘i thÃ´ng tin chá»‰ Ä‘á»ƒ xem (nhÃ£n, giÃ¡ trá»‹) á»Ÿ phÃ­a trÃªn
void drawMenuWithInfo(
    int selected,
    const string& title,
    const vector<pair<string, string>>& info,
    const vector<string>& items,
    const string& footer
);

// Hiá»ƒn thá»‹ menu cÃ³ khá»‘i thÃ´ng tin vÃ  nháº­n lá»±a chá»n
int menuWithInfo(
    const string& title,
    const vector<pair<string, string>>& info,
    const vector<string>& items,
    const string& footer
);

void showMessageScreen(
    const string& title,
    const string& message,
    const string& footer
);

int menu(
    const string& title,
    const vector<string>& items,
    const string& footer
);

// ---------- Há»“ sÆ¡ ngÆ°á»i dÃ¹ng, danh sÃ¡ch báº¡n bÃ¨, báº¡n chung ----------
vector<string> splitList(const string& text, char separator = ';');

string joinList(const vector<string>& items, const string& separator);

bool isValidAccountId(const string& id);

bool readProfileFile(const fs::path& profileFile, UserProfile& profile);

bool findProfileById(const string& accountId, UserProfile& profile);

// Äá»•i ngÃ y lÆ°u trong file sang dáº¡ng DD-MM-YYYY Ä‘á»ƒ hiá»ƒn thá»‹
// (nháº­n cáº£ YYYY-MM-DD vÃ  DD-MM-YYYY, sai dáº¡ng thÃ¬ tráº£ vá» chuá»—i rá»—ng)
string toDisplayDate(const string& s);

vector<FriendEntry> readFriendList(const string& accountId);

vector<string> getFriendIds(const string& accountId);

int countFriends(const string& accountId);

int countMutualFriends(const string& idA, const string& idB);

vector<string> getMutualFriendIds(const string& idA, const string& idB);

// Táº¡o khá»‘i thÃ´ng tin hiá»ƒn thá»‹ cá»§a 1 há»“ sÆ¡: {nhÃ£n, giÃ¡ trá»‹}
// mutualFriends >= 0 thÃ¬ thÃªm dÃ²ng "Sá»‘ Báº¡n Chung" (dÃ¹ng khi xem trang cá»§a ngÆ°á»i khÃ¡c)
// showEmpty = true: cÃ¡c má»¥c tÃ¹y chá»n bá» trá»‘ng váº«n hiá»‡n vá»›i chá»¯ "(chÆ°a cÃ³)" (dÃ¹ng cho trang cá»§a tÃ´i)
vector<pair<string, string>> buildProfileInfo(const UserProfile& profile,
                                              int mutualFriends = -1,
                                              bool showEmpty = false);

// ---------- Xá»­ lÃ½ chuá»—i, thá»i gian, Ä‘á»c/ghi file ----------
string cutChars(const string& s, int n);

string tailChars(const string& s, int n);

string cutText(const string& s, int maxLen);

// Cáº¯t khoáº£ng tráº¯ng vÃ  xuá»‘ng dÃ²ng á»Ÿ Ä‘áº§u vÃ  cuá»‘i chuá»—i
string trimBlank(const string& s);

string oneLine(const string& s);

unsigned int decodeAt(const string& s, size_t i, int& length);

string foldText(const string& s);

vector<string> wrapParagraph(const string& p, int width);

vector<string> wrapText(const string& text, int width);

// MÃ£ hÃ³a ná»™i dung nhiá»u dÃ²ng thÃ nh 1 dÃ²ng Ä‘á»ƒ lÆ°u file (\ -> \\, xuá»‘ng dÃ²ng -> \n)
string escapeText(const string& s);

string unescapeText(const string& s);

// NgÃ y hiá»‡n táº¡i dáº¡ng YYYY-MM-DD
string todayIso();

// Thá»i Ä‘iá»ƒm hiá»‡n táº¡i dáº¡ng "YYYY-MM-DD HH:MM:SS"
string nowTimestamp();

// Äá»c chuá»—i "YYYY-MM-DD HH:MM:SS" thÃ nh time_t
bool parseTimestamp(const string& s, time_t& result);

// "YYYY-MM-DD..." -> "DD/MM/YYYY"
string toSlashDate(const string& iso);

// Thá»i gian hiá»ƒn thá»‹: "x phÃºt / giá» / ngÃ y / thÃ¡ng trÆ°á»›c", quÃ¡ 1 nÄƒm thÃ¬ hiá»‡n dd/mm/yyyy
string timeAgo(const string& timestamp);

vector<string> readLines(const fs::path& path);

// Ghi Ä‘Ã¨ toÃ n bá»™ file báº±ng cÃ¡c dÃ²ng
bool writeLines(const fs::path& path, const vector<string>& lines);

// ThÃªm 1 dÃ²ng vÃ o cuá»‘i file (tá»± táº¡o thÆ° má»¥c náº¿u chÆ°a cÃ³)
bool appendLine(const fs::path& path, const string& line);

vector<string> listAccountIds();

string getDisplayName(const string& accountId);

int getConsoleWidth();

void notify(const string& message);

// ---------- MÃ n hÃ¬nh Ä‘á»c (reader), nháº­p vÄƒn báº£n, danh sÃ¡ch tháº» (card) ----------
TextBlock makeBlock(const string& text, int color = 7, int align = 0,
                    int indent = 0, int widthPercent = 100);

vector<string> wrapKeepSpaces(const string& text, int width);

vector<DisplayLine> layoutBlocks(const vector<TextBlock>& blocks, int blockWidth);

vector<string> splitFooter(const string& footer, int maxLen);

void printAt(int x, int y, const string& s, int color);

int computeReaderBlockWidth(int width);

void drawReaderFrame(
    const string& title,
    const vector<TextBlock>& blocks,
    ReaderState& state,
    const vector<BottomLine>& bottom,
    const string& footer
);

// Xá»­ lÃ½ phÃ­m cuá»™n. Tráº£ vá» true náº¿u phÃ­m vá»«a nháº¥n lÃ  phÃ­m cuá»™n
bool applyScrollKey(const KeyInput& key, ReaderState& state);

// MÃ n hÃ¬nh Ä‘á»c: tráº£ vá» 0 khi nháº¥n ESC, ngÆ°á»£c láº¡i tráº£ vá» mÃ£ phÃ­m hÃ nh Ä‘á»™ng
// (chá»¯ HOA náº±m trong actionKeys, hoáº·c '\n' náº¿u actionKeys cÃ³ '\n' vÃ  nháº¥n ENTER)
int runReader(
    const string& title,
    const vector<TextBlock>& blocks,
    ReaderState& state,
    const string& footer,
    const string& actionKeys
);

// Há»i xÃ¡c nháº­n báº±ng menu KhÃ´ng / CÃ³ (máº·c Ä‘á»‹nh chá»n "KhÃ´ng"). Tráº£ vá» true náº¿u chá»n "CÃ³"
bool confirmScreen(const string& question);

// MÃ n hÃ¬nh nháº­p vÄƒn báº£n. Tráº£ vá» true náº¿u xÃ¡c nháº­n (ENTER), false náº¿u nháº¥n ESC.
// value vá»«a lÃ  giÃ¡ trá»‹ ban Ä‘áº§u vá»«a lÃ  káº¿t quáº£
bool inputText(const InputOptions& options, string& value);

bool editTextKey(const KeyInput& key, string& text, int maxChars);

void buildCardBlocks(
    const vector<Card>& cards,
    int selected,
    int blockWidth,
    vector<TextBlock>& blocks,
    vector<int>& startLine,
    vector<int>& endLine
);

void ensureLinesVisible(ReaderState& state, int startL, int endL);

TextBlock makeCenteredNotice(const string& text, int color, int blockWidth);

BottomLine makeSearchLine(const string& query);

void runCardList(
    const CardListConfig& config,
    const function<vector<CardItem>(int textWidth)>& source,
    const function<void(const string& key)>& onSelect
);

// ---------- ThÃ´ng bÃ¡o ----------
fs::path notificationFile(const string& accountId);

// Äá»c thÃ´ng bÃ¡o cá»§a 1 tÃ i khoáº£n (cÅ© nháº¥t á»Ÿ Ä‘áº§u)
vector<Notification> readNotifications(const string& accountId);

bool writeNotifications(const string& accountId, vector<Notification> list);

// Gá»­i thÃ´ng bÃ¡o cho toId. KhÃ´ng bÃ¡o cho chÃ­nh mÃ¬nh, khÃ´ng bÃ¡o trÃ¹ng thÃ´ng bÃ¡o chÆ°a Ä‘á»c giá»‘ng há»‡t
// (thÃ­ch bÃ i thÃ¬ má»—i ngÆ°á»i chá»‰ bÃ¡o 1 láº§n dÃ¹ báº¥m thÃ­ch / bá» thÃ­ch nhiá»u láº§n)
void addNotification(const string& toId, const string& type, const string& fromId,
                     const string& target = "");

// Gá»¡ thÃ´ng bÃ¡o "lá»i má»i káº¿t báº¡n" cá»§a fromId khá»i há»™p thÃ´ng bÃ¡o cá»§a owner
// (lá»i má»i Ä‘Ã£ Ä‘Æ°á»£c cháº¥p nháº­n, tá»« chá»‘i hoáº·c há»§y thÃ¬ thÃ´ng bÃ¡o khÃ´ng cÃ²n Ã½ nghÄ©a)
void dropFriendRequestNotifications(const string& ownerId, const string& fromId);

// Tin nháº¯n má»›i cÃ³ cáº§n bÃ¡o cho ngÆ°á»i kia khÃ´ng: nháº¯n láº§n Ä‘áº§u (lastMessageTime rá»—ng)
// hoáº·c Ä‘Ã£ lÃ¢u (>= NOTI_MESSAGE_GAP_SECONDS) chÆ°a ai nháº¯n trong cuá»™c trÃ² chuyá»‡n
bool shouldNotifyMessage(const string& lastMessageTime);

// ---------- Quan há»‡ báº¡n bÃ¨ / cháº·n ----------
// Ghi láº¡i toÃ n bá»™ friend.txt cá»§a 1 tÃ i khoáº£n
bool writeFriendList(const string& accountId, const vector<FriendEntry>& list);

string getRelation(const string& owner, const string& other);

// Äáº·t tráº¡ng thÃ¡i owner -> other (status rá»—ng = xÃ³a quan há»‡)
bool setRelation(const string& owner, const string& other, const string& status);

// Hai tÃ i khoáº£n cÃ³ lÃ  báº¡n bÃ¨ khÃ´ng
bool areFriends(const string& a, const string& b);

// CÃ³ ai cháº·n ai giá»¯a hai tÃ i khoáº£n khÃ´ng
bool isBlockedEither(const string& a, const string& b);

// ---------- BÃ¡o cÃ¡o vi pháº¡m ----------
// Äá»c toÃ n bá»™ bÃ¡o cÃ¡o (theo thá»© tá»± Ä‘Ã£ gá»­i, cÅ© nháº¥t á»Ÿ Ä‘áº§u)
vector<Report> readReports();

// Ghi Ä‘Ã¨ toÃ n bá»™ bÃ¡o cÃ¡o
bool writeReports(const vector<Report>& reports);

// NgÆ°á»i nÃ y Ä‘Ã£ bÃ¡o cÃ¡o Ä‘á»‘i tÆ°á»£ng nÃ y chÆ°a
bool hasReported(const string& type, const string& reporterId, const string& targetId);

// Gá»­i 1 bÃ¡o cÃ¡o má»›i
bool addReport(const string& type, const string& reporterId, const string& targetId,
               const string& reason);

// XÃ³a má»i bÃ¡o cÃ¡o vá» 1 Ä‘á»‘i tÆ°á»£ng (tÃ i khoáº£n hoáº·c bÃ i viáº¿t Ä‘Ã£ bá»‹ xÃ³a)
void removeReportsAbout(const string& targetId);

// Nháº­p lÃ½ do bÃ¡o cÃ¡o. Tráº£ vá» false náº¿u nháº¥n ESC
bool inputReportReason(const string& title, const string& prompt, string& reason);

// BÃ¡o cÃ¡o 1 ngÆ°á»i dÃ¹ng vá»›i lÃ½ do do ngÆ°á»i bÃ¡o cÃ¡o nháº­p
void reportUserFlow(const string& myId, const string& targetId);

// BÃ¡o cÃ¡o 1 bÃ i viáº¿t vá»›i lÃ½ do do ngÆ°á»i bÃ¡o cÃ¡o nháº­p
void reportPostFlow(const string& myId, const string& postId);

// ---------- BÃ i Ä‘Äƒng ----------
fs::path postDir(const string& postId);

string visibilityLabel(const string& visibility);

// Äá»c 1 bÃ i Ä‘Äƒng
bool readPost(const string& postId, Post& post);

// Ghi 1 bÃ i Ä‘Äƒng
bool writePost(const Post& post);

// ID cá»§a má»i bÃ i Ä‘Äƒng
vector<string> listPostIds();

// NgÆ°á»i xem cÃ³ Ä‘Æ°á»£c xem bÃ i Ä‘Äƒng nÃ y khÃ´ng
bool canViewPost(const string& viewerId, const Post& post);

// Danh sÃ¡ch ID Ä‘Ã£ thÃ­ch bÃ i Ä‘Äƒng
vector<string> readLikes(const string& postId);

// ThÃ­ch / bá» thÃ­ch. Tráº£ vá» true náº¿u sau khi báº¥m lÃ  Ä‘ang thÃ­ch
bool toggleLike(const string& postId, const string& accountId);

// Äá»c cÃ¡c bÃ¬nh luáº­n (má»—i dÃ²ng: ACCID|THá»œI GIAN|Ná»˜I DUNG)
vector<PostComment> readComments(const string& postId);

bool addComment(const string& postId, const string& accountId, const string& text);

// CÃ¡c bÃ i Ä‘Äƒng mÃ  viewerId Ä‘Æ°á»£c xem, má»›i nháº¥t á»Ÿ trÃªn.
// authorFilter khÃ´ng rá»—ng: chá»‰ láº¥y bÃ i cá»§a ngÆ°á»i Ä‘Ã³
vector<Post> loadPosts(const string& viewerId, const string& authorFilter);

// Ná»™i dung xem trÆ°á»›c (dÃ²ng Ä‘áº§u tiÃªn) cá»§a bÃ i Ä‘Äƒng
string previewText(const string& content, int maxLen);

// TiÃªu Ä‘á» hiá»ƒn thá»‹ trong danh sÃ¡ch (bÃ i cÅ© chÆ°a cÃ³ tiÃªu Ä‘á» thÃ¬ láº¥y dÃ²ng Ä‘áº§u cá»§a ná»™i dung)
string displayTitle(const Post& post, int maxLen);

// Chá»n quyá»n riÃªng tÆ° báº±ng menu. chosen = false náº¿u nháº¥n ESC
string chooseVisibility(const string& title, const string& footer, bool& chosen);

// XÃ³a háº³n 1 bÃ i Ä‘Äƒng
void deletePost(const string& postId);

// Äá»c 1 bÃ i Ä‘Äƒng: tiÃªu Ä‘á» cá»‘ Ä‘á»‹nh, ná»™i dung (kÃ¨m bÃ¬nh luáº­n) cuá»™n Ä‘Æ°á»£c, menu hÆ°á»›ng dáº«n cá»‘ Ä‘á»‹nh
void viewPost(const string& myId, const string& postId);

// Tháº» hiá»ƒn thá»‹ cá»§a 1 bÃ i Ä‘Äƒng: ngÆ°á»i Ä‘Äƒng - thá»i gian / tiÃªu Ä‘á» / ná»™i dung rÃºt gá»n
Card makePostCard(const Post& post, int textWidth);

// Danh sÃ¡ch bÃ i Ä‘Äƒng. Chá»‰ hiá»‡n vÃ i bÃ i má»›i nháº¥t: M xem thÃªm, A xem táº¥t cáº£, S tÃ¬m kiáº¿m (gÃµ tá»›i Ä‘Ã¢u lá»c tá»›i Ä‘Ã³).
// Má»—i bÃ i gá»“m 3 pháº§n: ngÆ°á»i Ä‘Äƒng - thá»i gian / tiÃªu Ä‘á» / ná»™i dung rÃºt gá»n, cÃ¡c bÃ i cÃ¡ch nhau 1 dÃ²ng trá»‘ng.
// BÃ i Ä‘ang chá»n Ä‘Æ°á»£c Ä‘á»•i mÃ u ná»n. â–² / â–¼, con lÄƒn chuá»™t, PgUp / PgDn, Home / End Ä‘á»ƒ di chuyá»ƒn
void browsePosts(
    const string& myId,
    const string& title,
    const function<vector<Post>()>& loader,
    bool showAuthor,
    const string& emptyMessage
);

// Xem cÃ¡c bÃ i Ä‘Äƒng cá»§a 1 ngÆ°á»i mÃ  mÃ¬nh Ä‘Æ°á»£c phÃ©p xem
void viewUserPosts(const string& myId, const string& otherId);

// ---------- TrÃ² chuyá»‡n riÃªng ----------
vector<ChatMessage> readMessages(const fs::path& file);

// Ghi thÃªm 1 tin nháº¯n vÃ o cuá»‘i file há»™i thoáº¡i
bool appendMessage(const fs::path& file, const string& sender, const string& text,
                   string* timeOut = nullptr);

fs::path privateChatFile(const string& a, const string& b);

string messageKey(const ChatMessage& m);

fs::path hiddenMessagesFile(const string& myId);

set<string> readHiddenMessageKeys(const string& myId, const fs::path& conversationFile);

bool hideMessageLocally(const string& myId, const fs::path& conversationFile, const ChatMessage& m);

bool rewriteMessages(const fs::path& file, const vector<ChatMessage>& messages);

fs::path nicknamesFile(const string& myId);

string getNickname(const string& myId, const string& otherId);

bool setNickname(const string& myId, const string& otherId, const string& nickname);

string chatDisplayName(const string& myId, const string& otherId);

// MÃ n hÃ¬nh trÃ² chuyá»‡n: tiÃªu Ä‘á» cá»‘ Ä‘á»‹nh - lá»‹ch sá»­ tin nháº¯n (cuá»™n báº±ng chuá»™t) - Ã´ nháº­p - menu hÆ°á»›ng dáº«n.
// blockReason(): tráº£ vá» rá»—ng náº¿u Ä‘Æ°á»£c phÃ©p gá»­i, ngÆ°á»£c láº¡i lÃ  lÃ½ do khÃ´ng gá»­i Ä‘Æ°á»£c
void runChat(
    const string& myId,
    const string& title,
    const fs::path& file,
    const function<string()>& blockReason,
    const string& notifyUserId = ""     // Chat riÃªng: ID ngÆ°á»i kia Ä‘á»ƒ gá»­i thÃ´ng bÃ¡o tin nháº¯n (nhÃ³m thÃ¬ Ä‘á»ƒ rá»—ng)
);

// Má»Ÿ cuá»™c trÃ² chuyá»‡n riÃªng vá»›i 1 ngÆ°á»i
void openPrivateChat(const string& myId, const string& otherId);

// ---------- NhÃ³m chat ----------
fs::path groupDir(const string& groupId);

bool readGroup(const string& groupId, Group& group);

vector<string> listGroupIds();

bool isGroupMember(const Group& group, const string& accountId);

string groupLastActivity(const Group& group);

vector<Group> getGroupsOf(const string& accountId);

// ---------- Trang cÃ¡ nhÃ¢n, danh sÃ¡ch ngÆ°á»i dÃ¹ng ----------
string relationLabel(const string& status);

CardItem makePersonItem(const string& myId, const string& id, bool showMutual, const string& extra = "");

void personListScreen(
    const string& myId,
    const string& title,
    const string& emptyMessage,
    const string& selectHint,
    const function<vector<string>()>& idsOf,
    bool showMutual = false
);

void mutualFriendsScreen(const string& myId, const string& otherId);

// Xem trang cÃ¡ nhÃ¢n cá»§a 1 ngÆ°á»i (myId: ID cá»§a tÃ´i). CÃ¡c nÃºt thay Ä‘á»•i theo quan há»‡ hiá»‡n táº¡i
void viewUserProfile(const string& myId, const string& otherId);

// ---------- DÃ¹ng chung cho gá»£i Ã½ káº¿t báº¡n ----------
bool canSuggestTo(const string& myId, const string& id, UserProfile& profile);

#endif // CORE_H

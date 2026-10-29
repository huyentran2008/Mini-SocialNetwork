// Module: Tin nhắn riêng
#pragma once
#include <set>

/*
    TIN NHẮN RIÊNG VÀ NHÓM CHAT
    Data/Conversations/.Private/<ID nhỏ>_<ID lớn>.txt        (mỗi dòng: NGƯỜI GỬI|THỜI GIAN|NỘI DUNG)
    Data/Conversations/.Group/GRP<số>/ info.txt, members.txt, messages.txt
                                                                                        */

struct ChatMessage {
    string sender;      // ID người gửi, hoặc "SYSTEM" cho thông báo của nhóm
    string time;
    string text;
};
// Đọc các tin nhắn trong 1 file hội thoại
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
// Ghi thêm 1 tin nhắn vào cuối file hội thoại
bool appendMessage(const fs::path& file, const string& sender, const string& text,
                   string* timeOut = nullptr) {
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

// Tin nhắn bị thu hồi được đánh dấu ngay trong file hội thoại để cả hai phía cùng thấy.
const string WITHDRAWN_MESSAGE_TEXT = "__MINISOCIAL_WITHDRAWN_MESSAGE__";
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
// Màn hình trò chuyện: tiêu đề cố định - lịch sử tin nhắn (cuộn bằng chuột) - ô nhập - menu hướng dẫn.
// blockReason(): trả về rỗng nếu được phép gửi, ngược lại là lý do không gửi được
void runChat(
    const string& myId,
    const string& title,
    const fs::path& file,
    const function<string()>& blockReason,
    const string& notifyUserId = ""     // Chat riêng: ID người kia để gửi thông báo tin nhắn (nhóm thì để rỗng)
) {
    vector<ChatMessage> messages = readMessages(file);
    string displayTitle = title;
    set<string> hiddenKeys = readHiddenMessageKeys(myId, file);
    map<string, string> names;
    int selectedMessage = 0;
    int selectionMode = 0; // 0: bình thường, 1: xóa phía mình, 2: thu hồi
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
    state.offset = INT_MAX / 2;     // Mở lên là thấy tin nhắn mới nhất

    // Dấu hiệu thay đổi của file hội thoại (kích thước + thời điểm ghi) để biết có tin nhắn mới
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
        // Vẽ giao diện
        [&]() {
            vector<TextBlock> blocks;

            if (messages.empty()) {
                blocks.push_back(makeBlock("Chưa có tin nhắn nào. Hãy bắt đầu cuộc trò chuyện!", 8, 1));
            }

            string lastDate;

            int visibleIndex = 0;
            for (const ChatMessage& message : messages) {
                if (hiddenKeys.count(messageKey(message))) continue;
                string date = (message.time.size() >= 10) ? message.time.substr(0, 10) : "";

                // Dòng ngăn cách khi sang ngày mới
                if (date != lastDate) {
                    lastDate = date;

                    if (!date.empty()) {
                        blocks.push_back(makeBlock(
                            "---------- " + toSlashDate(date) + " ----------", 8, 1));
                    }
                }

                // Thông báo của hệ thống (nhóm)
                if (message.sender == "SYSTEM") {
                    blocks.push_back(makeBlock(message.text, 8, 1));
                    continue;
                }

                bool withdrawn = message.text == WITHDRAWN_MESSAGE_TEXT;
                string shownText = withdrawn ? "Bạn đã thu hồi tin nhắn" : message.text;
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
                    // Hiện cùng dấu chọn ở tin nhắn đối phương để dễ phân biệt mục đang thao tác.
                    blocks.push_back(makeBlock(selector + nameOf(message.sender) + " · " + hhmm, 3, 0));
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
                    ? "CHẾ ĐỘ XÓA: ENTER chọn/bỏ chọn | DELETE lần nữa: Xóa phía bạn | /: Hủy | ESC: Thoát chế độ"
                    : selectionMode == 2
                    ? "CHẾ ĐỘ THU HỒI: ENTER chọn/bỏ chọn | / lần nữa: Thu hồi | DELETE: Hủy | ESC: Thoát chế độ"
                    : "Gõ để soạn tin | ENTER: Gửi | DELETE: Chọn tin để xóa | /: Chọn tin để thu hồi | F2: Đặt biệt danh | Cuộn/▲/▼: Xem tin"
            );
        },
        // Xử lý phím
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
                    errorMessage = "Không thể lưu trạng thái thu hồi tin nhắn.";
                }
                if (!changed && mode == 2 && errorMessage.empty())
                    errorMessage = "Chỉ thu hồi được tin nhắn của bạn trong vòng 10 phút.";
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
                        errorMessage = "Bạn chỉ có thể thu hồi tin nhắn do chính mình gửi.";
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
                nicknameOptions.title = "ĐẶT BIỆT DANH";
                nicknameOptions.prompt = "Nhập biệt danh; để trống để bỏ biệt danh.";
                nicknameOptions.maxLength = 40;
                if (!inputText(nicknameOptions, newName)) return false;
                if (setNickname(myId, notifyUserId, newName)) {
                    names.erase(notifyUserId);
                    displayTitle = "=============== TRÒ CHUYỆN VỚI " + chatDisplayName(myId, notifyUserId) + " ===============";
                    string notice = trimString(newName).empty()
                        ? "Bạn đã bỏ biệt danh của " + getDisplayName(notifyUserId)
                        : "Bạn đã đặt biệt danh cho " + getDisplayName(notifyUserId) + " là \"" + trimString(newName) + "\"";
                    if (appendMessage(file, "SYSTEM", notice)) {
                        ChatMessage systemMessage;
                        systemMessage.sender = "SYSTEM";
                        systemMessage.time = nowTimestamp();
                        systemMessage.text = notice;
                        messages.push_back(systemMessage);
                        lastStamp = fileStamp();
                        state.offset = INT_MAX / 2;
                    }
                    errorMessage = trimString(newName).empty() ? "Đã bỏ biệt danh." : "Đã cập nhật biệt danh.";
                } else errorMessage = "Không lưu được biệt danh.";
                return false;
            }

            errorMessage.clear();

            // Không được phép gửi (bị chặn, không còn là bạn bè...)
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

                // Nhắn lần đầu hoặc nhắn lại sau nhiều ngày thì báo cho người kia (không báo mỗi tin)
                bool notifyOther = !notifyUserId.empty() &&
                                   shouldNotifyMessage(messages.empty() ? "" : messages.back().time);

                if (!appendMessage(file, myId, text, &time)) {
                    errorMessage = "Không gửi được tin nhắn, vui lòng thử lại!";
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
                lastStamp = fileStamp();    // Tin vừa gửi đã nằm trong danh sách, không cần đọc lại

                input.clear();
                state.offset = INT_MAX / 2;     // Cuộn xuống tin mới nhất
            }
            else if (key.type == KEY_CHARACTER) {
                bool leadingSpace = input.empty() && key.character == " ";

                if (!leadingSpace && textLength(input) < 500) {
                    input += key.character;
                }
            }

            return false;
        },
        // Mỗi 0,7 giây không có phím: kiểm tra tin nhắn mới của người kia và tự cập nhật
        700,
        [&]() -> bool {
            bool changed = false;
            auto stamp = fileStamp();

            if (stamp != lastStamp) {
                lastStamp = stamp;

                // Đang xem cuối cuộc trò chuyện thì tự cuộn xuống tin mới, đang xem tin cũ thì giữ nguyên chỗ đọc
                bool atBottom = state.offset >= state.maxOffset;

                messages = readMessages(file);
                hiddenKeys = readHiddenMessageKeys(myId, file);

                if (atBottom) {
                    state.offset = INT_MAX / 2;
                }

                changed = true;
            }

            // Quan hệ thay đổi (bị chặn, bị xóa khỏi nhóm...) thì cập nhật dòng nhập tin
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
// Mở cuộc trò chuyện riêng với 1 người
void openPrivateChat(const string& myId, const string& otherId) {
    string title = "=============== TRÒ CHUYỆN VỚI " + chatDisplayName(myId, otherId) + " ===============";

    runChat(myId, title, privateChatFile(myId, otherId), [&]() -> string {
        if (isBlockedEither(myId, otherId)) {
            return "Bạn không thể nhắn tin cho người này!";
        }

        if (!areFriends(myId, otherId)) {
            return "Hai bạn không còn là bạn bè nên không thể nhắn tin!";
        }

        return "";
    }, otherId);
}


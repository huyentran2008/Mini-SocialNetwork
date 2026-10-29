// Module: Màn hình đọc có cuộn, xác nhận, nhập văn bản
#pragma once

/*
    MÀN HÌNH ĐỌC CÓ CUỘN: TIÊU ĐỀ CỐ ĐỊNH - NỘI DUNG CUỘN - MENU HƯỚNG DẪN CỐ ĐỊNH
    Nội dung được canh giữa (chừa lề trái / phải), cuộn bằng con lăn chuột,
    phím ▲ / ▼, PageUp / PageDown, Home / End
                                                                                    */

// Một khối văn bản trong phần nội dung
struct TextBlock {
    string text;
    int color = 7;
    int align = 0;          // 0: canh trái, 1: canh giữa, 2: canh phải (trong khối nội dung)
    int indent = 0;         // Thụt lề bên trái
    int widthPercent = 100; // Tỉ lệ độ rộng tối đa của khối chữ (dùng cho tin nhắn)
    bool raw = false;       // true: giữ nguyên đúng 1 dòng, không ngắt dòng (người gọi đã tự ngắt / canh lề)
    bool keepSpaces = false; // true: giữ nguyên mọi khoảng trắng (kể cả ở cuối dòng) - dùng cho ô đang soạn thảo
};
TextBlock makeBlock(const string& text, int color = 7, int align = 0,
                    int indent = 0, int widthPercent = 100) {
    TextBlock block;
    block.text = text;
    block.color = color;
    block.align = align;
    block.indent = indent;
    block.widthPercent = widthPercent;
    return block;
}
// Một dòng đã ngắt sẵn để vẽ
struct DisplayLine {
    string text;
    int color;
    int offset;     // Vị trí bắt đầu trong khối nội dung
};
// Dòng hiển thị cố định ngay phía trên menu hướng dẫn (thông báo lỗi, ô nhập tin nhắn...)
struct BottomLine {
    string text;
    int color = 7;
    bool leftAlign = false;     // true: canh trái theo khối nội dung, false: canh giữa
    bool cursor = false;        // true: đặt con trỏ nhập ở cuối dòng này
    bool keepTail = false;      // true: dòng quá dài thì giữ phần đuôi (đang gõ)
};
// Trạng thái cuộn của màn hình đọc
struct ReaderState {
    int offset = 0;             // Dòng đầu tiên đang hiển thị
    int maxOffset = 0;
    int viewHeight = 1;
    bool wantEndCursor = false; // Đặt con trỏ ngay sau chữ cuối cùng (soạn thảo)
    bool endVisible = false;
    int endX = 0;
    int endY = 0;
};
// Ngắt dòng nhưng GIỮ NGUYÊN khoảng trắng (kể cả khoảng trắng ở cuối), dùng cho ô đang soạn thảo.
// wrapText bỏ khoảng trắng cuối dòng nên khi vừa gõ dấu cách, con trỏ nằm sát chữ trước và chữ tiếp theo
// (đặc biệt khi gõ tiếng Việt bằng bộ gõ) bị hiện dính vào chữ trước đó
vector<string> wrapKeepSpaces(const string& text, int width) {
    vector<string> lines;
    size_t start = 0;

    while (true) {
        size_t newline = text.find('\n', start);
        string paragraph = (newline == string::npos) ? text.substr(start)
                                                     : text.substr(start, newline - start);

        string current;
        int currentLen = 0;
        int lastSpace = -1;     // Vị trí (byte) ngay sau dấu cách cuối cùng trong current

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
                    // Ngắt dòng ngay sau dấu cách cuối cùng
                    lines.push_back(current.substr(0, lastSpace));
                    current = current.substr(lastSpace);
                    currentLen = textLength(current);
                    lastSpace = -1;
                }
                else {
                    // Không có chỗ ngắt: cắt cứng, ký tự vừa gõ xuống dòng mới
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
// Ngắt các khối văn bản thành các dòng vừa độ rộng blockWidth
vector<DisplayLine> layoutBlocks(const vector<TextBlock>& blocks, int blockWidth) {
    vector<DisplayLine> lines;

    for (const TextBlock& block : blocks) {
        // Dòng đã được người gọi tự dàn trang
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
// Chia menu hướng dẫn (các mục cách nhau bởi " | ") thành nhiều dòng nếu màn hình hẹp
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
// In chuỗi tại vị trí (x, y) với màu color
void printAt(int x, int y, const string& s, int color) {
    gotoxy(x, y);
    setColor(color);
    writeText(s);
}
// Độ rộng khối nội dung của màn hình đọc theo độ rộng cửa sổ (chừa lề trái / phải)
int computeReaderBlockWidth(int width) {
    int margin = max(2, width / 8);
    int blockWidth = min(100, width - 2 * margin);
    if (blockWidth < 10) blockWidth = max(1, width - 2);
    return blockWidth;
}
// Vẽ 1 khung hình của màn hình đọc: tiêu đề - nội dung (cuộn) - dòng phụ - menu hướng dẫn
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

    // Chừa dòng cuối cùng của màn hình để tránh bị cuộn khi ghi
    int rows = height - 1;

    // Khối nội dung nằm giữa, chừa lề trái / phải
    int blockWidth = computeReaderBlockWidth(width);
    int blockX = left + (width - blockWidth) / 2;

    int maxLen = width - 2;

    vector<DisplayLine> lines = layoutBlocks(blocks, blockWidth);
    int total = (int)lines.size();

    // Chiều cao khu vực nội dung (tính dư chỗ cho phần trăm cuộn ở menu hướng dẫn)
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

    // Tiêu đề (luôn hiển thị)
    printCenter(cutText(title, maxLen), top, 11, width, left);
    printCenter(string(blockWidth, '-'), top + 1, 8, width, left);

    // Nội dung
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

    // Thanh cuộn bên phải khối nội dung
    if (scrollable) {
        int barX = blockX + blockWidth + 1;
        if (barX > left + width - 1) barX = left + width - 1;

        int thumb = max(1, contentHeight * contentHeight / total);
        int thumbPos = (state.maxOffset == 0)
                     ? 0
                     : state.offset * (contentHeight - thumb) / state.maxOffset;

        for (int i = 0; i < contentHeight; i++) {
            bool inThumb = (i >= thumbPos && i < thumbPos + thumb);
            printAt(barX, top + 2 + i, inThumb ? "█" : "│", inThumb ? 11 : 8);
        }
    }

    // Các dòng phụ nằm sát phía trên menu hướng dẫn
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

    // Menu hướng dẫn (luôn hiển thị)
    int separatorY = top + 2 + contentHeight + (int)bottom.size();
    printCenter(string(blockWidth, '-'), separatorY, 8, width, left);

    for (size_t k = 0; k < footerLines.size(); k++) {
        printCenter(footerLines[k], separatorY + 1 + (int)k, 10, width, left);
    }

    // Con trỏ nhập liệu (đặt cuối cùng vì các lệnh in ở trên làm con trỏ di chuyển)
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
// Xử lý phím cuộn. Trả về true nếu phím vừa nhấn là phím cuộn
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
// Màn hình đọc: trả về 0 khi nhấn ESC, ngược lại trả về mã phím hành động
// (chữ HOA nằm trong actionKeys, hoặc '\n' nếu actionKeys có '\n' và nhấn ENTER)
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
        // Vẽ giao diện
        [&]() {
            drawReaderFrame(title, blocks, state, noBottom, footer);
        },
        // Xử lý phím
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
// Hỏi xác nhận bằng menu Không / Có (mặc định chọn "Không"). Trả về true nếu chọn "Có"
bool confirmScreen(const string& question) {
    vector<string> items = {"Không", "Có"};

    int choice = menu(
        cutText(question, getConsoleWidth() - 4),
        items,
        "Sử dụng ▲ / ▼ | ENTER: Chọn | ESC: Hủy"
    );

    return choice == 2;
}

/*
    NHẬP VĂN BẢN (1 DÒNG HOẶC NHIỀU DÒNG)
                                            */

enum InputMode {
    INPUT_FREE = 0,     // Nhận mọi ký tự (kể cả chữ có dấu)
    INPUT_ASCII = 1,    // Chỉ nhận ký tự ASCII không khoảng trắng (mật khẩu)
    INPUT_DATE = 2      // Chỉ nhận chữ số, tự thêm '-' (DD-MM-YYYY)
};
struct InputOptions {
    string title;
    string prompt;                  // Dòng gợi ý phía trên ô nhập
    string footer;                  // Rỗng: dùng menu hướng dẫn mặc định
    int maxLength = 100;
    int mode = INPUT_FREE;
    bool multiline = false;         // true: Shift+ENTER hoặc Ctrl+ENTER để xuống dòng
    bool mask = false;              // true: hiển thị dấu '*'
    function<string(const string&)> validate;   // Trả về thông báo lỗi, rỗng nếu hợp lệ
};
// Màn hình nhập văn bản. Trả về true nếu xác nhận (ENTER), false nếu nhấn ESC.
// value vừa là giá trị ban đầu vừa là kết quả
bool inputText(const InputOptions& options, string& value) {
    string text = value;
    string errorMessage;
    bool accepted = false;

    ReaderState state;
    state.wantEndCursor = true;

    string footer = options.footer;

    if (footer.empty()) {
        footer = options.multiline
               ? "ENTER: Xác nhận | Shift+ENTER: Xuống dòng | Backspace: Xóa | ESC: Hủy"
               : "ENTER: Xác nhận | Backspace: Xóa | ESC: Hủy";
    }

    runLoop(
        // Vẽ giao diện
        [&]() {
            vector<TextBlock> blocks;

            if (!options.prompt.empty()) {
                blocks.push_back(makeBlock(options.prompt, 8, 1));
                blocks.push_back(makeBlock("", 7));
            }

            // Ô đang soạn thảo: giữ nguyên dấu cách để con trỏ luôn đứng đúng chỗ
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

            // Luôn cuộn xuống cuối để thấy chỗ đang gõ
            state.offset = INT_MAX / 2;

            drawReaderFrame(options.title, blocks, state, bottom, footer);
        },
        // Xử lý phím
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

                // Ngày sinh: xóa luôn dấu '-' tự thêm vào
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
                    // Không bắt đầu bằng khoảng trắng
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


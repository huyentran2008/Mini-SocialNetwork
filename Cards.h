// Module: Danh sách thẻ nhiều dòng và nhập chữ từng phím
#pragma once

/*
    DÙNG CHUNG CHO CÁC DANH SÁCH DẠNG "THẺ" (bài đăng, kết quả tìm kiếm bạn bè):
    mỗi thẻ gồm nhiều dòng (dòng đầu / tiêu đề / nội dung), các thẻ cách nhau 1 dòng trống,
    thẻ đang chọn được đổi màu nền. Hiển thị bằng khung của màn hình đọc (có cuộn, thanh cuộn).
                                                                                        */

struct Card {
    string head;                // Dòng 1 (ví dụ: tên - thời gian)
    string title;               // Dòng 2
    vector<string> body;        // Các dòng còn lại (đã được ngắt / rút gọn sẵn)
    int headColor = 14;
    int titleColor = 11;
    int bodyColor = 7;
};

// Xử lý 1 phím khi đang gõ chữ vào text (Backspace xóa, ký tự thường thì thêm vào).
// Trả về true nếu nội dung vừa thay đổi
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
// Dựng các dòng hiển thị cho danh sách thẻ. Mỗi dòng là 1 khối "raw" nên chỉ số khối = chỉ số dòng.
// startLine[i] / endLine[i]: dòng đầu / cuối của thẻ i
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

    // Chữ màu xanh lá / đỏ / vàng (trạng thái) vẫn giữ màu khi thẻ được chọn, các màu khác dùng màu mặc định
    auto keepColor = [](int color, int fallback) -> int {
        return (color == 10 || color == 12 || color == 14) ? (0x10 | color) : fallback;
    };

    // Thẻ đang chọn: tô nền xanh, kẻ kín chiều ngang cho thành 1 dải màu
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

        addLine((isSel ? " ▶ " : "   ") + cutText(card.head, textWidth), card.headColor,
                keepColor(card.headColor, 0x1E), isSel);
        addLine("   " + cutText(card.title, textWidth), card.titleColor,
                keepColor(card.titleColor, 0x1F), isSel);

        for (const string& line : card.body) {
            addLine("   " + cutText(line, textWidth), card.bodyColor,
                    keepColor(card.bodyColor, 0x17), isSel);
        }

        endLine[i] = (int)blocks.size() - 1;

        // Khoảng trống nhỏ giữa các thẻ cho đỡ rối mắt
        addLine("", 7, 7, false);
    }
}
// Cuộn sao cho các dòng [startL, endL] nằm trong vùng nhìn thấy
void ensureLinesVisible(ReaderState& state, int startL, int endL) {
    if (startL < state.offset) {
        state.offset = startL;
    }
    else if (endL >= state.offset + state.viewHeight) {
        state.offset = endL - state.viewHeight + 1;
    }
}
// Dòng thông báo canh giữa (dùng ở cuối danh sách)
TextBlock makeCenteredNotice(const string& text, int color, int blockWidth) {
    TextBlock block;
    block.raw = true;
    block.text = cutText(text, blockWidth);
    block.color = color;
    block.indent = max(0, (blockWidth - textLength(block.text)) / 2);
    return block;
}
// Ô tìm kiếm hiển thị sát phía trên menu hướng dẫn
BottomLine makeSearchLine(const string& query) {
    BottomLine line;
    line.text = "Tìm kiếm: " + query;
    line.color = 14;
    line.leftAlign = true;
    line.cursor = true;
    line.keepTail = true;
    return line;
}

/*
    DANH SÁCH THẺ CÓ PHÂN TRANG VÀ TÌM KIẾM (dùng chung cho bài đăng, bạn bè, lời mời, gợi ý,
    tin nhắn, nhóm chat, thông báo)
        ▲ / ▼ / cuộn chuột / PgUp / PgDn / Home / End : chọn thẻ
        ENTER : mở thẻ đang chọn
        M     : xem thêm 1 trang      A : xem toàn bộ danh sách
        S     : tìm kiếm - gõ tới đâu lọc tới đó (không cần nhấn ENTER), ESC để thoát tìm kiếm
                                                                                        */

struct CardItem {
    string key;             // Định danh (ID tài khoản, ID bài đăng...) để biết đang chọn thẻ nào
    Card card;
    string searchText;      // Chữ dùng để tìm kiếm (không phân biệt hoa thường và dấu)
};
struct CardListConfig {
    string title;
    string emptyMessage;
    string selectHint = "Xem";      // Chữ hiện sau "ENTER:" ở menu hướng dẫn
    string noun = "mục";            // Gọi tên mục: "bài đăng", "người", "thông báo"...
    string moreWord = "mục nữa";    // "Còn N <moreWord>": "bài đăng cũ hơn", "người nữa"...
    int pageSize = 5;               // Số thẻ hiện ban đầu và mỗi lần nhấn M. <= 0: luôn hiện toàn bộ
    bool searchable = true;         // Có cho tìm kiếm bằng phím S không
};

// source(textWidth) trả về toàn bộ thẻ (đã sắp xếp), textWidth là độ rộng chữ tối đa của 1 dòng thẻ.
// onSelect(key) được gọi khi nhấn ENTER; xong thì danh sách được nạp lại
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
            showMessageScreen(config.title, config.emptyMessage, "ENTER / ESC: Quay lại");
            return;
        }

        int total = (int)all.size();

        // Chữ tìm kiếm đã bỏ dấu, tính 1 lần cho mỗi lần nạp danh sách
        vector<string> folded;

        for (const CardItem& item : all) {
            folded.push_back(foldText(item.searchText));
        }

        // Quay lại từ màn hình khác: chọn lại đúng thẻ đó (nếu còn)
        if (!selectedKey.empty() && !searching) {
            for (int i = 0; i < total; i++) {
                if (all[i].key == selectedKey) {
                    shown = max(shown, i + 1);
                }
            }
        }

        shown = max(1, min(shown, total));

        // Các thẻ đang hiển thị (chỉ số trong all): kết quả tìm kiếm hoặc các thẻ đầu danh sách
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

            // Thông báo ở cuối danh sách
            bool hasQuery = searching && !trimString(query).empty();
            bool canExpand = paged && !hasQuery && shown < total;

            string notice;
            int noticeColor = 8;

            if (hasQuery) {
                notice = count > 0 ? "── Tìm thấy " + to_string(count) + " " + config.noun + " ──"
                                   : "Không tìm thấy " + config.noun + " nào phù hợp";
                noticeColor = count > 0 ? 8 : 12;
            }
            else if (canExpand) {
                notice = "── Còn " + to_string(total - shown) + " " + config.moreWord
                       + " - nhấn M để xem thêm, A để xem tất cả ──";
                noticeColor = 14;
            }
            else {
                notice = "── Đã hiển thị tất cả " + to_string(total) + " " + config.noun + " ──";
            }

            blocks.push_back(makeCenteredNotice(notice, noticeColor, blockWidth));
            int lastLine = (int)blocks.size() - 1;

            vector<BottomLine> bottom;
            string footer;

            if (searching) {
                bottom.push_back(makeSearchLine(query));
                footer = "Gõ để tìm | ▲ / ▼ / Cuộn chuột: Chọn | ENTER: " + config.selectHint
                       + " | ESC: Thoát tìm kiếm";
            }
            else {
                footer = "▲ / ▼ / Cuộn chuột: Chọn | ENTER: " + config.selectHint + " | ";
                if (config.searchable) footer += "S: Tìm kiếm | ";
                if (canExpand)         footer += "M: Xem thêm | A: Xem tất cả | ";
                footer += "ESC: Quay lại";
            }

            // Giữ thẻ đang chọn trong vùng nhìn thấy (thẻ cuối thì cuộn tới cả thông báo cuối danh sách)
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

            // Chiều cao khung thật chỉ biết sau khi vẽ: lệch thì chỉnh lại rồi vẽ lại
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
                    case KEY_SCROLL_UP:   selected -= 2;  break;    // Lăn chuột: nhảy 2 thẻ mỗi nấc
                    case KEY_SCROLL_DOWN: selected += 2;  break;
                    case KEY_PAGE_UP:     selected -= pageStep; break;
                    case KEY_PAGE_DOWN:   selected += pageStep; break;
                    case KEY_HOME:        selected = 0;   break;
                    case KEY_END:         selected = count - 1; break;

                    case KEY_ESCAPE:
                        if (searching) {
                            // Thoát tìm kiếm, giữ nguyên thẻ đang chọn (nếu có)
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
                                selected = 0;       // Kết quả vừa đổi: về đầu danh sách
                            }
                        }
                        else if (config.searchable && (key.asciiCode == 's' || key.asciiCode == 'S')) {
                            searching = true;
                            query.clear();
                            selected = 0;
                        }
                        // M: xem thêm 1 trang và nhảy tới thẻ đầu tiên vừa hiện ra
                        else if (paged && (key.asciiCode == 'm' || key.asciiCode == 'M') && shown < total) {
                            int oldShown = shown;
                            shown = min(total, shown + config.pageSize);
                            selected = oldShown;
                        }
                        // A: xem toàn bộ danh sách
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

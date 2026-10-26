// Module: Khung vẽ menu, menu cuộn, menu thông tin
#pragma once

/*
    HÀM IN VÀ HIỂN THỊ MENU
                            */

// Vòng lặp: vẽ -> nhận phím -> xử lý. handleKey trả về true khi muốn dừng vòng lặp
// Nếu có onTick: cứ mỗi tickMs mili giây không có phím thì gọi onTick(), trả về true nếu cần vẽ lại
void runLoop(
    const function<void()>& draw,
    const function<bool(const KeyInput&)>& handleKey,
    int tickMs = 0,
    const function<bool()>& onTick = nullptr
) {
    bool shouldExit = false;
    bool needRedraw = true;

    while (!shouldExit) {
        if (needRedraw) {
            draw();
        }

        KeyInput key = readKey(onTick ? tickMs : -1);

        // Chưa có phím nào: chỉ kiểm tra dữ liệu mới
        if (key.type == KEY_TICK) {
            needRedraw = onTick();
            continue;
        }

        // Phím không nhận diện được thì không cần vẽ lại
        needRedraw = (key.type != KEY_NONE);

        shouldExit = handleKey(key);
    }
}
// Trả về true nếu phím vừa nhấn là mũi tên lên/xuống
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
// In chuỗi s căn giữa tại dòng y với màu sắc color
void printCenter(const string& s, int y, int color,
                 int width, int left) {
    int x = left + (width - textLength(s)) / 2;
    if(x < left) x = left;

    gotoxy(x, y);
    setColor(color);
    writeText(s);
}
// In 1 thông báo căn giữa màn hình (tự vẽ và hiển thị 1 khung hình riêng)
void printCenter(const string& s) {
    beginFrame();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    int width = csbi.srWindow.Right
              - csbi.srWindow.Left + 1;

    int height = csbi.srWindow.Bottom
               - csbi.srWindow.Top + 1;

    // Căn giữa ngang
    int x = csbi.srWindow.Left
          + (width - textLength(s)) / 2;

    // Căn giữa dọc
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
// Vẽ menu với các mục được căn giữa theo chiều ngang và chiều dọc
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
// Vẽ menu có thêm khối thông tin chỉ để xem (nhãn, giá trị) ở phía trên
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

    // Độ rộng lớn nhất của nhãn để căn thẳng cột dấu ':'
    int maxLabel = 0;
    for (const auto& line : info) {
        int len = textLength(line.first);
        if (len > maxLabel) maxLabel = len;
    }

    // Độ rộng của cả khối thông tin
    int blockWidth = 0;
    for (const auto& line : info) {
        int len = maxLabel + 3 + textLength(line.second);
        if (len > blockWidth) blockWidth = len;
    }

    // Tiêu đề + dòng trống + thông tin + đường kẻ + các mục + dòng trống + footer
    int totalLines = 1 + 1 + (int)info.size() + 1
                   + (int)items.size() + 1 + 1;
    int startY = top + (height - totalLines) / 2;
    if (startY < top) startY = top;

    int y = startY;

    // Tiêu đề
    printCenter(title, y++, 11, width, left);
    y++;

    // Khối thông tin: cả khối được căn giữa, các dòng căn trái theo khối
    int infoX = left + (width - blockWidth) / 2;
    if (infoX < left) infoX = left;

    for (const auto& line : info) {
        string label = line.first
                     + string(maxLabel - textLength(line.first), ' ')
                     + " : ";

        gotoxy(infoX, y++);
        setColor(3);            // Nhãn: màu xanh lơ
        writeText(label);
        setColor(15);           // Giá trị: màu trắng sáng
        writeText(line.second);
    }

    // Đường kẻ ngăn cách phần thông tin và phần lựa chọn
    int separatorLength = textLength(title);
    if (blockWidth > separatorLength) separatorLength = blockWidth;

    printCenter(string(separatorLength, '-'), y++, 8, width, left);

    // Các mục chọn được
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
// Hiển thị menu có khối thông tin và nhận lựa chọn
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
        // Vẽ giao diện
        [&]() {
            drawMenuWithInfo(selected, title, info, items, footer);
        },
        // Xử lý phím
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

// Khai báo trước (cutText được định nghĩa ở phần mở rộng phía dưới)
string cutText(const string& s, int maxLen);

// Vẽ menu dài có cuộn: chỉ hiển thị 'visible' mục bắt đầu từ vị trí 'offset'
void drawScrollMenu(
    int selected,
    int offset,
    int visible,
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

    int total = (int)items.size();

    // Tiêu đề + các mục đang hiển thị + dòng trống + footer
    int totalLines = visible + 3;
    int startY = top + (height - totalLines) / 2;
    if (startY < top) startY = top;

    printCenter(title, startY, 11, width, left);

    for (int i = 0; i < visible; i++) {
        int index = offset + i;

        if(selected == index + 1) {
            printCenter("> " + items[index] + " <",
                        startY + 1 + i, 14, width, left);
        } else {
            printCenter(items[index], startY + 1 + i, 7, width, left);
        }
    }

    // Thanh cuộn ở mép phải (chỉ hiện khi có nhiều mục hơn số dòng hiển thị)
    if (total > visible) {
        int barX = left + width - 1;
        int thumb = max(1, visible * visible / total);
        int range = total - visible;
        int thumbPos = (range == 0) ? 0 : offset * (visible - thumb) / range;

        for (int i = 0; i < visible; i++) {
            bool inThumb = (i >= thumbPos && i < thumbPos + thumb);

            gotoxy(barX, startY + 1 + i);
            setColor(inThumb ? 11 : 8);
            writeText(inThumb ? "█" : "│");
        }
    }

    // Footer kèm vị trí đang chọn
    string footerText = footer + " | " + to_string(selected)
                      + "/" + to_string(total);

    if (total > visible) {
        footerText += " | Chuột / PgUp / PgDn: Cuộn";
    }

    printCenter(cutText(footerText, width - 2),
                startY + visible + 2, 10, width, left);

    setColor(7);
    endFrame();
}
// Menu dài có cuộn. Trả về vị trí chọn
// Hỗ trợ: ▲ / ▼, con lăn chuột, PageUp / PageDown, Home / End
// startSelected: vị trí được chọn sẵn khi mở menu
int scrollMenu(
    const string& title,
    const vector<string>& items,
    const string& footer,
    int startSelected = 1
) {
    if(items.empty()) return 0;

    int count = (int)items.size();
    int selected = startSelected;
    if(selected < 1 || selected > count) selected = 1;

    int offset = 0;
    int visible = 1;        // Số dòng đang hiển thị (cập nhật mỗi lần vẽ)
    int result = 0;

    runLoop(
        // Vẽ giao diện
        [&]() {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);

            int height = csbi.srWindow.Bottom
                       - csbi.srWindow.Top + 1;

            // Chừa chỗ cho tiêu đề, dòng trống và footer
            visible = height - 4;
            if(visible > count) visible = count;
            if(visible < 1) visible = 1;

            // Cuộn để mục đang chọn luôn nằm trong vùng hiển thị
            if(selected - 1 < offset) offset = selected - 1;
            if(selected - 1 >= offset + visible) offset = selected - visible;

            // Phòng khi cửa sổ đổi kích thước
            if(offset > count - visible) offset = count - visible;
            if(offset < 0) offset = 0;

            drawScrollMenu(selected, offset, visible, title, items, footer);
        },
        // Xử lý phím
        [&](const KeyInput& key) -> bool {
            // ▲ / ▼ (quay vòng)
            if(moveSelection(selected, count, key.type)) {
                return false;
            }

            // Cuộn nhanh: con lăn chuột, PageUp / PageDown, Home / End (không quay vòng)
            bool moved = true;
            int page = max(1, visible - 1);

            switch (key.type) {
                case KEY_SCROLL_UP:   selected = max(1, selected - 3);         break;
                case KEY_SCROLL_DOWN: selected = min(count, selected + 3);     break;
                case KEY_PAGE_UP:     selected = max(1, selected - page);      break;
                case KEY_PAGE_DOWN:   selected = min(count, selected + page);  break;
                case KEY_HOME:        selected = 1;                            break;
                case KEY_END:         selected = count;                        break;
                default:              moved = false;                           break;
            }

            if(moved) {
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
// Màn hình chỉ có tiêu đề + thông báo, chờ ENTER hoặc ESC để quay lại
void showMessageScreen(
    const string& title,
    const string& message,
    const string& footer
) {
    runLoop(
        // Vẽ giao diện
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

            // 5 dòng: tiêu đề, trống, thông báo, trống, footer
            int startY = top + (height - 5) / 2;
            if (startY < top) startY = top;

            printCenter(title, startY, 11, width, left);
            printCenter(message, startY + 2, 14, width, left);
            printCenter(footer, startY + 4, 10, width, left);

            setColor(7);
            endFrame();
        },
        // Xử lý phím
        [&](const KeyInput& key) -> bool {
            return key.type == KEY_ENTER || key.type == KEY_ESCAPE;
        }
    );
}
// Hiển thị menu và nhận lựa chọn từ người dùng
int menu(
    const string& title,
    const vector<string>& items,
    const string& footer
) {
    if(items.empty()) return 0;

    int selected = 1;
    int result = 0;

    runLoop(
        // Vẽ giao diện
        [&]() {
            drawMenu(selected, title, items, footer);
        },
        // Xử lý phím
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

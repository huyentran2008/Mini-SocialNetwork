// Module: Chọn 1 hoặc nhiều mục từ danh sách có tìm kiếm trực tiếp
#pragma once

/*
    BỘ CHỌN TỪ DANH SÁCH (tỉnh thành, trường học, sở thích)
        ▲ / ▼ / cuộn chuột / PgUp / PgDn / Home / End : di chuyển
        ENTER : chọn (sở thích: chọn / bỏ chọn)
        S     : chuyển sang tìm kiếm - gõ tới đâu danh sách lọc tới đó (không cần nhấn ENTER),
                không phân biệt hoa thường và dấu (gõ "ha" ra "Hà Nội", "Hà Giang"...)
        ESC   : thoát tìm kiếm; ở danh sách thì hủy (chọn 1) hoặc xong (chọn nhiều)
                                                                                        */

// Các mục khớp từ khóa (không phân biệt hoa thường và dấu). Mục bắt đầu bằng từ khóa xếp trước
vector<int> filterOptions(const vector<PickOption>& options, const string& query) {
    string key = foldText(trimString(query));
    vector<int> starts, contains;

    for (int i = 0; i < (int)options.size(); i++) {
        // Mục "(Không chọn)" (tên rỗng) chỉ hiện khi chưa gõ gì
        if (options[i].name.empty()) {
            if (key.empty()) starts.push_back(i);
            continue;
        }

        if (key.empty()) {
            starts.push_back(i);
            continue;
        }

        string name = foldText(options[i].name);

        if (name.compare(0, key.size(), key) == 0) {
            starts.push_back(i);
        }
        else if (foldText(options[i].name + " " + options[i].extra + " " + options[i].keywords)
                     .find(key) != string::npos) {
            contains.push_back(i);
        }
    }

    starts.insert(starts.end(), contains.begin(), contains.end());
    return starts;
}
// Lõi của bộ chọn. multi = false: chọn 1 (kết quả nằm ở chosen[0], rỗng là "không chọn");
// multi = true: chọn tối đa maxPick mục. Trả về false nếu hủy (chỉ có ở chế độ chọn 1)
bool runOptionPicker(
    const string& title,
    const vector<PickOption>& sourceOptions,
    bool multi,
    int maxPick,
    vector<string>& chosen
) {
    // Chế độ chọn 1 có thêm mục đầu tiên "(Không chọn)" để xóa giá trị
    vector<PickOption> options;

    if (!multi) {
        options.push_back({"", "", ""});
    }

    options.insert(options.end(), sourceOptions.begin(), sourceOptions.end());

    bool searching = false;
    string query;
    int selected = 0;
    string message;
    bool accepted = false;
    ReaderState state;
    state.viewHeight = 20;

    auto visibleList = [&]() -> vector<int> {
        return filterOptions(options, searching ? query : "");
    };

    auto isChosen = [&](const string& name) -> bool {
        return find(chosen.begin(), chosen.end(), name) != chosen.end();
    };

    // Chọn 1: mở lên là đứng sẵn ở giá trị hiện tại
    if (!multi && !chosen.empty()) {
        for (int i = 0; i < (int)options.size(); i++) {
            if (options[i].name == chosen[0]) {
                selected = i;
            }
        }
    }

    auto draw = [&]() {
        vector<int> list = visibleList();
        int count = (int)list.size();

        if (selected >= count) selected = count - 1;
        if (selected < 0) selected = 0;

        int blockWidth = computeReaderBlockWidth(getConsoleWidth());
        vector<TextBlock> blocks;

        if (list.empty()) {
            blocks.push_back(makeBlock("Không tìm thấy kết quả phù hợp", 12));
        }

        for (int k = 0; k < count; k++) {
            const PickOption& option = options[list[k]];
            bool isSel = (k == selected);

            string label = option.name.empty() ? "(Không chọn)" : option.name;
            if (!option.extra.empty()) label += "  (" + option.extra + ")";

            string mark;
            if (multi)                    mark = isChosen(option.name) ? "[x] " : "[ ] ";
            else if (!option.name.empty() && isChosen(option.name)) mark = "✓ ";
            else                          mark = "  ";

            string line = (isSel ? " ▶ " : "   ") + mark + cutText(label, max(5, blockWidth - 8));

            TextBlock block;
            block.raw = true;

            if (isSel) {
                int len = textLength(line);
                if (len < blockWidth) line += string(blockWidth - len, ' ');
                block.text = cutChars(line, blockWidth);
                block.color = 0x1F;
            }
            else {
                block.text = line;
                block.color = isChosen(option.name) ? 10 : 7;
            }

            blocks.push_back(block);
        }

        // Dòng phụ phía dưới: số mục đã chọn, thông báo, ô tìm kiếm
        vector<BottomLine> bottom;

        if (multi) {
            string names;

            for (const string& name : chosen) {
                if (!names.empty()) names += ", ";
                names += name;
            }

            BottomLine info;
            info.text = "Đã chọn " + to_string(chosen.size()) + "/" + to_string(maxPick)
                      + (names.empty() ? "" : ": " + names);
            info.color = 10;
            info.leftAlign = true;
            bottom.push_back(info);
        }

        if (!message.empty()) {
            BottomLine error;
            error.text = message;
            error.color = 12;
            error.leftAlign = true;
            bottom.push_back(error);
        }

        if (searching) {
            bottom.push_back(makeSearchLine(query));
        }

        string footer = "▲ / ▼ / Cuộn chuột: Di chuyển | ";

        if (searching) {
            footer += multi ? "Gõ để tìm | ENTER: Chọn / bỏ chọn | ESC: Thoát tìm kiếm"
                            : "Gõ để tìm | ENTER: Chọn | ESC: Thoát tìm kiếm";
        }
        else if (multi) {
            footer += "ENTER: Chọn / bỏ chọn | S: Tìm kiếm | ESC: Xong";
        }
        else {
            footer += "ENTER: Chọn | S: Tìm kiếm | ESC: Hủy";
        }

        int lineIndex = (count == 0) ? 0 : selected;

        ensureLinesVisible(state, lineIndex, lineIndex);
        drawReaderFrame(title, blocks, state, bottom, footer);

        // Chiều cao khung thật chỉ biết sau khi vẽ: lệch thì chỉnh lại rồi vẽ lại
        int oldOffset = state.offset;
        ensureLinesVisible(state, lineIndex, lineIndex);

        if (state.offset != oldOffset) {
            drawReaderFrame(title, blocks, state, bottom, footer);
        }
    };

    auto handleKey = [&](const KeyInput& key) -> bool {
        vector<int> list = visibleList();
        int count = (int)list.size();

        switch (key.type) {
            case KEY_UP:          selected -= 1;  message.clear(); break;
            case KEY_DOWN:        selected += 1;  message.clear(); break;
            case KEY_SCROLL_UP:   selected -= 3;  break;
            case KEY_SCROLL_DOWN: selected += 3;  break;
            case KEY_PAGE_UP:     selected -= max(1, state.viewHeight - 1); break;
            case KEY_PAGE_DOWN:   selected += max(1, state.viewHeight - 1); break;
            case KEY_HOME:        selected = 0;   break;
            case KEY_END:         selected = count - 1; break;

            case KEY_ESCAPE:
                if (searching) {
                    // Thoát tìm kiếm: giữ nguyên mục đang chọn trong danh sách đầy đủ
                    string current = (selected >= 0 && selected < count) ? options[list[selected]].name : "";
                    searching = false;
                    query.clear();
                    message.clear();

                    selected = 0;
                    vector<int> all = visibleList();

                    for (int k = 0; k < (int)all.size(); k++) {
                        if (options[all[k]].name == current) {
                            selected = k;
                        }
                    }

                    return false;
                }

                accepted = multi;       // Chọn nhiều: ESC = xong. Chọn 1: ESC = hủy
                return true;

            case KEY_ENTER:
                if (count == 0) {
                    return false;
                }

                {
                    const string& name = options[list[selected]].name;

                    if (!multi) {
                        chosen.clear();
                        if (!name.empty()) chosen.push_back(name);
                        accepted = true;
                        return true;
                    }

                    auto it = find(chosen.begin(), chosen.end(), name);

                    if (it != chosen.end()) {
                        chosen.erase(it);
                        message.clear();
                    }
                    else if ((int)chosen.size() >= maxPick) {
                        message = "Chỉ được chọn tối đa " + to_string(maxPick) + " mục, hãy bỏ bớt 1 mục trước!";
                    }
                    else {
                        chosen.push_back(name);
                        message.clear();
                    }
                }

                return false;

            case KEY_CHARACTER:
                if (searching) {
                    if (editTextKey(key, query, 40)) {
                        selected = 0;   // Danh sách vừa lọc lại: về đầu danh sách
                    }
                }
                else if (key.asciiCode == 's' || key.asciiCode == 'S') {
                    searching = true;
                    query.clear();
                    message.clear();
                }

                return false;

            case KEY_BACKSPACE:
                if (searching && editTextKey(key, query, 40)) {
                    selected = 0;
                }

                return false;

            default:
                return false;
        }

        if (selected >= count) selected = count - 1;
        if (selected < 0) selected = 0;

        return false;
    };

    runLoop(draw, handleKey);
    hideCursor();
    setColor(7);

    return accepted;
}
// Chọn 1 mục từ danh sách. value là giá trị hiện tại (rỗng = chưa chọn). Trả về false nếu hủy
bool pickOne(const string& title, const vector<PickOption>& options, string& value) {
    vector<string> chosen;

    if (!value.empty()) {
        chosen.push_back(value);
    }

    if (!runOptionPicker(title, options, false, 1, chosen)) {
        return false;
    }

    value = chosen.empty() ? "" : chosen[0];
    return true;
}
// Chọn nhiều mục (tối đa maxPick) từ danh sách
void pickMany(const string& title, const vector<PickOption>& options,
              vector<string>& values, int maxPick) {
    runOptionPicker(title, options, true, maxPick, values);
}
// Đổi giá trị sang mục kế tiếp / trước đó theo vòng: (trống) -> mục 1 -> ... -> mục cuối -> (trống)
string cycleOption(const vector<PickOption>& options, const string& value, int step) {
    int slots = (int)options.size() + 1;    // Ô 0 là "trống"
    int index = 0;

    for (int i = 0; i < (int)options.size(); i++) {
        if (options[i].name == value) {
            index = i + 1;
        }
    }

    index = ((index + step) % slots + slots) % slots;
    return index == 0 ? "" : options[index - 1].name;
}

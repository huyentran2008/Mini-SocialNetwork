#include "search_service.h"
#include "sort_service.h"

void searchUsersScreen(const string& myId) {
    const string title = "=============== TÃŒM KIáº¾M NGÆ¯á»œI DÃ™NG ===============";
    const int MAX_RESULTS = 20;

    string query;
    int selected = 0;
    ReaderState state;
    state.viewHeight = 20;

    vector<UserProfile> results;
    int totalMatches = 0;
    bool showFilters = false;
    int filterCursor = 0;
    vector<bool> enabledFilters(5, false);
    const vector<string> filterNames = {"QuÃª quÃ¡n", "NÆ¡i á»Ÿ", "TrÆ°á»ng há»c", "NÆ¡i lÃ m viá»‡c", "Sá»Ÿ thÃ­ch"};

    auto matchesFilters = [&](const UserProfile& p) {
        UserProfile me;
        if (!findProfileById(myId, me)) return true;
        auto same = [&](const string& a, const string& b) {
            return !foldText(trimString(a)).empty() &&
                   foldText(trimString(a)) == foldText(trimString(b));
        };
        if (enabledFilters[0] && !same(me.hometown, p.hometown)) return false;
        if (enabledFilters[1] && !same(me.residence, p.residence)) return false;
        if (enabledFilters[2] && !same(me.school, p.school)) return false;
        if (enabledFilters[3] && !same(me.workplace, p.workplace)) return false;
        if (enabledFilters[4]) {
            vector<string> mine = splitList(me.hobbies), theirs = splitList(p.hobbies);
            bool common = false;
            for (const string& a : mine) for (const string& b : theirs)
                if (foldText(trimString(a)) == foldText(trimString(b))) common = true;
            if (!common) return false;
        }
        return true;
    };
    auto anyFilter = [&]() {
        return find(enabledFilters.begin(), enabledFilters.end(), true) != enabledFilters.end();
    };

    auto refresh = [&]() {
        results.clear();
        totalMatches = 0;

        string key = foldText(trimString(query));

        if (key.empty() && !anyFilter()) {
            return;
        }

        vector<UserProfile> matched;

        for (const string& id : listAccountIds()) {
            if (id == myId) {
                continue;
            }

            UserProfile profile;

            if (!findProfileById(id, profile)) {
                continue;
            }

            if (getRelation(id, myId) == "BLOCK" || profile.status == "LOCKED") {
                continue;
            }

            bool textMatch = key.empty() ||
                foldText(profile.fullName).find(key) != string::npos ||
                foldText(profile.username).find(key) != string::npos ||
                foldText(profile.id).find(key) != string::npos;
            if (textMatch && matchesFilters(profile)) matched.push_back(profile);
        }

        sortUsersByNameMatch(matched, key);

        totalMatches = (int)matched.size();

        if ((int)matched.size() > MAX_RESULTS) {
            matched.resize(MAX_RESULTS);
        }

        results = matched;
    };

    auto drawAll = [&]() {
        int count = (int)results.size();

        if (selected >= count) selected = count - 1;
        if (selected < 0) selected = 0;

        int blockWidth = computeReaderBlockWidth(getConsoleWidth());

        vector<Card> cards;

        for (const UserProfile& profile : results) {
            Card card;
            card.head = (profile.fullName.empty() ? profile.username : profile.fullName)
                      + " (@" + profile.username + ")";

            string status = getRelation(myId, profile.id);

            if (status == "FRIEND")                 card.head += " [Báº¡n bÃ¨]";
            else if (status == "PENDING_SENT")      card.head += " [ÄÃ£ gá»­i lá»i má»i]";
            else if (status == "PENDING_RECEIVED")  card.head += " [Chá» báº¡n cháº¥p nháº­n]";
            else if (status == "BLOCK")             card.head += " [ÄÃ£ cháº·n]";

            card.title = profile.bio.empty() ? "(chÆ°a cÃ³ giá»›i thiá»‡u)" : oneLine(profile.bio);
            card.titleColor = profile.bio.empty() ? 8 : 7;
            card.body = {to_string(countFriends(profile.id)) + " báº¡n bÃ¨"};
            card.bodyColor = 3;
            cards.push_back(card);
        }

        vector<TextBlock> blocks;
        if (showFilters) {
            blocks.push_back(makeBlock("â”€â”€ Bá»˜ Lá»ŒC (TAB Ä‘Ã³ng/má»Ÿ, ENTER báº­t/táº¯t) â”€â”€", 14, 1));
            for (int i = 0; i < (int)filterNames.size(); ++i) {
                string line = string(enabledFilters[i] ? "[x] " : "[ ] ") + filterNames[i];
                blocks.push_back(makeBlock(line, i == filterCursor ? 14 : 7, 1));
            }
        }
        vector<int> startLine, endLine;
        buildCardBlocks(cards, selected, blockWidth, blocks, startLine, endLine);

        string notice;
        int noticeColor = 8;

        if (trimString(query).empty() && !anyFilter()) {
            notice = "GÃµ tÃªn/tÃ i khoáº£n hoáº·c nháº¥n TAB Ä‘á»ƒ má»Ÿ bá»™ lá»c";
        }
        else if (trimString(query).empty() && anyFilter() && count == 0) {
            notice = "KhÃ´ng cÃ³ ngÆ°á»i dÃ¹ng khá»›p vá»›i cÃ¡c bá»™ lá»c Ä‘Ã£ chá»n";
            noticeColor = 12;
        }
        else if (count == 0) {
            notice = "KhÃ´ng tÃ¬m tháº¥y ngÆ°á»i dÃ¹ng nÃ o phÃ¹ há»£p";
            noticeColor = 12;
        }
        else if (totalMatches > count) {
            notice = "â”€â”€ Hiá»ƒn thá»‹ " + to_string(count) + "/" + to_string(totalMatches)
                   + " káº¿t quáº£, hÃ£y gÃµ thÃªm Ä‘á»ƒ thu háº¹p â”€â”€";
            noticeColor = 14;
        }
        else {
            notice = "â”€â”€ TÃ¬m tháº¥y " + to_string(totalMatches) + " ngÆ°á»i dÃ¹ng â”€â”€";
        }

        blocks.push_back(makeCenteredNotice(notice, noticeColor, blockWidth));
        int lastLine = (int)blocks.size() - 1;

        vector<BottomLine> bottom = {makeSearchLine(query)};
        string footer = "GÃµ Ä‘á»ƒ tÃ¬m | TAB: Bá»™ lá»c | â–² / â–¼ / Cuá»™n chuá»™t: Chá»n | ENTER: Xem há»“ sÆ¡ / báº­t lá»c | ESC: Quay láº¡i";

        auto ensureVisible = [&]() {
            if (count == 0) {
                state.offset = 0;
                return;
            }

            int e = (selected == count - 1) ? lastLine : endLine[selected];
            ensureLinesVisible(state, startLine[selected], e);
        };

        ensureVisible();
        drawReaderFrame(title, blocks, state, bottom, footer);

        int oldOffset = state.offset;
        ensureVisible();

        if (state.offset != oldOffset) {
            drawReaderFrame(title, blocks, state, bottom, footer);
        }
    };

    runLoop(
        drawAll,
        [&](const KeyInput& key) -> bool {
            int count = (int)results.size();

            if (key.type == KEY_TAB) {
                showFilters = !showFilters;
                return false;
            }

            if (showFilters) {
                if (key.type == KEY_UP || key.type == KEY_SCROLL_UP) {
                    filterCursor = (filterCursor + (int)filterNames.size() - 1) % (int)filterNames.size();
                    return false;
                }
                if (key.type == KEY_DOWN || key.type == KEY_SCROLL_DOWN) {
                    filterCursor = (filterCursor + 1) % (int)filterNames.size();
                    return false;
                }
                if (key.type == KEY_ENTER || (key.type == KEY_CHARACTER && key.character == " ")) {
                    enabledFilters[filterCursor] = !enabledFilters[filterCursor];
                    selected = 0;
                    refresh();
                    return false;
                }
                if (key.type == KEY_ESCAPE) {
                    showFilters = false;
                    return false;
                }
            }

            switch (key.type) {
                case KEY_UP:          selected -= 1;  break;
                case KEY_DOWN:        selected += 1;  break;
                case KEY_SCROLL_UP:   selected -= 2;  break;
                case KEY_SCROLL_DOWN: selected += 2;  break;
                case KEY_PAGE_UP:     selected -= 5;  break;
                case KEY_PAGE_DOWN:   selected += 5;  break;
                case KEY_HOME:        selected = 0;   break;
                case KEY_END:         selected = count - 1; break;

                case KEY_ESCAPE:
                    return true;

                case KEY_ENTER:
                    if (count > 0) {
                        viewUserProfile(myId, results[max(0, min(selected, count - 1))].id);
                        refresh();
                    }

                    return false;

                case KEY_BACKSPACE:
                case KEY_CHARACTER:
                    if (editTextKey(key, query, 50)) {
                        selected = 0;
                        refresh();
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

    hideCursor();
    setColor(7);
}

#include "app.h"
#include "bfs_algorithm.h"
#include "friend_suggester.h"
#include "search_service.h"

int runApp(int argc, char* argv[]) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    error_code ec;
    if (!fs::is_directory("Data/Account", ec)) {
        cerr << "KhÃ´ng tÃ¬m tháº¥y thÆ° má»¥c Data/Account. HÃ£y cháº¡y file .exe trong thÆ° má»¥c project (nÆ¡i cÃ³ Data/).\n";
        return 1;
    }

    string myId = (argc > 1) ? argv[1] : "";

    // Cháº¿ Ä‘á»™ in vÄƒn báº£n: kiá»ƒm tra BFS (khoáº£ng cÃ¡ch) vÃ  báº¡n chung mÃ  khÃ´ng cáº§n giao diá»‡n console
    if (argc > 2 && string(argv[2]) == "--bfs") {
        UserProfile me;
        if (!findProfileById(myId, me)) {
            cerr << "KhÃ´ng tÃ¬m tháº¥y tÃ i khoáº£n: " << myId << '\n';
            return 1;
        }

        cout << "Tai khoan: " << myId << " | ban be: " << countFriends(myId) << '\n';
        cout << "--- suggestByFriendGraph (BFS, toi da " << SUGGEST_MAX_DEPTH << " cap) ---\n";

        for (const Suggestion& s : suggestByFriendGraph(myId)) {
            cout << s.id << " | khoang cach=" << s.distance << " | ban chung=" << s.mutual
                 << " | so ban=" << s.friendCount << " | " << s.name << '\n';
        }

        cout << "--- computeSuggestions ---\n";
        for (const Suggestion& s : computeSuggestions(myId)) {
            cout << s.id << " | " << suggestionLabel(s) << '\n';
        }

        cout << "--- suggestByCommonGroup ---\n";
        for (const Suggestion& s : suggestByCommonGroup(myId)) {
            cout << s.id << " | nhom chung=" << s.commonGroups << " | " << s.name << '\n';
        }
        return 0;
    }
    enableMouseInput();
    initDoubleBuffer();
    hideCursor();

    UserProfile me;
    if (myId.empty() || !findProfileById(myId, me)) {
        InputOptions options;
        options.title = "=============== TEST: CHá»ŒN TÃ€I KHOáº¢N ===============";
        options.prompt = "Nháº­p ID tÃ i khoáº£n (vÃ­ dá»¥: ACC000001)";
        options.maxLength = 20;
        options.validate = [](const string& text) -> string {
            UserProfile p;
            return findProfileById(text, p) ? "" : "KhÃ´ng tÃ¬m tháº¥y tÃ i khoáº£n nÃ y!";
        };

        myId = "";
        if (!inputText(options, myId)) {
            cleanupDoubleBuffer();
            restoreInputMode();
            return 0;
        }
    }

    const vector<string> items = {
        "Gá»£i Ã Káº¿t Báº¡n (BFS / Báº¡n Chung / NhÃ³m Chat)",
        "TÃ¬m Kiáº¿m NgÆ°á»i DÃ¹ng",
        "Xem Há»“ SÆ¡ NgÆ°á»i DÃ¹ng Theo ID",
        "ThoÃ¡t"
    };

    while (true) {
        int choice = menu(
            "=============== TEST: " + getDisplayName(myId) + " (" + myId + ") ===============",
            items,
            "Sá»­ dá»¥ng â–² / â–¼ | ENTER: Chá»n | ESC: ThoÃ¡t"
        );

        if (choice == 1) {
            suggestionScreen(myId);
        }
        else if (choice == 2) {
            searchUsersScreen(myId);
        }
        else if (choice == 3) {
            InputOptions options;
            options.title = "=============== XEM Há»’ SÆ  NGÆ¯á»œI DÃ™NG ===============";
            options.prompt = "Nháº­p ID ngÆ°á»i cáº§n xem (vÃ­ dá»¥: ACC000002)";
            options.maxLength = 20;
            options.validate = [](const string& text) -> string {
                UserProfile p;
                return findProfileById(text, p) ? "" : "KhÃ´ng tÃ¬m tháº¥y tÃ i khoáº£n nÃ y!";
            };

            string otherId;
            if (inputText(options, otherId)) {
                viewUserProfile(myId, otherId);
            }
        }
        else {
            break;
        }
    }
    cleanupDoubleBuffer();
    restoreInputMode();
    return 0;
}

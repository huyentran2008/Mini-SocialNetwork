// APP: menu console - cai dat
#include "app.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdlib>

using namespace std;

void App::printBrief(int id) const {
    const User* u = net.getUser(id);
    if (!u) return;
    cout << "  [#" << u->id << "] " << u->name << "  (" << u->age << " tuoi, " << u->friends.size() << " ban)";
    if (net.areFriends(me, id)) cout << "  <ban be>";
    cout << "\n";
    cout << "        " << left << setw(36) << ("Noi o: " + orDash(u->city))
         << "Que quan: " << orDash(u->hometown) << "\n";
    cout << "        " << left << setw(36) << ("Truong: " + orDash(u->school))
         << "Noi lam viec: " << orDash(u->workplace) << "\n";
    cout << "        So thich: " << orDash(TextUtil::join(u->hobbies, ", ")) << "\n\n";
}

void App::printUserList(const vector<int>& ids) const {
    if (ids.empty()) { cout << "  (Khong co ket qua)\n"; return; }
    cout << "  Tim thay " << ids.size() << " nguoi:\n";
    for (size_t i = 0; i < ids.size(); ++i) printBrief(ids[i]);
}

void App::printProfile(int id) const {
    const User* u = net.getUser(id);
    if (!u) return;
    cout << "  +-- [#" << u->id << "] " << u->name << "  (@" << u->username << ")\n";
    cout << "  | Tuoi: " << u->age << "\n";
    cout << "  | Noi o: " << u->city << " | Que quan: " << u->hometown << "\n";
    cout << "  | Truong: " << u->school << " | Noi lam viec: " << u->workplace << "\n";
    cout << "  | So thich: " << TextUtil::join(u->hobbies, ", ") << "\n";
    cout << "  +-- So ban: " << u->friends.size() << "\n";
}

void App::printSuggestions(const vector<Suggestion>& v) const {
    if (v.empty()) { cout << "  (Khong co goi y nao)\n"; return; }
    cout << "  Tong cong " << v.size() << " goi y:\n\n";
    for (size_t i = 0; i < v.size(); ++i) {
        const User* u = net.getUser(v[i].userId);
        ostringstream who;
        who << "[#" << u->id << "] " << u->name;
        cout << "  " << right << setw(2) << (i + 1) << ". " << left << setw(36) << who.str()
             << "Diem: " << v[i].score << "\n";
        for (size_t r = 0; r < v[i].reasons.size(); ++r) cout << "        - " << v[i].reasons[r] << "\n";
        cout << "\n";
    }
}

void App::showSorted(vector<int> ids) const {
    if (ids.empty()) { cout << "  (Khong co ket qua)\n"; return; }
    cout << "  Sap xep ket qua: 0 = giu nguyen, 1 = theo ten, 2 = theo so ban (giam dan)\n";
    int c = readInt("  Chon: ", 0, 2);
    if (c == 1) ids = sorter.sortUsersByName(ids);
    else if (c == 2) ids = sorter.sortUsersByFriendCount(ids);
    printUserList(ids);
}

int App::pickUser(const string& label) const {
    while (true) {
        string s = readLine("  " + label + " (nhap ID hoac ten, 0 = huy): ");
        if (s.empty()) { cout << "  ! Khong duoc de trong.\n"; continue; }
        if (s[0] == '-' && TextUtil::isAllDigits(s.substr(1))) { cout << "  ! Khong duoc nhap so am.\n"; continue; }
        if (TextUtil::isAllDigits(s)) {
            if (s.size() > 9) { cout << "  ! So qua lon.\n"; continue; }
            int id = atoi(s.c_str());
            if (id == 0) return -1;
            if (net.exists(id)) return id;
            cout << "  ! Khong co nguoi dung voi ID " << id << ".\n";
            continue;
        }
        string t = TextUtil::collapseSpaces(s);
        int c = TextUtil::checkText(t);
        if (c == 1) { cout << "  ! Ten khong duoc chua so (neu muon nhap ID thi chi nhap so).\n"; continue; }
        if (c == 2) { cout << "  ! Chi nhap chu cai khong dau (a-z) va khoang trang.\n"; continue; }
        vector<int> found = search.searchByName(net.allIds(), t);
        if (found.empty()) { cout << "  ! Khong co ai co ten chua \"" << t << "\".\n"; continue; }
        if (found.size() == 1) {
            cout << "  => Chon:";
            printBrief(found[0]);
            return found[0];
        }
        cout << "  Co " << found.size() << " nguoi khop ten, hay chon ID:\n";
        for (size_t i = 0; i < found.size(); ++i) printBrief(found[i]);
        while (true) {
            int id = readInt("  Nhap ID trong danh sach tren (0 = nhap lai ten): ", 0, 999999999);
            if (id == 0) break;
            if (find(found.begin(), found.end(), id) != found.end()) return id;
            cout << "  ! ID " << id << " khong nam trong danh sach tren.\n";
        }
    }
}

int App::pickOther(const string& label) const {
    while (true) {
        int id = pickUser(label);
        if (id < 0) return -1;
        if (id != me) return id;
        cout << "  ! Day la tai khoan cua ban, hay chon nguoi khac.\n";
    }
}

int App::pickMyGroup() const {
    vector<const Group*> mine;
    const vector<Group>& gs = net.allGroups();
    for (size_t i = 0; i < gs.size(); ++i) if (gs[i].members.count(me)) mine.push_back(&gs[i]);
    if (mine.empty()) { cout << "  (Ban chua tham gia nhom nao)\n"; return -1; }
    for (size_t i = 0; i < mine.size(); ++i)
        cout << "  [" << mine[i]->id << "] " << mine[i]->name << " (" << mine[i]->members.size() << " thanh vien)\n";
    while (true) {
        int id = readInt("  Chon ID nhom (0 = huy): ", 0, 999999999);
        if (id == 0) return -1;
        const Group* g = net.getGroup(id);
        if (g && g->members.count(me)) return id;
        cout << "  ! Ban khong o trong nhom nay.\n";
    }
}

Attr App::pickAttr() const {
    cout << "  1. Noi o   2. Que quan   3. Noi lam viec   4. Truong   5. So thich\n";
    int c = readInt("  Chon tieu chi: ", 1, 5);
    static const Attr map[] = {A_CITY, A_HOMETOWN, A_WORKPLACE, A_SCHOOL, A_HOBBY};
    return map[c - 1];
}

bool App::pickScope(vector<int>& base) const {
    cout << "  Pham vi: 1. Toan he thong   2. Ban be cua toi   3. Trong nhom cua toi   0. Huy\n";
    int c = readInt("  Chon: ", 0, 3);
    if (c == 0) return false;
    if (c == 1) base = net.allIds();
    else if (c == 2) base = search.friendsOf(me);
    else {
        int g = pickMyGroup();
        if (g < 0) return false;
        base = search.membersOf(g);
    }
    base.erase(remove(base.begin(), base.end(), me), base.end());
    return true;
}

void App::attrSearch(Attr a) {
    vector<int> base;
    if (!pickScope(base)) return;
    string v = readText(string("  Nhap ") + attrLabel(a) + " (nhap mot phan cung duoc): ");
    showSorted(search.searchByAttribute(a, v, &base));
}

void App::searchMenu() {
    while (true) {
        cout << "\n===== TIM KIEM =====\n"
             << " 1. Theo ID\n"
             << " 2. Theo username\n"
             << " 3. Theo ten\n"
             << " 4. Theo tieu chi chung\n"
             << " 5. Theo noi o\n"
             << " 6. Theo que quan\n"
             << " 7. Theo noi lam viec\n"
             << " 8. Theo truong\n"
             << " 9. Theo so thich\n"
             << "10. Ban chung giua toi va 1 nguoi khac\n"
             << " 0. Quay lai\n";
        int c = readInt("Chon: ", 0, 10);
        if (c == 0) return;
        switch (c) {
            case 1: {
                int id = readInt("Nhap ID: ", 0, 999999999);
                const User* u = search.searchById(id);
                if (u) printProfile(u->id); else cout << "  (Khong tim thay)\n";
                break;
            }
            case 2: {
                string un = readLine("Nhap username (vd: nguyen.van.an): ");
                const User* u = search.searchByUsername(un);
                if (u) printProfile(u->id); else cout << "  (Khong tim thay)\n";
                break;
            }
            case 3: {
                vector<int> base;
                if (!pickScope(base)) break;
                string kw = readText("  Nhap tu khoa ten: ");
                showSorted(search.searchByName(base, kw));
                break;
            }
            case 4: {
                vector<int> base;
                if (!pickScope(base)) break;
                cout << "  (Enter de bo qua dieu kien)\n";
                FilterCriteria fc;
                fc.city = readText("  Noi o: ", true);
                fc.hometown = readText("  Que quan: ", true);
                fc.school = readText("  Truong: ", true);
                fc.workplace = readText("  Noi lam viec: ", true);
                fc.hobby = readText("  So thich: ", true);
                if (fc.empty()) { cout << "  ! Chua co dieu kien nao.\n"; break; }
                showSorted(search.filterUsers(base, fc));
                break;
            }
            case 5: attrSearch(A_CITY); break;
            case 6: attrSearch(A_HOMETOWN); break;
            case 7: attrSearch(A_WORKPLACE); break;
            case 8: attrSearch(A_SCHOOL); break;
            case 9: attrSearch(A_HOBBY); break;
            case 10: {
                int b = pickOther("Nguoi can xem ban chung voi toi");
                if (b < 0) break;
                cout << "  So ban chung giua toi va " << net.getUser(b)->name
                     << ": " << net.mutualFriends(me, b).size() << "\n";
                string kw = readText("  Loc theo ten (Enter neu khong loc): ", true);
                showSorted(search.searchMutualFriends(me, b, kw));
                break;
            }
        }
    }
}

vector<Suggestion> App::suggestByKeyword(Attr a) const {
    string kw = readText(string("  Nhap tu khoa ") + attrLabel(a) +
                         " (vd: hat se khop ca hat; Enter = dung thong tin cua toi): ", true);
    if (kw.empty() && net.getUser(me)->values(a).empty()) {
        cout << "  ! Ban chua co thong tin \"" << attrLabel(a) << "\", hay nhap tu khoa.\n";
        return vector<Suggestion>();
    }
    return suggester.suggestByAttribute(me, a, kw);
}

void App::suggestMenu() {
    while (true) {
        cout << "\n===== GOI Y BAN BE CHO TOI =====\n"
             << "1. Ban cua ban\n"
             << "2. Goi y tong hop\n"
             << "3. Theo noi o\n"
             << "4. Theo que quan\n"
             << "5. Theo truong hoc\n"
             << "6. Theo noi lam viec\n"
             << "7. Theo so thich\n"
             << "8. Trong nhom cua toi\n"
             << "0. Quay lai\n";
        int c = readInt("Chon: ", 0, 8);
        if (c == 0) return;
        vector<Suggestion> res;
        switch (c) {
            case 1: res = suggester.suggestFriends(me); break;
            case 2: res = suggester.suggestAll(me); break;
            case 3: res = suggestByKeyword(A_CITY); break;
            case 4: res = suggestByKeyword(A_HOMETOWN); break;
            case 5: res = suggestByKeyword(A_SCHOOL); break;
            case 6: res = suggestByKeyword(A_WORKPLACE); break;
            case 7: res = suggestByKeyword(A_HOBBY); break;
            case 8: {
                int g = pickMyGroup();
                if (g < 0) continue;
                res = suggester.suggestInGroup(me, g);
                break;
            }
        }
        printSuggestions(res);
    }
}

void App::bfsMenu() {
    while (true) {
        cout << "\n===== KET NOI (BFS) TU TAI KHOAN CUA TOI =====\n"
             << "1. Khoang cach ngan nhat giua toi va 1 nguoi\n"
             << "2. Duong ket noi ngan nhat tu toi den 1 nguoi\n"
             << "3. Nhung nguoi cach toi dung k buoc\n"
             << "4. Kiem tra toi co duong noi den 1 nguoi khong\n"
             << "0. Quay lai\n";
        int c = readInt("Chon: ", 0, 4);
        if (c == 0) return;
        if (c == 3) {
            int k = readInt("k (1-20): ", 1, 20);
            showSorted(bfsAlgo.getUsersAtDistance(me, k));
            continue;
        }
        int b = pickOther("Nguoi can kiem tra");
        if (b < 0) continue;
        if (c == 1) {
            int d = bfsAlgo.calculateDistance(me, b);
            if (d < 0) cout << "  Khong co duong noi giua toi va nguoi nay.\n";
            else cout << "  Khoang cach ngan nhat: " << d << " buoc"
                      << (d == 1 ? " (la ban be)" : "") << ".\n";
        } else if (c == 2) {
            vector<int> p = bfsAlgo.findShortestPath(me, b);
            if (p.empty()) cout << "  Khong co duong noi giua toi va nguoi nay.\n";
            else {
                cout << "  ";
                for (size_t i = 0; i < p.size(); ++i) {
                    if (i) cout << " -> ";
                    cout << (p[i] == me ? string("Toi") : net.getUser(p[i])->name) << "(#" << p[i] << ")";
                }
                cout << "\n";
            }
        } else {
            cout << (bfsAlgo.isReachable(me, b) ? "  Co duong noi.\n" : "  Khong co duong noi.\n");
        }
    }
}

bool App::chooseAccount() {
    cout << "\nChon tai khoan cua ban:\n";
    me = pickUser("Tai khoan cua toi");
    if (me < 0) return false;
    cout << "\nDang dung tai khoan:\n";
    printProfile(me);
    return true;
}

void App::run() {
    while (true) {
        cout << "\n========== MENU CHINH ==========\n"
             << "Tai khoan: [#" << me << "] " << net.getUser(me)->name
             << " (" << net.getUser(me)->friends.size() << " ban)\n"
             << "1. Tim kiem\n"
             << "2. Goi y ban be\n"
             << "3. Khoang cach & duong ket noi (BFS)\n"
             << "0. Thoat\n";
        int c = readInt("Chon: ", 0, 3);
        switch (c) {
            case 0: cout << "Tam biet!\n"; return;
            case 1: searchMenu(); break;
            case 2: suggestMenu(); break;
            case 3: bfsMenu(); break;
        }
    }
}

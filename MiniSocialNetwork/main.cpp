#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

#include "UserManager.h"
#include "ProfileManager.h"
#include "FileManager.h"

using namespace std;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    cout << "\nNhan Enter de tiep tuc..." << endl;
    cin.clear();
    string dummy;
    getline(cin, dummy);
}

void setupConsole() {
#ifdef _WIN32
    // Set UTF-8 for Vietnamese support
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    // Optional: set console font if needed
#endif
    // Keep sync for better compatibility on Windows terminals
    // ios_base::sync_with_stdio(false);
    // cin.tie(nullptr);
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

vector<string> splitInterests(const string& input) {
    vector<string> result;
    stringstream ss(input);
    string item;
    while (getline(ss, item, ',')) {
        string t = trim(item);
        if (!t.empty()) result.push_back(t);
    }
    return result;
}

void printHeader(const string& title) {
    cout << "\n========================================\n";
    cout << "  " << title << "\n";
    cout << "========================================\n";
}

void showMainMenu() {
    printHeader("MANG XA HOI MINI - VIET NAM");
    cout << "1. Dang nhap\n";
    cout << "2. Dang ky tai khoan\n";
    cout << "0. Thoat chuong trinh\n";
    cout << "----------------------------------------\n";
    cout << "Chon: ";
}

void showUserMenu(User* user) {
    printHeader("MENU NGUOI DUNG: " + user->getUsername());
    cout << "1. Xem ho so cua toi\n";
    cout << "2. Chinh sua ho so\n";
    cout << "3. Tim kiem nguoi dung\n";
    cout << "4. Xem danh sach ban be\n";
    cout << "5. Gui loi moi ket ban\n";
    cout << "6. Xem / Chap nhan loi moi ket ban\n";
    cout << "7. Huy ket ban\n";
    cout << "8. Xem ho so nguoi khac\n";
    cout << "9. Dang xuat\n";
    cout << "0. Thoat chuong trinh\n";
    cout << "----------------------------------------\n";
    cout << "Chon: ";
}

void showAdminMenu(User* user) {
    printHeader("MENU ADMIN: " + user->getUsername());
    cout << "1. Xem ho so cua toi\n";
    cout << "2. Chinh sua ho so\n";
    cout << "3. Tim kiem nguoi dung\n";
    cout << "4. Xem danh sach ban be\n";
    cout << "5. Gui loi moi ket ban\n";
    cout << "6. Xem / Chap nhan loi moi ket ban\n";
    cout << "7. Huy ket ban\n";
    cout << "8. Xem ho so nguoi khac\n";
    cout << "9. [Admin] Xem tat ca tai khoan\n";
    cout << "10.[Admin] Xoa tai khoan\n";
    cout << "11. Dang xuat\n";
    cout << "0. Thoat chuong trinh\n";
    cout << "----------------------------------------\n";
    cout << "Chon: ";
}

void doRegister(UserManager& um, FileManager& fm) {
    printHeader("DANG KY TAI KHOAN");
    string username, password, confirm;
    cout << "Nhap username: ";
    getline(cin, username);
    username = trim(username);
    if (username.empty()) {
        cout << "Username khong duoc de trong!\n";
        return;
    }
    if (username == "admin") {
        cout << "Username 'admin' da duoc danh rieng. Vui long chon ten khac!\n";
        return;
    }
    if (um.isUsernameTaken(username)) {
        cout << "Username da ton tai!\n";
        return;
    }
    cout << "Nhap mat khau: ";
    getline(cin, password);
    cout << "Nhap lai mat khau: ";
    getline(cin, confirm);
    if (password != confirm) {
        cout << "Mat khau khong khop!\n";
        return;
    }
    if (password.empty()) {
        cout << "Mat khau khong duoc de trong!\n";
        return;
    }

    // Always register as normal user (no admin choice)
    User* user = um.registerUser(username, password, false);
    if (user) {
        cout << "Dang ky thanh cong! ID: " << user->getId() << "\n";
        cout << "Nhap ho ten day du: ";
        string fullName;
        getline(cin, fullName);

        cout << "Nhap thanh pho ban dang song: ";
        string city;
        getline(cin, city);

        Profile p;
        p.createProfile(fullName, trim(city), "", "", "", vector<string>());
        user->setProfile(p);
        fm.saveAll(um);
        cout << "Ho so da duoc tao. Ban co the chinh sua them sau!\n";
    } else {
        cout << "Dang ky that bai!\n";
    }
}

bool doLogin(UserManager& um) {
    printHeader("DANG NHAP");
    string username, password;
    cout << "Username: ";
    getline(cin, username);
    cout << "Mat khau: ";
    getline(cin, password);
    User* user = um.login(trim(username), password);
    if (user) {
        cout << "Dang nhap thanh cong! Xin chao " << user->getUsername() << "\n";
        return true;
    }
    cout << "Sai username hoac mat khau!\n";
    return false;
}

void doViewMyProfile(ProfileManager& pm, User* user) {
    pm.viewProfile(user->getId());
}

void doEditProfile(ProfileManager& pm, UserManager& um, FileManager& fm, User* user) {
    printHeader("CHINH SUA HO SO");
    string fullName, city, hometown, school, workplace, interestsStr;
    cout << "Ho ten day du [" << user->getProfile().getFullName() << "]: ";
    getline(cin, fullName);
    if (fullName.empty()) fullName = user->getProfile().getFullName();

    cout << "Thanh pho [" << user->getProfile().getCity() << "]: ";
    getline(cin, city);
    if (city.empty()) city = user->getProfile().getCity();

    cout << "Que quan [" << user->getProfile().getHometown() << "]: ";
    getline(cin, hometown);
    if (hometown.empty()) hometown = user->getProfile().getHometown();

    cout << "Truong hoc [" << user->getProfile().getSchool() << "]: ";
    getline(cin, school);
    if (school.empty()) school = user->getProfile().getSchool();

    cout << "Noi lam viec [" << user->getProfile().getWorkplace() << "]: ";
    getline(cin, workplace);
    if (workplace.empty()) workplace = user->getProfile().getWorkplace();

    // Goi y so thich
    cout << "\n--- Goi y so thich ---\n";
    cout << "1.The thao  2.Vui choi  3.Giai tri  4.Am nhac  5.Du lich\n";
    cout << "6.Doc sach  7.Game  8.Am thuc  9.Cong nghe  10.Phim anh\n";
    cout << "Ban co the nhap so (cach nhau dau phay) hoac tu viet so thich.\n";
    cout << "So thich hien tai: ";
    auto ints = user->getProfile().getInterests();
    if (ints.empty()) cout << "(chua co)";
    else {
        for (size_t i = 0; i < ints.size(); ++i) {
            if (i) cout << ", ";
            cout << ints[i];
        }
    }
    cout << "\nNhap so thich moi (Enter de giu nguyen): ";
    getline(cin, interestsStr);

    vector<string> interests;
    if (interestsStr.empty()) {
        interests = ints;
    } else {
        // Map số thành tên sở thích
        vector<string> suggestions = {
            "The thao", "Vui choi", "Giai tri", "Am nhac", "Du lich",
            "Doc sach", "Game", "Am thuc", "Cong nghe", "Phim anh"
        };
        stringstream ss(interestsStr);
        string token;
        while (getline(ss, token, ',')) {
            string t = trim(token);
            if (t.empty()) continue;
            // Nếu là số từ 1-10
            try {
                int idx = stoi(t);
                if (idx >= 1 && idx <= 10) {
                    interests.push_back(suggestions[idx - 1]);
                    continue;
                }
            } catch (...) {}
            // Nếu không phải số thì giữ nguyên text
            interests.push_back(t);
        }
    }

    if (pm.editProfile(user->getId(), fullName, city, hometown, school, workplace, interests)) {
        cout << "Cap nhat ho so thanh cong!\n";
        fm.saveAll(um);
    } else {
        cout << "Cap nhat that bai!\n";
    }
}

void doSearchUser(UserManager& um) {
    printHeader("TIM KIEM NGUOI DUNG");
    cout << "1. Tim theo username\n";
    cout << "2. Tim theo ID\n";
    cout << "Chon: ";
    string choice;
    getline(cin, choice);
    if (choice == "1") {
        cout << "Nhap username: ";
        string uname;
        getline(cin, uname);
        User* u = um.getUserByUsername(trim(uname));
        if (u) {
            cout << "Tim thay: ID=" << u->getId() << ", Username=" << u->getUsername()
                 << ", Ban be=" << u->getFriendCount() << "\n";
        } else {
            cout << "Khong tim thay!\n";
        }
    } else if (choice == "2") {
        cout << "Nhap ID: ";
        string idStr;
        getline(cin, idStr);
        try {
            int id = stoi(trim(idStr));
            User* u = um.getUserById(id);
            if (u) {
                cout << "Tim thay: ID=" << u->getId() << ", Username=" << u->getUsername()
                     << ", Ban be=" << u->getFriendCount() << "\n";
            } else {
                cout << "Khong tim thay!\n";
            }
        } catch (...) {
            cout << "ID khong hop le!\n";
        }
    }
}

void showFriendSuggestions(UserManager& um, User* user) {
    string myCity = user->getProfile().getCity();
    if (myCity.empty()) {
        cout << "Ban chua cap nhat thanh pho trong ho so. Hay chinh sua ho so de nhan goi y ban be!\n";
        return;
    }

    cout << "\n--- Goi y ban be cung thanh pho: " << myCity << " ---\n";
    auto allUsers = um.getAllUsers();
    int count = 0;
    for (User* u : allUsers) {
        if (u->getId() == user->getId()) continue;
        if (um.areFriends(user->getId(), u->getId())) continue;
        if (u->getIsAdmin()) continue; // khong goi y admin
        if (u->getProfile().getCity() == myCity) {
            cout << " + ID: " << u->getId()
                 << " | Username: " << u->getUsername()
                 << " | Ho ten: " << u->getProfile().getFullName() << "\n";
            ++count;
            if (count >= 8) break; // gioi han 8 goi y
        }
    }
    if (count == 0) {
        cout << "Hien chua co nguoi dung nao cung thanh pho voi ban.\n";
    } else {
        cout << "(Nhap username o tren de gui loi moi ket ban)\n";
    }
}

void doViewFriends(UserManager& um, User* user) {
    printHeader("DANH SACH BAN BE");
    auto friends = um.getFriends(user->getId());
    if (friends.empty()) {
        cout << "Ban chua co ban be nao.\n";
        showFriendSuggestions(um, user);
        return;
    }
    cout << "So ban be: " << friends.size() << "\n";
    for (User* f : friends) {
        cout << " - ID: " << f->getId() << " | Username: " << f->getUsername()
             << " | Ho ten: " << f->getProfile().getFullName() << "\n";
    }
}

void doSendFriendRequest(UserManager& um, FileManager& fm, User* user) {
    printHeader("GUI LOI MOI KET BAN");

    // Luon hien goi y neu chua co ban be
    auto friends = um.getFriends(user->getId());
    if (friends.empty()) {
        showFriendSuggestions(um, user);
        cout << "\n";
    }

    cout << "Nhap username nguoi muon ket ban (hoac Enter de quay lai): ";
    string uname;
    getline(cin, uname);
    uname = trim(uname);
    if (uname.empty()) return;

    User* target = um.getUserByUsername(uname);
    if (!target) {
        cout << "Khong tim thay nguoi dung!\n";
        return;
    }
    if (target->getId() == user->getId()) {
        cout << "Khong the ket ban voi chinh minh!\n";
        return;
    }
    if (um.areFriends(user->getId(), target->getId())) {
        cout << "Hai ban da la ban be roi!\n";
        return;
    }
    if (um.sendFriendRequest(user->getId(), target->getId())) {
        cout << "Da gui loi moi ket ban den " << target->getUsername() << "!\n";
        fm.saveAll(um);
    } else {
        cout << "Khong the gui loi moi (co the da gui truoc do)!\n";
    }
}

void doHandleRequests(UserManager& um, FileManager& fm, User* user) {
    printHeader("LOI MOI KET BAN");
    auto reqs = um.getPendingRequests(user->getId());
    if (reqs.empty()) {
        cout << "Khong co loi moi nao.\n";
        return;
    }
    cout << "Danh sach loi moi:\n";
    for (size_t i = 0; i < reqs.size(); ++i) {
        cout << (i + 1) << ". ID: " << reqs[i]->getId()
             << " | Username: " << reqs[i]->getUsername()
             << " | Ho ten: " << reqs[i]->getProfile().getFullName() << "\n";
    }
    cout << "Nhap so thu tu de chap nhan (0 de huy tat ca / bo qua): ";
    string choice;
    getline(cin, choice);
    try {
        int idx = stoi(trim(choice));
        if (idx == 0) {
            for (User* r : reqs) {
                um.rejectFriendRequest(user->getId(), r->getId());
            }
            cout << "Da tu choi tat ca loi moi.\n";
            fm.saveAll(um);
        } else if (idx >= 1 && idx <= static_cast<int>(reqs.size())) {
            User* r = reqs[idx - 1];
            if (um.acceptFriendRequest(user->getId(), r->getId())) {
                cout << "Da chap nhan ket ban voi " << r->getUsername() << "!\n";
                fm.saveAll(um);
            }
        } else {
            cout << "Lua chon khong hop le.\n";
        }
    } catch (...) {
        cout << "Lua chon khong hop le.\n";
    }
}

void doRemoveFriend(UserManager& um, FileManager& fm, User* user) {
    printHeader("HUY KET BAN");
    cout << "Nhap username ban muon huy ket ban: ";
    string uname;
    getline(cin, uname);
    User* target = um.getUserByUsername(trim(uname));
    if (!target) {
        cout << "Khong tim thay nguoi dung!\n";
        return;
    }
    if (um.removeFriend(user->getId(), target->getId())) {
        cout << "Da huy ket ban voi " << target->getUsername() << "!\n";
        fm.saveAll(um);
    } else {
        cout << "Hai ban khong phai la ban be!\n";
    }
}

void doViewOtherProfile(ProfileManager& pm, UserManager& um) {
    printHeader("XEM HO SO NGUOI KHAC");
    cout << "Nhap username: ";
    string uname;
    getline(cin, uname);
    User* u = um.getUserByUsername(trim(uname));
    if (u) {
        pm.viewProfile(u->getId());
    } else {
        cout << "Khong tim thay!\n";
    }
}

void doAdminListUsers(UserManager& um) {
    printHeader("DANH SACH TAT CA TAI KHOAN");
    auto users = um.getAllUsers();
    cout << "Tong so tai khoan: " << users.size() << "\n";
    for (User* u : users) {
        cout << "ID: " << u->getId()
             << " | Username: " << u->getUsername()
             << " | Admin: " << (u->getIsAdmin() ? "Co" : "Khong")
             << " | Ban be: " << u->getFriendCount()
             << " | Ho ten: " << u->getProfile().getFullName() << "\n";
    }
}

void doAdminDeleteUser(UserManager& um, FileManager& fm, User* current) {
    printHeader("XOA TAI KHOAN");
    cout << "Nhap username can xoa: ";
    string uname;
    getline(cin, uname);
    User* target = um.getUserByUsername(trim(uname));
    if (!target) {
        cout << "Khong tim thay!\n";
        return;
    }
    if (target->getId() == current->getId()) {
        cout << "Khong the tu xoa tai khoan cua minh!\n";
        return;
    }
    cout << "Ban co chac muon xoa tai khoan " << target->getUsername() << "? (y/n): ";
    string ans;
    getline(cin, ans);
    if (ans == "y" || ans == "Y") {
        if (um.deleteUser(target->getId())) {
            cout << "Da xoa tai khoan thanh cong!\n";
            fm.saveAll(um);
        } else {
            cout << "Xoa that bai!\n";
        }
    } else {
        cout << "Da huy.\n";
    }
}

int main() {
    setupConsole();

    cout << "Dang khoi dong chuong trinh..." << endl;

    UserManager um;
    ProfileManager pm(&um);
    FileManager fm("data");

    // Load existing data
    bool loaded = fm.loadAll(um);

    // Tao san tai khoan Admin neu chua co
    if (!um.isUsernameTaken("admin")) {
        User* admin = um.registerUser("admin", "admin12345", true);
        if (admin) {
            Profile p;
            p.createProfile("Quan Tri Vien", "Ha Noi", "Ha Noi", "", "Mini Social Network", vector<string>());
            admin->setProfile(p);
            fm.saveAll(um);
            cout << "Da tao tai khoan Admin mac dinh (admin / admin12345)\n";
        }
    }

    cout << "Da tai du lieu. So nguoi dung hien co: " << um.getUserCount() << endl;
    if (!loaded) {
        cout << "(Chua co du lieu cu hoac file rong)" << endl;
    }

    bool running = true;
    while (running) {
        if (!um.isLoggedIn()) {
            showMainMenu();
            string choice;
            getline(cin, choice);
            choice = trim(choice);
            if (choice == "1") {
                doLogin(um);
                pause();
            } else if (choice == "2") {
                doRegister(um, fm);
                pause();
            } else if (choice == "0") {
                running = false;
            } else {
                cout << "Lua chon khong hop le!\n";
                pause();
            }
        } else {
            User* current = um.getCurrentUser();
            if (current->getIsAdmin()) {
                showAdminMenu(current);
            } else {
                showUserMenu(current);
            }
            string choice;
            getline(cin, choice);
            choice = trim(choice);

            if (choice == "0") {
                running = false;
            } else if (choice == "1") {
                doViewMyProfile(pm, current);
                pause();
            } else if (choice == "2") {
                doEditProfile(pm, um, fm, current);
                pause();
            } else if (choice == "3") {
                doSearchUser(um);
                pause();
            } else if (choice == "4") {
                doViewFriends(um, current);
                pause();
            } else if (choice == "5") {
                doSendFriendRequest(um, fm, current);
                pause();
            } else if (choice == "6") {
                doHandleRequests(um, fm, current);
                pause();
            } else if (choice == "7") {
                doRemoveFriend(um, fm, current);
                pause();
            } else if (choice == "8") {
                doViewOtherProfile(pm, um);
                pause();
            } else if (current->getIsAdmin() && choice == "9") {
                doAdminListUsers(um);
                pause();
            } else if (current->getIsAdmin() && choice == "10") {
                doAdminDeleteUser(um, fm, current);
                pause();
            } else if ((!current->getIsAdmin() && choice == "9") ||
                       (current->getIsAdmin() && choice == "11")) {
                um.logout();
                cout << "Da dang xuat.\n";
                pause();
            } else {
                cout << "Lua chon khong hop le!\n";
                pause();
            }
        }
    }

    // Save before exit
    fm.saveAll(um);
    cout << "Da luu du lieu. Tam biet!\n";
    return 0;
}

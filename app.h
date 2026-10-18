// APP: menu console - khai bao
#ifndef APP_H
#define APP_H

#include "core.h"
#include "search_service.h"
#include "sort_service.h"
#include "bfs_algorithm.h"
#include "friend_suggester.h"

class App {
    SocialNetwork net;
    SearchService search;
    BFSAlgorithm bfsAlgo;
    FriendSuggester suggester;
    SortService sorter;
    int me;                                    // ID tai khoan dang dung

    // ----- hien thi -----
    // In du thong tin de nhin vao la biet vi sao nguoi do khop (noi o, que, truong, noi lam viec, so thich)
    static std::string orDash(const std::string& v) { return v.empty() ? std::string("-") : v; }
    void printBrief(int id) const;
    void printUserList(const std::vector<int>& ids) const;
    void printProfile(int id) const;
    void printSuggestions(const std::vector<Suggestion>& v) const;
    void showSorted(std::vector<int> ids) const;

    // ----- chon doi tuong -----
    // Nhap ID HOAC ten. Nhap ten: khong ai khop -> bao loi; dung 1 nguoi -> chon luon;
    // nhieu nguoi trung ten -> in danh sach kem ID, bat chon ID trong danh sach do.
    // Tra ve -1 neu nhap 0 de huy.
    int pickUser(const std::string& label) const;
    // Chon mot nguoi KHAC toi (-1 neu huy)
    int pickOther(const std::string& label) const;
    // Chon 1 nhom MA TOI DANG THAM GIA (-1 neu khong co / huy)
    int pickMyGroup() const;
    Attr pickAttr() const;
    // Pham vi tim kiem -> tap nguoi dung "base" (khong gom chinh toi). false neu huy.
    bool pickScope(std::vector<int>& base) const;

    // ----- tim kiem -----
    // Tim theo MOT thuoc tinh rieng (noi o / que quan / noi lam viec / truong / so thich)
    void attrSearch(Attr a);

    void searchMenu();

    // ----- goi y ban be (cho tai khoan cua toi) -----
    std::vector<Suggestion> suggestByKeyword(Attr a) const;

    void suggestMenu();

    // ----- BFS: khoang cach giua toi va nguoi khac -----
    void bfsMenu();

public:
    App() : search(net), bfsAlgo(net), suggester(net, bfsAlgo), sorter(net), me(-1) {}

    bool loadData(const std::string& path) { return DataLoader::load(path, net); }

    // Chon tai khoan dang dung (nhap ID hoac ten). false neu huy.
    bool chooseAccount();

    void run();
};

#endif // APP_H

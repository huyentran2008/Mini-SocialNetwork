// MAIN: diem vao chuong trinh
// Bien dich: g++ -std=c++11 -O2 *.cpp -o social      Chay: ./social data.txt
#include <iostream>
#include <string>
#include "app.h"

using namespace std;

int main(int argc, char** argv) {
    cout << "=====================================\n"
         << "     MANG XA HOI MINI (C++11)\n"
         << "=====================================\n";
    App app;
    string path = (argc > 1) ? argv[1] : "data.txt";
    while (!app.loadData(path)) {
        path = readLine("Nhap duong dan file du lieu (0 = thoat): ");
        if (path == "0") return 0;
    }
    if (!app.chooseAccount()) return 0;
    app.run();
    return 0;
}

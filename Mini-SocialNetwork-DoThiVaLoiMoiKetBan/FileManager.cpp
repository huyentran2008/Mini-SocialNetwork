#include "FileManager.h"
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#define createDir(path) _mkdir(path)
#else
#include <unistd.h>
#define createDir(path) mkdir(path, 0777)
#endif

static void makeDir(const std::string& path) {
    createDir(path.c_str());
}

void FileManager::initializeSystemDirectories() {
    makeDir("Data");
    makeDir("Data/Accounts");
    makeDir("Data/Conversations");
    makeDir("Data/Conversations/PRIVATE");
    makeDir("Data/Conversations/GROUP");
    makeDir("Data/Posts");
    makeDir("Data/System");

    std::ofstream("Data/System/account_id.txt", std::ios::app).close();
    std::ofstream("Data/System/group_id.txt", std::ios::app).close();
    std::ofstream("Data/System/post_id.txt", std::ios::app).close();

    std::ifstream checkAcc("Data/System/account_id.txt");
    checkAcc.seekg(0, std::ios::end);
    if (checkAcc.tellg() == 0) {
        checkAcc.close();
        saveNextId("account", 1);
    } else {
        checkAcc.close();
    }

    std::ifstream checkPost("Data/System/post_id.txt");
    checkPost.seekg(0, std::ios::end);
    if (checkPost.tellg() == 0) {
        checkPost.close();
        saveNextId("post", 1);
    } else {
        checkPost.close();
    }
}

void FileManager::createAccountDataStructure(const std::string& accountId, 
                                           const std::string& username, 
                                           const std::string& fullName, 
                                           const std::string& password) {
    std::string accDir = "Data/Accounts/" + accountId;
    makeDir(accDir);

    std::ofstream profileFile(accDir + "/profile.txt");
    if (profileFile.is_open()) {
        profileFile << "id=" << accountId << "\n";
        profileFile << "username=" << username << "\n";
        profileFile << "password=" << password << "\n";
        profileFile << "full_name=" << fullName << "\n";
        profileFile << "birthday=2008-05-20\n";
        profileFile << "gender=Male\n";
        profileFile << "bio=Xin chào các con zời\n";
        profileFile << "created_at=2026-10-04\n";
        profileFile.close();
    }

    std::ofstream friendFile(accDir + "/friend.txt");
    if (friendFile.is_open()) {
        friendFile << "ACC000002|FRIEND|2026-10-01\n";
        friendFile << "ACC000003|PENDING_SENT|2026-10-03\n";
        friendFile.close();
    }

    std::ofstream postsFile(accDir + "/posts.txt");
    if (postsFile.is_open()) {
        postsFile << "PST000001|2026-10-01\n";
        postsFile.close();
    }
}

bool FileManager::fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.is_open();
}

int FileManager::getNextId(const std::string& type) {
    std::string path = "Data/System/";
    if (type == "account") path += "account_id.txt";
    else if (type == "group") path += "group_id.txt";
    else if (type == "post") path += "post_id.txt";
    else return 1;

    std::ifstream file(path);
    int id = 1;
    if (file.is_open()) {
        file >> id;
        file.close();
    }
    return id;
}

void FileManager::saveNextId(const std::string& type, int nextId) {
    std::string path = "Data/System/";
    if (type == "account") path += "account_id.txt";
    else if (type == "group") path += "group_id.txt";
    else if (type == "post") path += "post_id.txt";
    else return;

    std::ofstream file(path, std::ios::trunc);
    if (file.is_open()) {
        file << nextId;
        file.close();
    }
}
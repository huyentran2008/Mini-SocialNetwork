#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

class FileManager {
public:
    static void initializeSystemDirectories();
    static void createAccountDataStructure(const std::string& accountId, 
                                           const std::string& username, 
                                           const std::string& fullName, 
                                           const std::string& password);
    static bool fileExists(const std::string& filePath);
    static int getNextId(const std::string& type);
    static void saveNextId(const std::string& type, int nextId);
};

#endif
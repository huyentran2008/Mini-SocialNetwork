#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <vector>
#include <sstream>

class Profile {
private:
    std::string fullName;
    std::string city;
    std::string hometown;
    std::string school;
    std::string workplace;
    std::vector<std::string> interests;

public:
    Profile();
    Profile(const std::string& fullName, const std::string& city,
            const std::string& hometown, const std::string& school,
            const std::string& workplace, const std::vector<std::string>& interests);

    // Required methods
    void createProfile(const std::string& fullName, const std::string& city,
                       const std::string& hometown, const std::string& school,
                       const std::string& workplace, const std::vector<std::string>& interests);
    void updateProfile(const std::string& fullName, const std::string& city,
                       const std::string& hometown, const std::string& school,
                       const std::string& workplace, const std::vector<std::string>& interests);
    bool validateProfile() const;
    std::string getFullName() const;
    std::string getProfileSummary() const;
    std::string getCity() const;
    std::string getHometown() const;
    std::string getSchool() const;
    std::string getWorkplace() const;
    std::vector<std::string> getInterests() const;
    int countCommonInterests(const Profile& other) const;

    // Helpers for file I/O
    std::string serialize() const;
    void deserialize(const std::string& data);
    void clear();
};

#endif

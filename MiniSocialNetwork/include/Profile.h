#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <vector>
#include <iostream>

class Profile {
private:
    std::string fullName;
    int age;
    std::string gender;
    std::string city;
    std::string hometown;
    std::string school;
    std::string workplace;
    std::vector<std::string> interests;
    std::string bio;

public:
    Profile();

    // Tạo hồ sơ từ dữ liệu đầu vào
    static Profile createProfile(
        const std::string& fullName,
        int age,
        const std::string& gender,
        const std::string& city,
        const std::string& hometown,
        const std::string& school,
        const std::string& workplace,
        const std::vector<std::string>& interests,
        const std::string& bio
    );

    // Cập nhật các trường của hồ sơ
    void updateProfile(
        const std::string& fullName,
        int age,
        const std::string& gender,
        const std::string& city,
        const std::string& hometown,
        const std::string& school,
        const std::string& workplace,
        const std::vector<std::string>& interests,
        const std::string& bio
    );

    // Kiểm tra tính hợp lệ của dữ liệu hồ sơ
    bool validateProfile() const;

    // Getter
    std::string getFullName() const;
    int getAge() const;
    std::string getGender() const;
    std::string getCity() const;
    std::string getHometown() const;
    std::string getSchool() const;
    std::string getWorkplace() const;
    std::vector<std::string> getInterests() const;
    std::string getBio() const;

    // Trả về chuỗi tóm tắt hồ sơ để UI hiển thị
    std::string getProfileSummary() const;

    // Đếm số sở thích chung với một hồ sơ khác
    int countCommonInterests(const Profile& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Profile& profile);
};

#endif

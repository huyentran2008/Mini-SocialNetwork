#include "../include/Profile.h"
#include <sstream>
#include <algorithm>

Profile::Profile() : age(0) {}

Profile Profile::createProfile(
    const std::string& fullName,
    int age,
    const std::string& gender,
    const std::string& city,
    const std::string& hometown,
    const std::string& school,
    const std::string& workplace,
    const std::vector<std::string>& interests,
    const std::string& bio
) {
    Profile p;
    p.fullName = fullName;
    p.age = age;
    p.gender = gender;
    p.city = city;
    p.hometown = hometown;
    p.school = school;
    p.workplace = workplace;
    p.interests = interests;
    p.bio = bio;
    return p;
}

void Profile::updateProfile(
    const std::string& fullName,
    int age,
    const std::string& gender,
    const std::string& city,
    const std::string& hometown,
    const std::string& school,
    const std::string& workplace,
    const std::vector<std::string>& interests,
    const std::string& bio
) {
    this->fullName = fullName;
    this->age = age;
    this->gender = gender;
    this->city = city;
    this->hometown = hometown;
    this->school = school;
    this->workplace = workplace;
    this->interests = interests;
    this->bio = bio;
}

bool Profile::validateProfile() const {
    if (fullName.empty()) return false;
    if (age < 1 || age > 120) return false;
    if (gender.empty()) return false;
    return true;
}

std::string Profile::getFullName() const { return fullName; }
int Profile::getAge() const { return age; }
std::string Profile::getGender() const { return gender; }
std::string Profile::getCity() const { return city; }
std::string Profile::getHometown() const { return hometown; }
std::string Profile::getSchool() const { return school; }
std::string Profile::getWorkplace() const { return workplace; }
std::vector<std::string> Profile::getInterests() const { return interests; }
std::string Profile::getBio() const { return bio; }

std::string Profile::getProfileSummary() const {
    std::ostringstream oss;
    oss << "Họ tên: " << fullName
        << " | Tuổi: " << age
        << " | Giới tính: " << gender
        << " | Thành phố: " << city
        << " | Quê quán: " << hometown
        << " | Trường: " << school
        << " | Nơi làm việc: " << workplace
        << " | Sở thích: ";
    for (size_t i = 0; i < interests.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << interests[i];
    }
    oss << " | Giới thiệu: " << bio;
    return oss.str();
}

int Profile::countCommonInterests(const Profile& other) const {
    int count = 0;
    for (const auto& interest : interests) {
        for (const auto& otherInterest : other.interests) {
            if (interest == otherInterest) {
                ++count;
                break;
            }
        }
    }
    return count;
}

std::ostream& operator<<(std::ostream& os, const Profile& profile) {
    os << profile.getProfileSummary();
    return os;
}

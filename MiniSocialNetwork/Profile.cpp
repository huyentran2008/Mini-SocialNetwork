#include "Profile.h"
#include <algorithm>
#include <sstream>

Profile::Profile() {
    clear();
}

Profile::Profile(const std::string& fullName, const std::string& city,
                 const std::string& hometown, const std::string& school,
                 const std::string& workplace, const std::vector<std::string>& interests)
    : fullName(fullName), city(city), hometown(hometown),
      school(school), workplace(workplace), interests(interests) {}

void Profile::createProfile(const std::string& fullName, const std::string& city,
                            const std::string& hometown, const std::string& school,
                            const std::string& workplace, const std::vector<std::string>& interests) {
    this->fullName = fullName;
    this->city = city;
    this->hometown = hometown;
    this->school = school;
    this->workplace = workplace;
    this->interests = interests;
}

void Profile::updateProfile(const std::string& fullName, const std::string& city,
                            const std::string& hometown, const std::string& school,
                            const std::string& workplace, const std::vector<std::string>& interests) {
    createProfile(fullName, city, hometown, school, workplace, interests);
}

bool Profile::validateProfile() const {
    return !fullName.empty();
}

std::string Profile::getFullName() const {
    return fullName;
}

std::string Profile::getProfileSummary() const {
    std::ostringstream oss;
    oss << "Ho ten: " << fullName << "\n"
        << "Thanh pho: " << city << "\n"
        << "Que quan: " << hometown << "\n"
        << "Truong: " << school << "\n"
        << "Noi lam viec: " << workplace << "\n"
        << "So thich: ";
    if (interests.empty()) {
        oss << "(khong co)";
    } else {
        for (size_t i = 0; i < interests.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << interests[i];
        }
    }
    return oss.str();
}

std::string Profile::getCity() const { return city; }
std::string Profile::getHometown() const { return hometown; }
std::string Profile::getSchool() const { return school; }
std::string Profile::getWorkplace() const { return workplace; }
std::vector<std::string> Profile::getInterests() const { return interests; }

int Profile::countCommonInterests(const Profile& other) const {
    int count = 0;
    for (const auto& interest : interests) {
        if (std::find(other.interests.begin(), other.interests.end(), interest) != other.interests.end()) {
            ++count;
        }
    }
    return count;
}

std::string Profile::serialize() const {
    std::ostringstream oss;
    oss << fullName << "|" << city << "|" << hometown << "|"
        << school << "|" << workplace << "|";
    for (size_t i = 0; i < interests.size(); ++i) {
        if (i > 0) oss << ",";
        oss << interests[i];
    }
    return oss.str();
}

void Profile::deserialize(const std::string& data) {
    clear();
    std::istringstream iss(data);
    std::string token;
    std::vector<std::string> parts;
    while (std::getline(iss, token, '|')) {
        parts.push_back(token);
    }
    if (parts.size() >= 5) {
        fullName = parts[0];
        city = parts[1];
        hometown = parts[2];
        school = parts[3];
        workplace = parts[4];
        if (parts.size() >= 6 && !parts[5].empty()) {
            std::istringstream iss2(parts[5]);
            std::string interest;
            while (std::getline(iss2, interest, ',')) {
                if (!interest.empty()) interests.push_back(interest);
            }
        }
    }
}

void Profile::clear() {
    fullName.clear();
    city.clear();
    hometown.clear();
    school.clear();
    workplace.clear();
    interests.clear();
}

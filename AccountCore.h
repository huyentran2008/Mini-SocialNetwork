// Module: Kiểm tra tài khoản, validate dữ liệu, tạo tài khoản, trạng thái đăng nhập
#pragma once

// Đọc username và password (đã mã hóa Hex) từ file profile.txt
bool readProfile(const fs::path& profileFile,
                 string& fileUsername,
                 string& filePassword) {
    ifstream file(profileFile);

    if (!file.is_open()) {
        return false;
    }

    fileUsername.clear();
    filePassword.clear();

    string line;

    while (getline(file, line)) {
        // Bỏ ký tự '\r' nếu file có xuống dòng kiểu Windows
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t pos = line.find('=');

        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "username") {
            fileUsername = value;
        }
        else if (key == "password") {
            filePassword = value;
        }
    }

    return true;
}
// Kiểm tra tài khoản khi đăng nhập và đăng ký
bool checkAccount(const string& inputUsername,
                  const string& inputPassword,
                  bool onlyCheckUsername = false) {

    // Thư mục chứa các tài khoản
    fs::path accountDir = "Data/Account";

    // Kiểm tra thư mục có tồn tại không
    error_code ec;

    if (!fs::is_directory(accountDir, ec) || ec) {
        return false;
    }

    // Mã hóa mật khẩu vừa nhập sang Hex
    string inputPasswordHex = stringToHex(inputPassword);

    // Duyệt các thư mục con có dạng ACC<số>
    for (const auto& entry : fs::directory_iterator(accountDir, ec)) {

        if (!entry.is_directory()) {
            continue;
        }

        string dirName = entry.path().filename().string();

        // Tên thư mục phải bắt đầu bằng "ACC" và theo sau là ít nhất 1 chữ số
        if (dirName.size() < 4 || dirName.substr(0, 3) != "ACC") {
            continue;
        }

        bool isValid = all_of(dirName.begin() + 3, dirName.end(),
                              [](unsigned char c) { return isdigit(c); });

        if (!isValid) {
            continue;
        }

        // Đọc thông tin trong profile.txt
        fs::path profileFile = entry.path() / "profile.txt";

        if (!fs::is_regular_file(profileFile, ec)) {
            continue;
        }

        string fileUsername;
        string filePassword;

        if (!readProfile(profileFile, fileUsername, filePassword)) {
            continue;
        }

        // So sánh username
        if (fileUsername != inputUsername) {
            continue;
        }

        // Chỉ cần kiểm tra username tồn tại
        if (onlyCheckUsername) {
            return true;
        }

        // So sánh thêm mật khẩu Hex
        if (filePassword == inputPasswordHex) {
            return true;
        }
    }

    return false;
}

/*
    TÀI KHOẢN QUẢN TRỊ VIÊN VÀ TRẠNG THÁI KHÓA TÀI KHOẢN
    - Quản trị viên đăng nhập ở cùng màn hình đăng nhập của người dùng.
      Đổi ADMIN_USERNAME / ADMIN_PASSWORD bên dưới để đổi tài khoản quản trị.
      (mật khẩu vẫn phải thỏa quy tắc 8-15 ký tự, có chữ hoa, chữ thường, số, ký tự đặc biệt)
    - Trạng thái tài khoản người dùng nằm ở dòng status= trong profile.txt:
      ACTIVE (hoạt động, mặc định nếu thiếu dòng này) hoặc LOCKED (bị quản trị viên khóa)
                                                                                    */
const string ADMIN_USERNAME = "admin";
const string ADMIN_PASSWORD = "Admin@123";

// Đúng tài khoản và mật khẩu quản trị viên
bool isAdminLogin(const string& username, const string& password) {
    return username == ADMIN_USERNAME && password == ADMIN_PASSWORD;
}
// Tên tài khoản dành riêng cho quản trị viên (không phân biệt hoa thường), người dùng không được đăng ký
bool isReservedUsername(const string& username) {
    string lower = username;

    for (char& c : lower) {
        c = (char)tolower((unsigned char)c);
    }

    return lower == ADMIN_USERNAME;
}
// Đọc giá trị của 1 khóa trong profile.txt ("" nếu không có)
string readProfileValue(const fs::path& profileFile, const string& key) {
    ifstream file(profileFile);

    if (!file.is_open()) {
        return "";
    }

    string line;
    string prefix = key + "=";

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.compare(0, prefix.size(), prefix) == 0) {
            return line.substr(prefix.size());
        }
    }

    return "";
}
// Tài khoản (theo username) có đang bị khóa không
bool isUsernameLocked(const string& username) {
    error_code ec;

    for (const auto& entry : fs::directory_iterator("Data/Account", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        fs::path profileFile = entry.path() / "profile.txt";

        if (readProfileValue(profileFile, "username") == username) {
            return readProfileValue(profileFile, "status") == "LOCKED";
        }
    }

    return false;
}

/*
    KIỂM TRA DỮ LIỆU VÀ TẠO TÀI KHOẢN
                                        */

// Dữ liệu người dùng nhập trong form đăng ký
struct RegisterForm {
    string username;
    string password;
    string fullName;
    string birthday;    // DD-MM-YYYY (có thể trống)
    string gender;      // Male / Female / Other (có thể trống)
    string bio;         // Giới thiệu (có thể trống)
};
// Kiểm tra mật khẩu hợp lệ: 8-15 ký tự, có chữ hoa, chữ thường, số, ký tự đặc biệt
bool isValidPassword(const string& s) {
    if(s.length() < 8 || s.length() > 15) {
        return false;
    }

    bool hasDigit = false;
    bool hasUpper = false;
    bool hasLower = false;
    bool hasSpecial = false;

    for (unsigned char c : s) {
        if(c >= '0' && c <= '9')       hasDigit = true;
        else if(c >= 'A' && c <= 'Z')  hasUpper = true;
        else if(c >= 'a' && c <= 'z')  hasLower = true;
        else                           hasSpecial = true;
    }

    return hasDigit && hasUpper &&
           hasLower && hasSpecial;
}
// Năm nhuận: chia hết cho 400, hoặc chia hết cho 4 nhưng không chia hết cho 100
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
// Số ngày của 1 tháng trong 1 năm (có tính năm nhuận)
int daysInMonth(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30,
                               31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    return days[month - 1];
}
// Các hàm kiểm tra: trả về chuỗi rỗng nếu hợp lệ, ngược lại trả về thông báo lỗi
string validateUsername(const string& s) {
    if (s.length() < 5 || s.length() > 15) {
        return "Tài khoản phải có từ 5 đến 15 ký tự!";
    }

    for (unsigned char c : s) {
        bool ok = (c >= '0' && c <= '9') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  c == '_';

        if (!ok) {
            return "Tài khoản chỉ gồm chữ cái, số và dấu gạch dưới!";
        }
    }

    if (isReservedUsername(s)) {
        return "Tên tài khoản này dành riêng cho hệ thống, hãy chọn tên khác!";
    }

    if (checkAccount(s, "", true)) {
        return "Tài khoản đã tồn tại!";
    }

    return "";
}
string validatePassword(const string& s) {
    if (!isValidPassword(s)) {
        return "Mật khẩu phải có chữ hoa, chữ thường, số, ký tự đặc biệt và dài 8-15 ký tự!";
    }

    return "";
}
// Kiểm tra tên đã có người dùng chưa (định nghĩa ở phần mở rộng phía dưới)
bool fullNameTaken(const string& name, const string& exceptAccountId);

string validateFullName(const string& s, const string& exceptAccountId = "") {
    string name = trimString(s);

    if (name.empty()) {
        return "Họ và tên không được để trống!";
    }

    if (textLength(name) > 50) {
        return "Họ và tên tối đa 50 ký tự!";
    }

    if (fullNameTaken(name, exceptAccountId)) {
        return "Họ và tên đã có người sử dụng!";
    }

    return "";
}
// Ngày sinh: có thể trống, nếu nhập thì phải đúng dạng DD-MM-YYYY và là ngày có thật
string validateBirthday(const string& s) {
    if (s.empty()) {
        return "";
    }

    // Kiểm tra định dạng DD-MM-YYYY
    if (s.length() != 10 || s[2] != '-' || s[5] != '-') {
        return "Ngày sinh phải có dạng DD-MM-YYYY!";
    }

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;

        if (s[i] < '0' || s[i] > '9') {
            return "Ngày sinh phải có dạng DD-MM-YYYY!";
        }
    }

    int day   = stoi(s.substr(0, 2));
    int month = stoi(s.substr(3, 2));
    int year  = stoi(s.substr(6, 4));

    if (year < 1900) {
        return "Năm sinh phải từ 1900 trở đi!";
    }

    if (month < 1 || month > 12) {
        return "Tháng phải từ 01 đến 12!";
    }

    int maxDay = daysInMonth(month, year);

    if (day < 1 || day > maxDay) {
        return "Tháng " + to_string(month) + "/" + to_string(year)
             + " chỉ có " + to_string(maxDay) + " ngày!";
    }

    // Ngày sinh không được ở tương lai
    SYSTEMTIME st;
    GetLocalTime(&st);

    int today = (int)st.wYear * 10000 + (int)st.wMonth * 100 + (int)st.wDay;
    int birth = year * 10000 + month * 100 + day;

    if (birth > today) {
        return "Ngày sinh không được ở tương lai!";
    }

    return "";
}
// Giới tính: có thể trống, nếu có thì phải là Male / Female / Other
string validateGender(const string& s) {
    if (s.empty()) {
        return "";
    }

    if (find(GENDER_OPTIONS.begin(), GENDER_OPTIONS.end(), s) == GENDER_OPTIONS.end()) {
        return "Giới tính chỉ được chọn Male, Female hoặc Other!";
    }

    return "";
}
string validateBio(const string& s) {
    if (textLength(trimString(s)) > 100) {
        return "Giới thiệu tối đa 100 ký tự!";
    }

    return "";
}
// Kiểm tra 1 ô của form: 1 = tài khoản, 2 = mật khẩu, 3 = họ tên,
// 4 = ngày sinh, 5 = giới tính, 6 = giới thiệu
string validateRegisterField(int field, const RegisterForm& form) {
    switch (field) {
        case 1: return validateUsername(form.username);
        case 2: return validatePassword(form.password);
        case 3: return validateFullName(form.fullName);
        case 4: return validateBirthday(form.birthday);
        case 5: return validateGender(form.gender);
        case 6: return validateBio(form.bio);
    }

    return "";
}
// Tạo ID mới: lấy số lớn nhất trong các thư mục ACC<số> rồi cộng 1
// Ví dụ: ACC000001, ACC000002, ...
string generateAccountId() {
    fs::path accountDir = "Data/Account";
    error_code ec;
    unsigned long long maxNumber = 0;

    for (const auto& entry : fs::directory_iterator(accountDir, ec)) {

        if (!entry.is_directory(ec)) {
            continue;
        }

        string dirName = entry.path().filename().string();

        if (dirName.size() < 4 || dirName.substr(0, 3) != "ACC") {
            continue;
        }

        string digits = dirName.substr(3);

        bool isValid = all_of(digits.begin(), digits.end(),
                              [](unsigned char c) { return c >= '0' && c <= '9'; });

        // Bỏ qua tên quá dài để tránh tràn số
        if (!isValid || digits.size() > 18) {
            continue;
        }

        unsigned long long number = stoull(digits);

        if (number > maxNumber) {
            maxNumber = number;
        }
    }

    // Đệm số 0 phía trước cho đủ 6 chữ số
    string number = to_string(maxNumber + 1);

    if (number.length() < 6) {
        number = string(6 - number.length(), '0') + number;
    }

    return "ACC" + number;
}
// Tạo thư mục Data/Account/<ID>/ và ghi file profile.txt
bool createAccount(const RegisterForm& form, string& accountId) {
    accountId = generateAccountId();

    fs::path accountDir = fs::path("Data/Account") / accountId;
    error_code ec;

    if (!fs::create_directories(accountDir, ec) || ec) {
        return false;
    }

    ofstream file(accountDir / "profile.txt");

    if (!file.is_open()) {
        fs::remove_all(accountDir, ec);
        return false;
    }

    file << "id=" << accountId << '\n'
         << "username=" << form.username << '\n'
         << "password=" << stringToHex(form.password) << '\n'
         << "full_name=" << trimString(form.fullName) << '\n'
         << "birthday=" << form.birthday << '\n'
         << "gender=" << form.gender << '\n'
         << "bio=" << trimString(form.bio) << '\n'
         << "created_at=" << todayString() << '\n'
         << "status=ACTIVE" << '\n';

    file.close();

    // Nếu ghi lỗi thì xóa thư mục vừa tạo để không để lại tài khoản hỏng
    if (file.fail()) {
        fs::remove_all(accountDir, ec);
        return false;
    }

    return true;
}
// Lưu trạng thái đăng nhập: ghi username vào Data/System/isLogin.txt (1 dòng)
bool saveLoginState(const string& username) {
    ofstream file("Data/System/isLogin.txt", ios::trunc);

    if (!file.is_open()) {
        return false;
    }

    file << username << '\n';
    return !file.fail();
}
// Xóa trạng thái đăng nhập (dùng khi đăng xuất)
void clearLoginState() {
    error_code ec;
    fs::remove("Data/System/isLogin.txt", ec);
}

// Module: Báo cáo người dùng và bài viết
#pragma once

/*
    BÁO CÁO VI PHẠM (Data/Report/reports.txt)
    Mỗi dòng: LOẠI|NGƯỜI BÁO CÁO|ĐỐI TƯỢNG|THỜI GIAN|TRẠNG THÁI|LÝ DO
        TRẠNG THÁI = PENDING (chưa xử lý) hoặc DONE (đã xử lý). File cũ không có cột này vẫn đọc được (coi là chưa xử lý)
        LOẠI = USER: báo cáo người dùng (ĐỐI TƯỢNG là ID tài khoản, ví dụ ACC000002)
        LOẠI = POST: báo cáo bài viết   (ĐỐI TƯỢNG là ID bài viết, ví dụ POST000002)
    Quản trị viên xem các báo cáo này trong menu quản trị.
                                                                                    */

const fs::path REPORT_FILE = "Data/Report/reports.txt";
const string REPORT_USER = "USER";
const string REPORT_POST = "POST";

struct Report {
    string type;        // REPORT_USER hoặc REPORT_POST
    string reporterId;  // Người gửi báo cáo
    string targetId;    // Người dùng / bài viết bị báo cáo
    string time;        // YYYY-MM-DD HH:MM:SS
    string reason;      // Lý do
    bool handled = false;   // true: quản trị viên đã xử lý (DONE), false: chưa xử lý (PENDING)
};

// Đọc toàn bộ báo cáo (theo thứ tự đã gửi, cũ nhất ở đầu)
vector<Report> readReports() {
    vector<Report> reports;

    for (const string& line : readLines(REPORT_FILE)) {
        // Cắt theo 4 dấu '|' đầu tiên, phần còn lại (lý do) có thể chứa '|'
        vector<size_t> bars;
        size_t pos = 0;

        while (bars.size() < 4) {
            pos = line.find('|', pos);

            if (pos == string::npos) {
                break;
            }

            bars.push_back(pos);
            pos++;
        }

        if (bars.size() < 4) {
            continue;
        }

        Report report;
        report.type = line.substr(0, bars[0]);
        report.reporterId = line.substr(bars[0] + 1, bars[1] - bars[0] - 1);
        report.targetId = line.substr(bars[1] + 1, bars[2] - bars[1] - 1);
        report.time = line.substr(bars[2] + 1, bars[3] - bars[2] - 1);

        // Cột trạng thái (file cũ không có thì phần còn lại chính là lý do)
        string rest = line.substr(bars[3] + 1);
        size_t statusEnd = rest.find('|');

        if (statusEnd != string::npos) {
            string status = rest.substr(0, statusEnd);

            if (status == "PENDING" || status == "DONE") {
                report.handled = (status == "DONE");
                rest = rest.substr(statusEnd + 1);
            }
        }

        report.reason = unescapeText(rest);

        if (report.type != REPORT_USER && report.type != REPORT_POST) {
            continue;
        }

        reports.push_back(report);
    }

    return reports;
}
// Ghi đè toàn bộ báo cáo
bool writeReports(const vector<Report>& reports) {
    error_code ec;
    fs::create_directories(REPORT_FILE.parent_path(), ec);

    vector<string> lines;

    for (const Report& report : reports) {
        lines.push_back(report.type + "|" + report.reporterId + "|" + report.targetId + "|"
                        + report.time + "|" + (report.handled ? "DONE" : "PENDING") + "|"
                        + escapeText(report.reason));
    }

    return writeLines(REPORT_FILE, lines);
}
// Người này đã báo cáo đối tượng này chưa
bool hasReported(const string& type, const string& reporterId, const string& targetId) {
    for (const Report& report : readReports()) {
        if (report.type == type && report.reporterId == reporterId && report.targetId == targetId) {
            return true;
        }
    }

    return false;
}
// Gửi 1 báo cáo mới
bool addReport(const string& type, const string& reporterId, const string& targetId,
               const string& reason) {
    return appendLine(REPORT_FILE,
                      type + "|" + reporterId + "|" + targetId + "|"
                      + nowTimestamp() + "|PENDING|" + escapeText(reason));
}
// Đánh dấu 1 báo cáo là đã xử lý / chưa xử lý
bool setReportHandled(const Report& target, bool handled) {
    vector<Report> reports = readReports();

    for (Report& r : reports) {
        if (r.type == target.type && r.reporterId == target.reporterId &&
            r.targetId == target.targetId && r.time == target.time &&
            r.reason == target.reason) {
            r.handled = handled;
            return writeReports(reports);
        }
    }

    return false;
}
// Số báo cáo chưa xử lý
int countPendingReports() {
    int count = 0;

    for (const Report& report : readReports()) {
        if (!report.handled) {
            count++;
        }
    }

    return count;
}
// Xóa 1 báo cáo cụ thể (dùng khi quản trị viên đã xử lý xong)
bool removeReport(const Report& target) {
    vector<Report> reports = readReports();

    for (size_t i = 0; i < reports.size(); i++) {
        const Report& r = reports[i];

        if (r.type == target.type && r.reporterId == target.reporterId &&
            r.targetId == target.targetId && r.time == target.time &&
            r.reason == target.reason) {
            reports.erase(reports.begin() + i);
            return writeReports(reports);
        }
    }

    return false;
}
// Xóa mọi báo cáo về 1 đối tượng (tài khoản hoặc bài viết đã bị xóa)
void removeReportsAbout(const string& targetId) {
    vector<Report> reports = readReports();
    size_t before = reports.size();

    reports.erase(remove_if(reports.begin(), reports.end(),
                            [&](const Report& report) { return report.targetId == targetId; }),
                  reports.end());

    if (reports.size() != before) {
        writeReports(reports);
    }
}
// Xóa mọi báo cáo do 1 tài khoản gửi (tài khoản đã bị xóa)
void removeReportsBy(const string& reporterId) {
    vector<Report> reports = readReports();
    size_t before = reports.size();

    reports.erase(remove_if(reports.begin(), reports.end(),
                            [&](const Report& report) { return report.reporterId == reporterId; }),
                  reports.end());

    if (reports.size() != before) {
        writeReports(reports);
    }
}

/*
    NGƯỜI DÙNG GỬI BÁO CÁO
                            */

// Nhập lý do báo cáo. Trả về false nếu nhấn ESC
bool inputReportReason(const string& title, const string& prompt, string& reason) {
    InputOptions options;
    options.title = title;
    options.prompt = prompt;
    options.maxLength = 200;
    options.validate = [](const string& s) -> string {
        return s.empty() ? "Lý do báo cáo không được để trống!" : "";
    };

    return inputText(options, reason);
}
// Báo cáo 1 người dùng với lý do do người báo cáo nhập
void reportUserFlow(const string& myId, const string& targetId) {
    if (myId == targetId) {
        notify("Bạn không thể tự báo cáo chính mình!");
        return;
    }

    if (hasReported(REPORT_USER, myId, targetId)) {
        notify("Bạn đã báo cáo người này rồi, quản trị viên sẽ xem xét!");
        return;
    }

    string reason;

    if (!inputReportReason("=============== BÁO CÁO NGƯỜI DÙNG ===============",
                           "Nhập lý do báo cáo " + getDisplayName(targetId) + " (tối đa 200 ký tự)",
                           reason)) {
        return;
    }

    if (addReport(REPORT_USER, myId, targetId, reason)) {
        notify("Đã gửi báo cáo đến quản trị viên!");
    }
    else {
        notify("Không thể gửi báo cáo, vui lòng thử lại!");
    }
}
// Báo cáo 1 bài viết với lý do do người báo cáo nhập
void reportPostFlow(const string& myId, const string& postId) {
    if (hasReported(REPORT_POST, myId, postId)) {
        notify("Bạn đã báo cáo bài viết này rồi, quản trị viên sẽ xem xét!");
        return;
    }

    string reason;

    if (!inputReportReason("=============== BÁO CÁO BÀI VIẾT ===============",
                           "Nhập lý do báo cáo bài viết này (tối đa 200 ký tự)",
                           reason)) {
        return;
    }

    if (addReport(REPORT_POST, myId, postId, reason)) {
        notify("Đã gửi báo cáo đến quản trị viên!");
    }
    else {
        notify("Không thể gửi báo cáo, vui lòng thử lại!");
    }
}

/*
    BÁO CÁO SỰ CỐ TÀI KHOẢN (Data/Report/account_issues.txt)
    Người dùng đăng nhập sai nhiều lần có thể gửi báo cáo xin cấp lại mật khẩu cho quản trị viên.
    Mỗi dòng: ID TÀI KHOẢN|THỜI GIAN|TRẠNG THÁI (PENDING / DONE)|NỘI DUNG. Mỗi tài khoản chỉ giữ 1 báo cáo
    (gửi lại thì thay mới và về chưa xử lý). File cũ không có cột trạng thái vẫn đọc được.
    Quản trị viên mở báo cáo để đặt lại mật khẩu, khóa / mở khóa tài khoản...
                                                                                        */

const fs::path ISSUE_FILE = "Data/Report/account_issues.txt";

struct AccountIssue {
    string accountId;
    string time;
    string message;
    bool handled = false;   // true: đã xử lý (DONE)
};

vector<AccountIssue> readAccountIssues() {
    vector<AccountIssue> list;

    for (const string& line : readLines(ISSUE_FILE)) {
        size_t first = line.find('|');
        size_t second = (first == string::npos) ? string::npos : line.find('|', first + 1);

        if (second == string::npos) {
            continue;
        }

        AccountIssue issue;
        issue.accountId = line.substr(0, first);
        issue.time = line.substr(first + 1, second - first - 1);

        // Cột trạng thái (file cũ không có thì phần còn lại chính là nội dung)
        string rest = line.substr(second + 1);
        size_t statusEnd = rest.find('|');

        if (statusEnd != string::npos) {
            string status = rest.substr(0, statusEnd);

            if (status == "PENDING" || status == "DONE") {
                issue.handled = (status == "DONE");
                rest = rest.substr(statusEnd + 1);
            }
        }

        issue.message = unescapeText(rest);
        list.push_back(issue);
    }

    return list;
}
bool writeAccountIssues(const vector<AccountIssue>& list) {
    error_code ec;
    fs::create_directories(ISSUE_FILE.parent_path(), ec);

    vector<string> lines;

    for (const AccountIssue& issue : list) {
        lines.push_back(issue.accountId + "|" + issue.time + "|" + (issue.handled ? "DONE" : "PENDING")
                        + "|" + escapeText(issue.message));
    }

    return writeLines(ISSUE_FILE, lines);
}
// Xóa báo cáo sự cố của 1 tài khoản (đã xử lý xong, hoặc tài khoản đã bị xóa)
void removeAccountIssue(const string& accountId) {
    vector<AccountIssue> list = readAccountIssues();
    size_t before = list.size();

    list.erase(remove_if(list.begin(), list.end(),
                         [&](const AccountIssue& issue) { return issue.accountId == accountId; }),
               list.end());

    if (list.size() != before) {
        writeAccountIssues(list);
    }
}
// Đánh dấu báo cáo sự cố của 1 tài khoản là đã xử lý / chưa xử lý
bool setAccountIssueHandled(const string& accountId, bool handled) {
    vector<AccountIssue> list = readAccountIssues();

    for (AccountIssue& issue : list) {
        if (issue.accountId == accountId) {
            issue.handled = handled;
            return writeAccountIssues(list);
        }
    }

    return false;
}
// Số báo cáo sự cố chưa xử lý
int countPendingIssues() {
    int count = 0;

    for (const AccountIssue& issue : readAccountIssues()) {
        if (!issue.handled) {
            count++;
        }
    }

    return count;
}
// Gửi báo cáo sự cố (thay thế báo cáo cũ của cùng tài khoản nếu có)
bool addAccountIssue(const string& accountId, const string& message) {
    vector<AccountIssue> list = readAccountIssues();

    list.erase(remove_if(list.begin(), list.end(),
                         [&](const AccountIssue& issue) { return issue.accountId == accountId; }),
               list.end());

    AccountIssue issue;
    issue.accountId = accountId;
    issue.time = nowTimestamp();
    issue.message = message;
    list.push_back(issue);

    return writeAccountIssues(list);
}
// Từ màn hình đăng nhập: nhập báo cáo xin cấp lại mật khẩu gửi quản trị viên.
// typedUsername là tên tài khoản đã gõ ở màn hình đăng nhập (không có hoặc không tồn tại thì hỏi lại)
void forgotPasswordFlow(const string& typedUsername) {
    string username = typedUsername;
    string accountId = getAccountId(username);

    if (accountId.empty()) {
        InputOptions options;
        options.title = "=============== QUÊN MẬT KHẨU ===============";
        options.prompt = "Nhập tên tài khoản cần cấp lại mật khẩu";
        options.maxLength = 15;
        options.validate = [](const string& s) -> string {
            return getAccountId(s).empty() ? "Không tìm thấy tài khoản này!" : "";
        };

        username.clear();

        if (!inputText(options, username)) {
            return;
        }

        accountId = getAccountId(username);

        if (accountId.empty()) {
            return;
        }
    }

    InputOptions options;
    options.title = "=============== QUÊN MẬT KHẨU ===============";
    options.prompt = "Báo cáo xin cấp lại mật khẩu cho \"" + username
                   + "\" (ghi họ tên, ngày sinh... để quản trị viên xác minh, tối đa 200 ký tự)";
    options.maxLength = 200;
    options.validate = [](const string& s) -> string {
        return s.empty() ? "Nội dung báo cáo không được để trống!" : "";
    };

    string message;

    if (!inputText(options, message)) {
        return;
    }

    if (addAccountIssue(accountId, message)) {
        notify("Đã gửi báo cáo đến quản trị viên! Hãy liên hệ quản trị viên để nhận mật khẩu mới.");
    }
    else {
        notify("Không thể gửi báo cáo, vui lòng thử lại!");
    }
}

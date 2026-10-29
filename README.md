# MangXaHoiMini-GoiYKetBan-DSA

# English

## About the project

**MangXaHoiMini** ("Mini Social Network") is a Windows console social network written in **C++17**. It was built as a Data Structures & Algorithms (DSA) group project, with a focus on a **friend suggestion** feature that walks the friend graph using **Breadth-First Search (BFS)**.

All data is stored in plain `.txt` files under the `Data/` folder, so no database is needed. The interface is fully keyboard- and mouse-wheel-driven (arrow keys, Enter, Esc, scrolling) and supports Vietnamese with diacritics.

## Team members

| No. | Full name | Student ID |
|:---:|-----------|:----------:|
| 1 | Bùi Thanh Thảo | 6651071070 |
| 2 | Nguyễn Tấn Tài | 6651071062 |
| 3 | Nguyễn Thị Huyền Trân | 6651071080 |
| 4 | Nguyễn Văn Tuấn Khang | 6651071031 |
| 5 | Nguyễn Thị Hồng Nhung | 6651071050 |

## Main features

### Accounts and profiles
- Register and log in (passwords are stored hex-encoded). After a successful registration the user is logged in directly.
- Profile fields: full name, birthday (`dd-mm-yyyy`, validated including leap years), gender, bio, current residence, hometown, school, workplace and hobbies.
- Residence, hometown, school and hobbies are chosen from built-in Vietnamese lists (provinces, universities/colleges, hobbies) with live, accent-insensitive search.
- View your own profile and other people's profiles; edit your account in the settings menu.

### Friends
- Send, accept and cancel friend requests, unfriend, and block / unblock users.
- **Friend suggestions (BFS):** every account is a vertex and every `FRIEND` relation is an edge. Starting from user A, a BFS goes up to 3 levels deep (friend of a friend, friend of a friend of a friend). Closer people are ranked first; within the same level, people with more mutual friends come first. Suggestions also show a reason (same hometown, same school, shared hobbies, ...). If a user has no friends yet, a few popular users are suggested instead.
- A `Chung Nhóm Chat` (shared chat groups) suggestion list shows people who are in the same chat groups but are not yet friends.
- **Friend search:** type to search by name or username (accent- and case-insensitive). Press `TAB` to open or close the filter panel; filters are combined with AND, except hobbies, which match when at least one hobby is shared.

### Posts
- Create, edit and delete posts with three visibility levels: **Public**, **Friends** and **Only me**.
- Like and comment on posts. Time is shown as "x minutes / days / months ago", and as `dd/mm/yyyy` after more than one year.
- Long content uses a scrollable reading screen with a fixed title and a fixed guide bar.

### Messaging
- Private chats with timestamps.
- **Group chats:** a new member must be a friend of the person who adds them.
- **Nicknames:** press `F2` in a private chat to set or remove a nickname (saved per account, so each person can use a different one).
- **Delete messages on your side:** press `Delete` to enter selection mode, use `↑`/`↓` or the mouse wheel to move, `Enter` to mark/unmark, and press `Delete` again to confirm. Only the current account hides the selected messages.
- **Recall messages:** press `/` while the input box is empty to enter selection mode (same controls as delete) and press `/` again to recall. Only your own messages sent less than 10 minutes ago can be recalled; both sides then see `Bạn đã thu hồi tin nhắn`.
- `Esc` cancels selection mode without doing anything.

### Notifications and reports
- Notifications for friend requests, accepted requests, post likes, post comments and new messages.
- Users can report other users or posts.

### Administrator
- Sign in on the normal login screen with the admin account (see `TaiKhoan_MatKhau.txt`; the credentials are defined in `Modules/AccountCore.h`).
- Manage accounts (view, search, lock, unlock, delete), user and post reports, password-reset requests, posts (view, hide/show, delete) and chat groups (message content is not visible, to protect privacy), and view system statistics.

## Project structure

```text
MangXaHoiMini-GoiYKetBan-DSA/
├── main.cpp                  # Entry point: creates App and calls run()
├── README.md
├── TaiKhoan_MatKhau.txt      # Test accounts and passwords for the sample data
├── Modules/                  # All modules (header-only, included from App.h)
│   ├── Console.h  Text.h  Utils.h        # Console, key input, double buffer, UTF-8 / time / file helpers
│   ├── UI.h  Reader.h  Cards.h  Picker.h # Menus, scrolling reader, card lists, searchable pickers
│   ├── AccountCore.h  AuthUI.h           # Account validation, login / register screens
│   ├── Profile.h  ProfileScreens.h  Settings.h
│   ├── Friend.h  FriendMenus.h  Suggest.h  # Friend relations, search, BFS suggestions
│   ├── Post.h  Chat.h  Group.h           # Posts, private chats, group chats
│   ├── Notification.h  NotificationScreens.h
│   ├── Report.h  Admin.h                 # Reports and administrator tools
│   ├── MenuUser.h  App.h                 # User menu and main application class
│   └── VietnamData.h                     # Provinces, schools and hobbies lists
└── Data/                     # Plain-text storage (read and written at run time)
    ├── Account/ACCxxxxxx/    # profile.txt, friend.txt, notifications.txt, nicknames.txt
    ├── Post/POSTxxxxxx/      # post.txt, likes.txt, comments.txt
    ├── Conversations/        # .Private/ (1-1 chats), .Group/GRPxxxxxx/, .Hidden/
    └── Report/               # reports.txt, account_issues.txt
```

## Build and run

**Requirements:** Windows, a C++17 compiler (for example MSYS2 `g++`), and a console that can display Vietnamese (UTF-8). The program uses the Windows API (`<windows.h>`), so it does not build on Linux or macOS.

```bash
g++ -std=c++17 -O2 main.cpp -o MangXaHoiMini.exe
MangXaHoiMini.exe
```

Run the program **from the project root folder**, because it reads and writes `Data/` using relative paths.

## Data notes

- Hidden-message state: `Data/Conversations/.Hidden/<accountId>.txt`.
- Nicknames: `Data/Account/<accountId>/nicknames.txt`.
- Recalling a message updates the record inside the shared conversation file.
- Friend relations (`friend.txt`): one line per relation, `ACCID|STATUS|YYYY-MM-DD`, where `STATUS` is `FRIEND`, `PENDING_SENT`, `PENDING_RECEIVED` or `BLOCK`.
- The old message format `sender|timestamp|text` is still readable.

---

# Tiếng Việt

## Giới thiệu

**MangXaHoiMini** (Mạng xã hội mini) là ứng dụng mạng xã hội chạy trên console Windows, viết bằng **C++17**. Đây là bài tập nhóm môn Cấu trúc dữ liệu và giải thuật (DSA), trọng tâm là chức năng **gợi ý kết bạn** bằng cách duyệt đồ thị bạn bè theo thuật toán **BFS (duyệt theo chiều rộng)**.

Toàn bộ dữ liệu được lưu bằng các file `.txt` trong thư mục `Data/`, không cần cơ sở dữ liệu. Giao diện thao tác hoàn toàn bằng bàn phím và con lăn chuột (phím mũi tên, Enter, Esc, cuộn) và hiển thị được tiếng Việt có dấu.

## Thành viên nhóm

| STT | Họ và tên | Mã số sinh viên |
|:---:|-----------|:---------------:|
| 1 | Bùi Thanh Thảo | 6651071070 |
| 2 | Nguyễn Tấn Tài | 6651071062 |
| 3 | Nguyễn Thị Huyền Trân | 6651071080 |
| 4 | Nguyễn Văn Tuấn Khang | 6651071031 |
| 5 | Nguyễn Thị Hồng Nhung | 6651071050 |

## Các tính năng chính

### Tài khoản và trang cá nhân
- Đăng ký, đăng nhập (mật khẩu được lưu dưới dạng mã hóa Hex). Đăng ký thành công sẽ được đăng nhập luôn.
- Thông tin cá nhân gồm: họ tên, ngày sinh (`dd-mm-yyyy`, kiểm tra hợp lệ kể cả năm nhuận), giới tính, giới thiệu, nơi ở hiện tại, quê quán, trường học, nơi làm việc và sở thích.
- Nơi ở, quê quán, trường học và sở thích được chọn từ danh sách có sẵn (tỉnh thành, trường đại học / cao đẳng, sở thích) và có tìm kiếm trực tiếp, không phân biệt hoa thường và dấu.
- Xem trang cá nhân của mình và của người khác; chỉnh sửa tài khoản trong mục Cài Đặt.

### Bạn bè
- Gửi, chấp nhận, hủy lời mời kết bạn; hủy kết bạn; chặn và bỏ chặn.
- **Gợi ý kết bạn (BFS):** mỗi tài khoản là một đỉnh, mỗi quan hệ bạn bè (`FRIEND`) là một cạnh. Từ người dùng A, thuật toán BFS đi tối đa 3 tầng (bạn của bạn, bạn của bạn của bạn). Người gần hơn được xếp trước; cùng tầng thì ai có nhiều bạn chung hơn xếp trước. Mỗi gợi ý kèm lý do (cùng quê, cùng trường, chung sở thích...). Nếu chưa có bạn nào, chương trình chỉ gợi ý một vài người dùng nổi bật.
- Mục `Chung Nhóm Chat` liệt kê những người chưa kết bạn nhưng cùng tham gia nhóm chat, ưu tiên người có nhiều nhóm chung.
- **Tìm kiếm bạn bè:** gõ tới đâu tìm tới đó theo tên hoặc tài khoản (không phân biệt hoa thường và dấu). Nhấn `TAB` để mở/đóng bộ lọc; các tiêu chí kết hợp theo điều kiện AND, riêng sở thích khớp khi có ít nhất một sở thích chung.

### Bài đăng
- Tạo, chỉnh sửa, xóa bài đăng với ba chế độ hiển thị: **Công khai**, **Bạn bè**, **Chỉ mình tôi**.
- Thích và bình luận bài đăng. Thời gian hiển thị dạng "x phút / ngày / tháng trước", quá 1 năm thì hiển thị `dd/mm/yyyy`.
- Nội dung dài dùng màn hình đọc có cuộn, tiêu đề và thanh hướng dẫn luôn cố định.

### Nhắn tin
- Nhắn tin riêng có hiển thị thời gian.
- **Nhóm chat:** chỉ thêm được người là bạn bè của người thêm.
- **Biệt danh:** trong cuộc trò chuyện riêng, nhấn `F2` để đặt hoặc bỏ biệt danh cho người kia. Biệt danh được lưu riêng theo tài khoản nên mỗi người có thể đặt tên khác nhau.
- **Xóa tin nhắn phía mình:** nhấn `Delete` để vào chế độ chọn, dùng `↑`/`↓` hoặc con lăn để di chuyển, `Enter` để đánh dấu/bỏ đánh dấu, nhấn `Delete` lần nữa để thực hiện. Chỉ tài khoản hiện tại ẩn các tin đã chọn.
- **Thu hồi tin nhắn:** nhấn `/` khi ô nhập đang trống để vào chế độ chọn (thao tác giống xóa), nhấn `/` lần nữa để thu hồi. Chỉ tin do bạn gửi và chưa quá 10 phút mới được thu hồi; nội dung được thay bằng thông báo `Bạn đã thu hồi tin nhắn` ở cả hai phía.
- Trong chế độ chọn, `ESC` hủy chế độ mà không thực hiện thao tác.

### Thông báo và báo cáo
- Thông báo khi có lời mời kết bạn, lời mời được chấp nhận, bài đăng được thích, bài đăng có bình luận và có tin nhắn mới.
- Người dùng có thể báo cáo người dùng khác hoặc bài viết.

### Quản trị viên
- Đăng nhập ở màn hình đăng nhập thông thường bằng tài khoản quản trị (xem file `TaiKhoan_MatKhau.txt`; thông tin được khai báo trong `Modules/AccountCore.h`).
- Quản lý tài khoản (xem, tìm, khóa, mở khóa, xóa), báo cáo người dùng và bài viết, yêu cầu cấp lại mật khẩu, bài viết (xem, ẩn/hiện, xóa), nhóm chat (không xem được nội dung tin nhắn để đảm bảo riêng tư) và xem thống kê hệ thống.

## Cấu trúc dự án

```text
MangXaHoiMini-GoiYKetBan-DSA/
├── main.cpp                  # Điểm vào: tạo đối tượng App và gọi run()
├── README.md
├── TaiKhoan_MatKhau.txt      # Danh sách tài khoản và mật khẩu của bộ dữ liệu mẫu
├── Modules/                  # Các module (chỉ có file .h, được include từ App.h)
│   ├── Console.h  Text.h  Utils.h        # Console, nhận phím, double buffer, tiện ích UTF-8 / thời gian / file
│   ├── UI.h  Reader.h  Cards.h  Picker.h # Menu, màn hình đọc có cuộn, danh sách thẻ, bộ chọn có tìm kiếm
│   ├── AccountCore.h  AuthUI.h           # Kiểm tra tài khoản, màn hình đăng nhập / đăng ký
│   ├── Profile.h  ProfileScreens.h  Settings.h
│   ├── Friend.h  FriendMenus.h  Suggest.h  # Quan hệ bạn bè, tìm kiếm, gợi ý kết bạn bằng BFS
│   ├── Post.h  Chat.h  Group.h           # Bài đăng, tin nhắn riêng, nhóm chat
│   ├── Notification.h  NotificationScreens.h
│   ├── Report.h  Admin.h                 # Báo cáo và công cụ quản trị
│   ├── MenuUser.h  App.h                 # Menu người dùng và lớp ứng dụng chính
│   └── VietnamData.h                     # Danh sách tỉnh thành, trường học, sở thích
└── Data/                     # Dữ liệu dạng văn bản (đọc và ghi khi chạy chương trình)
    ├── Account/ACCxxxxxx/    # profile.txt, friend.txt, notifications.txt, nicknames.txt
    ├── Post/POSTxxxxxx/      # post.txt, likes.txt, comments.txt
    ├── Conversations/        # .Private/ (chat riêng), .Group/GRPxxxxxx/, .Hidden/
    └── Report/               # reports.txt, account_issues.txt
```

## Cách build và chạy

**Yêu cầu:** Windows, trình biên dịch hỗ trợ C++17 (ví dụ `g++` của MSYS2) và console hiển thị được tiếng Việt (UTF-8). Chương trình dùng Windows API (`<windows.h>`) nên không build được trên Linux hoặc macOS.

```bash
g++ -std=c++17 -O2 main.cpp -o MangXaHoiMini.exe
MangXaHoiMini.exe
```

Hãy chạy chương trình **từ thư mục gốc của dự án**, vì chương trình đọc và ghi thư mục `Data/` theo đường dẫn tương đối.

## Ghi chú dữ liệu

- Trạng thái xóa tin nhắn phía người dùng lưu ở `Data/Conversations/.Hidden/<accountId>.txt`.
- Biệt danh lưu ở `Data/Account/<accountId>/nicknames.txt`.
- Thu hồi tin nhắn cập nhật bản ghi trong file hội thoại dùng chung.
- Quan hệ bạn bè (`friend.txt`): mỗi dòng một quan hệ theo dạng `ACCID|STATUS|YYYY-MM-DD`, với `STATUS` là `FRIEND`, `PENDING_SENT`, `PENDING_RECEIVED` hoặc `BLOCK`.
- Định dạng bản ghi tin nhắn cũ `sender|timestamp|text` vẫn được đọc.

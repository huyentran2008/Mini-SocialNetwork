# Mini Social Network — C++

## 1. Giới thiệu dự án

Mini Social Network là dự án xây dựng một hệ thống mạng xã hội mini bằng ngôn ngữ C++, phục vụ mục đích học tập và thực hành các kiến thức về Lập trình hướng đối tượng, Cấu trúc dữ liệu và Giải thuật.

Hệ thống mô phỏng các chức năng cơ bản của một mạng xã hội, bao gồm quản lý người dùng, kết bạn, tìm kiếm và gợi ý bạn bè, phân tích mối quan hệ trong mạng lưới và tương tác với người dùng thông qua giao diện console.

## 2. Mục tiêu dự án

- Vận dụng kiến thức C++ và lập trình hướng đối tượng vào một dự án thực tế.
- Áp dụng các cấu trúc dữ liệu như Hash Table, Graph, Queue và Stack.
- Triển khai các thuật toán BFS, DFS, DSU và Tarjan.
- Phân chia hệ thống thành các module độc lập để thuận tiện phát triển, kiểm thử và tích hợp.
- Sử dụng Git và GitHub để quản lý mã nguồn và phối hợp làm việc nhóm.

## 3. Phân công chức năng và nhiệm vụ

### 3.1. Module Người dùng và Bảng băm

**Nhiệm vụ:** Quản lý thông tin người dùng và xây dựng cơ chế lưu trữ, tra cứu dữ liệu hiệu quả.

**Các thành phần phụ trách:**
- `User`: Đại diện cho thông tin người dùng.
- `UserManager`: Quản lý danh sách và các thao tác với người dùng.
- `Profile`: Quản lý hồ sơ cá nhân.
- `HashTable`: Cấu trúc bảng băm phục vụ lưu trữ và tra cứu.
- `AttributeIndex`: Hỗ trợ lập chỉ mục theo thuộc tính.

**Thuật toán và kỹ thuật:**
- Hash Table.
- Chaining (xử lý va chạm bằng danh sách liên kết).
- Rehash (xây dựng lại bảng băm khi cần thiết).

**Kết quả cần đạt:** Hỗ trợ thêm, tìm kiếm, cập nhật và quản lý người dùng; tổ chức dữ liệu để các module khác có thể sử dụng.

### 3.2. Module Đồ thị và Lời mời kết bạn

**Nhiệm vụ:** Mô hình hóa quan hệ bạn bè dưới dạng đồ thị và quản lý các lời mời kết bạn.

**Các thành phần phụ trách:**
- `Graph`: Biểu diễn mạng lưới quan hệ giữa người dùng.
- `FriendManager`: Quản lý quan hệ bạn bè.
- `FriendRequest`: Quản lý lời mời kết bạn.
- `PostManager::getFeed()`: Hỗ trợ lấy dữ liệu bài viết hiển thị trên bảng tin theo thiết kế hệ thống.

**Thuật toán và kỹ thuật:**
- Đồ thị danh sách kề (Adjacency List).

**Kết quả cần đạt:** Quản lý danh sách bạn bè, lời mời kết bạn và cung cấp dữ liệu quan hệ cho các module tìm kiếm, gợi ý và phân tích mạng lưới.

### 3.3. Module Tìm kiếm và Gợi ý kết bạn

**Nhiệm vụ:** Tìm kiếm người dùng và đề xuất những người có khả năng trở thành bạn bè dựa trên dữ liệu quan hệ.

**Các thành phần phụ trách:**
- `BFSAlgorithm`: Hỗ trợ duyệt đồ thị theo chiều rộng.
- `FriendSuggester`: Xây dựng chức năng gợi ý bạn bè.
- `SearchService`: Xử lý các yêu cầu tìm kiếm.
- `ConnectionAnalyzer`: Phân tích mức độ kết nối giữa người dùng.

**Thuật toán và kỹ thuật:**
- BFS (Breadth-First Search).
- Sorting (sắp xếp kết quả hoặc điểm gợi ý).

**Kết quả cần đạt:** Hỗ trợ tìm kiếm người dùng, duyệt quan hệ trong mạng lưới và đề xuất bạn bè theo tiêu chí được thống nhất trong thiết kế.

### 3.4. Module Liên thông và Thống kê

**Nhiệm vụ:** Phân tích cấu trúc mạng lưới xã hội, xác định các thành phần liên thông và các đặc điểm kết nối quan trọng.

**Các thành phần phụ trách:**
- `DSU`: Quản lý các tập hợp rời nhau.
- `DFSAlgorithm`: Duyệt đồ thị theo chiều sâu.
- `NetworkAnalyzer`: Phân tích cấu trúc mạng lưới.
- `TarjanAlgorithm`: Hỗ trợ phân tích các đặc điểm liên thông của đồ thị.

**Thuật toán và kỹ thuật:**
- DSU (Disjoint Set Union).
- DFS (Depth-First Search).
- Tarjan.

**Kết quả cần đạt:** Cung cấp các chức năng thống kê, phân tích thành phần liên thông và các thông tin cấu trúc mạng lưới theo yêu cầu của hệ thống.

### 3.5. Module UI và Các module hỗ trợ

**Nhiệm vụ:** Xây dựng giao diện console và tích hợp các module thành một chương trình hoàn chỉnh.

**Các thành phần phụ trách:**
- `Post`: Biểu diễn bài viết.
- `PostManager`: Quản lý bài viết.
- `PostSearchService`: Hỗ trợ tìm kiếm bài viết.
- `FileManager`: Quản lý thao tác đọc và ghi dữ liệu từ tệp.
- `Queue`: Cấu trúc dữ liệu hàng đợi.
- `Stack`: Cấu trúc dữ liệu ngăn xếp.

**Thuật toán và kỹ thuật:**
- Queue.
- Stack.
- I/O File.
- Console UI.

**Kết quả cần đạt:** Xây dựng menu tương tác, tiếp nhận dữ liệu đầu vào, hiển thị kết quả, hỗ trợ lưu trữ dữ liệu và tích hợp các chức năng của hệ thống.

## 4. Công nghệ và kiến thức sử dụng

- **Ngôn ngữ lập trình:** C++.
- **Quản lý mã nguồn:** Git và GitHub.
- **Lập trình:** Lập trình hướng đối tượng (OOP).
- **Cấu trúc dữ liệu:**
  - Hash Table (Bảng băm).
  - Graph (Đồ thị danh sách kề).
  - Queue (Hàng đợi).
  - Stack (Ngăn xếp).
  - DSU (Disjoint Set Union).
- **Thuật toán:**
  - BFS (Breadth-First Search).
  - DFS (Depth-First Search).
  - Tarjan Algorithm.
  - Sorting (Sắp xếp).
- **Quản lý dữ liệu:** File I/O (Đọc và ghi tệp).
- **Giao diện người dùng:** Console UI (Giao diện dòng lệnh).

## 5. Quy tắc phát triển và quản lý mã nguồn

Nhóm sử dụng GitHub để lưu trữ mã nguồn, theo dõi lịch sử thay đổi và phối hợp phát triển.

### Quy tắc làm việc

- Mỗi thành viên chịu trách nhiệm cho module được phân công.
- Thống nhất tên hàm, kiểu dữ liệu, tham số và giá trị trả về trước khi triển khai các module.
- Không tự ý sửa đổi giao diện giữa các module khi chưa trao đổi với thành viên liên quan.
- Mỗi thành viên cần kiểm thử chức năng của mình trước khi gửi code tích hợp.
- Các thay đổi quan trọng cần được kiểm tra trước khi đưa vào nhánh chính.

### Quy trình GitHub

1. Đồng bộ code mới nhất từ repository.
2. Tạo hoặc chuyển sang branch phụ trách.
3. Viết code và kiểm thử.
4. Commit các thay đổi với nội dung rõ ràng.
5. Push branch lên GitHub.
6. Tạo Pull Request để trưởng nhóm kiểm tra.
7. Merge vào `main` sau khi được chấp thuận.

### Quy ước đặt tên branch

- `main`: Phiên bản code chính.
- `feature/user-management`: Module người dùng.
- `feature/friend-graph`: Module đồ thị và kết bạn.
- `feature/friend-suggestion`: Module tìm kiếm và gợi ý.
- `feature/network-analysis`: Module liên thông và thống kê.
- `feature/console-ui`: Module giao diện và tích hợp.

## 6. Cấu trúc thư mục dự kiến

```text
Mini-SocialNetwork/
├── README.md
├── docs/
│   ├── phan-cong-chuc-nang.md
│   └── thiet-ke-he-thong.md
├── include/
│   ├── User.h
│   ├── UserManager.h
│   ├── Profile.h
│   ├── HashTable.h
│   ├── Graph.h
│   ├── FriendManager.h
│   ├── FriendRequest.h
│   ├── FriendSuggester.h
│   ├── NetworkAnalyzer.h
│   ├── Post.h
│   ├── PostManager.h
│   └── FileManager.h
├── src/
│   ├── main.cpp
│   ├── User.cpp
│   ├── UserManager.cpp
│   ├── Graph.cpp
│   ├── FriendManager.cpp
│   ├── FriendSuggester.cpp
│   ├── NetworkAnalyzer.cpp
│   ├── Post.cpp
│   ├── PostManager.cpp
│   └── FileManager.cpp
└── tests/
    └── README.md
```

*Lưu ý: Đây là cấu trúc dự kiến. Nhóm có thể điều chỉnh tên file, cách chia thư mục và số lượng module theo thiết kế thực tế. Không nhất thiết phải tạo tất cả các file ngay từ đầu.*

## 7. Kiểm thử và tích hợp

Các module cần được kiểm thử riêng trước khi tích hợp. Nhóm cần chú ý:

- Kiểm tra dữ liệu đầu vào không hợp lệ.
- Xử lý trường hợp người dùng hoặc bài viết không tồn tại.
- Kiểm tra dữ liệu trùng lặp và quan hệ bạn bè không hợp lệ.
- Kiểm tra các trường hợp đồ thị rỗng, đồ thị có một đỉnh hoặc nhiều thành phần liên thông.
- Kiểm tra thao tác đọc, ghi tệp và tình huống tệp không tồn tại.
- Kiểm tra sự tương thích giữa các module khi tích hợp.

## 8. Kết quả mong đợi

Hoàn thành một ứng dụng mạng xã hội mini chạy trên console, có khả năng quản lý người dùng, quan hệ bạn bè, bài viết và cung cấp các chức năng tìm kiếm, gợi ý, phân tích mạng lưới dựa trên các cấu trúc dữ liệu và thuật toán đã triển khai.

Dự án đồng thời giúp các thành viên rèn luyện kỹ năng lập trình C++, thiết kế module, kiểm thử phần mềm và làm việc nhóm thông qua GitHub.


# DT058 — Tạo mã Huffman từ bảng tần suất nhỏ (INT13185)

## Thông tin sinh viên và căn cứ đề tài

- **Họ tên:** Đặng Thế Phú — **MSSV:** `N24DCDT070` — **Nhóm:** `03`
- **Mã và tên đề tài:** `DT058` — Tạo mã Huffman từ bảng tần suất nhỏ
- **Giáo trình:** `Giao-trinh-Cau-truc-du-lieu-va-Giai-thuat-INT1306.pdf` — Chương 4, Mục 4.2.2 (Mã Huffman), Bài tập chương 4 câu 38 (trang in 93–94, 112; trang PDF 105–106, 124)
- **Ngôn ngữ & Trình biên dịch:** C++17 (`g++` MinGW-w64 / GCC)

## Cấu trúc thư mục

| Đường dẫn | Vai trò |
|---|---|
| `src/huffman.cpp` | Mã nguồn chính: tìm 2 nút nhỏ nhất, dựng cây Huffman, sinh mã nhị phân, tính tổng bit, kiểm tra tính không tiền tố, mã hóa/giải mã chuỗi thử và bộ kiểm thử 12 ca độc lập |
| `src/huffman.exe` | Tệp thực thi tương tác biên dịch từ `src/huffman.cpp` |
| `test/runtests.cpp` | Tệp chạy kiểm thử tự động toàn bộ 12 ca độc lập (không cần nhập menu) |
| `test/runtests.exe` | Tệp thực thi kiểm thử tự động biên dịch từ `test/runtests.cpp` |
| `docs/anh_ket_qua/` | Thư mục lưu 11 ảnh minh chứng thực thi `A01`–`A11` |

## Hợp đồng đầu vào / đầu ra

- **Đầu vào:**
  - Số lượng ký tự $m \in [1..8]$ (`int`).
  - Danh sách $m$ ký tự phân biệt không trùng lặp (`vector<char>`).
  - Danh sách $m$ tần suất nguyên dương $f[i] \in [1..100]$ (`vector<int>`).
  - Chuỗi thử tùy chọn chỉ gồm các ký tự có trong bảng (hoặc để trống).
  - Dữ liệu sai (ngoài miền $m \in [1..8]$, ký tự trùng, tần suất ngoài $[1..100]$) bị bộ kiểm tra từ chối và yêu cầu nhập lại.
- **Đầu ra:**
  - Bảng mã Huffman của từng ký tự (`Ký tự | Tần suất | Mã | Độ dài`).
  - Tổng số bit $\sum f[i] \times \text{len}(\text{ma}[i])$, tổng trọng số tại gốc, số phép so sánh trọng lượng và kết quả kiểm tra không tiền tố (`DUNG`/`SAI`).
  - Chuỗi bit mã hóa và chuỗi giải mã khôi phục (`KHOI PHUC DUNG`).

## Hướng dẫn biên dịch và chạy chương trình

Mở PowerShell tại thư mục gốc của kho mã:

```powershell
g++ --version
g++ -O2 -std=c++17 src/huffman.cpp -o src/huffman.exe
g++ -O2 -std=c++17 test/runtests.cpp -o test/runtests.exe
.\test\runtests.exe
```

Hoặc chạy tương tác từ bàn phím:

```powershell
.\src\huffman.exe
```
- Chọn `1`: Chạy tự động 5 ca trên phiếu (gồm 2 ví dụ đề bài) và bảng tổng kết **12/12 ca kiểm thử độc lập (38/38 phép kiểm assert PASS)**.
- Chọn `2`: Nhập bảng tần suất từ bàn phím.
- Chọn `0`: Thoát chương trình.

### Dữ liệu mẫu copy-paste nhanh (dùng khi chạy `.\src\huffman.exe`)

Khi đang ở dấu nhắc `Chon: ` của `.\src\huffman.exe`, có thể copy và dán trực tiếp từng khối dưới đây vào cửa sổ Terminal:

- **Ví dụ 1 đề bài (`A B C D`, tần suất `5 9 12 13` → Tổng bit = `78`):**
  ```text
  2
  4
  A B C D
  5 9 12 13

  ```
- **Ví dụ 2 đề bài (`A B`, tần suất `2 3`, chuỗi `"ABBA"` → Tổng bit = `5`, mã hóa `0110`):**
  ```text
  2
  2
  A B
  2 3
  ABBA
  ```
- **Ca tự thiết kế 1 (`A B C D`, tần suất `1 4 2 5`, chuỗi `"ACDB"` → Tổng bit = `22`, mã hóa `10010110`):**
  ```text
  2
  4
  A B C D
  1 4 2 5
  ACDB
  ```
- **Ca tự thiết kế 2 (`A B C D E`, tần suất `2 2 4 4 8`, chuỗi `"ABCDE"` → Tổng bit = `44`):**
  ```text
  2
  5
  A B C D E
  2 2 4 4 8
  ABCDE
  ```
- **Ca biên 1 ($m = 1$, ký tự `A`, tần suất `7`, chuỗi `"AAA"` → Mã `"0"`, tổng bit = `7`):**
  ```text
  2
  1
  A
  7
  AAA
  ```
- **Ca biên 2 ($m = 8$, ký tự `A..H`, tần suất bằng nhau `5` → Cây cân bằng độ dài `3`, tổng bit = `120`):**
  ```text
  2
  8
  A B C D E F G H
  5 5 5 5 5 5 5 5
  ABCDEFGH
  ```
- **Ca biên 3 ($f = 1$ nhỏ nhất, `A B C`, tần suất `1 1 1`, chuỗi `"ABC"` → Tổng bit = `5`):**
  ```text
  2
  3
  A B C
  1 1 1
  ABC
  ```
- **Ca kiểm tra lọc dữ liệu lỗi ($m=0$, $m=9$, ký tự trùng `A A B`, tần suất `1 120 5` ngoài miền `1..100`, sau đó nhập đúng):**
  ```text
  2
  0
  9
  3
  A A B
  A B C
  1 120 5
  2 3 5
  ABC
  ```

## Quy trình Clone sạch và chạy lại (A10)

```powershell
git clone https://github.com/Enderghas999/N24DCDT070_DT058_Huffman.git kiem_tra_ban_nop_N24DCDT070
cd kiem_tra_ban_nop_N24DCDT070
git rev-parse HEAD
g++ --version
g++ -O2 -std=c++17 test/runtests.cpp -o test/runtests.exe
.\test\runtests.exe
```

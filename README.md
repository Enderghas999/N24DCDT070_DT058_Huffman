# DT058 — Tạo mã Huffman từ bảng tần suất nhỏ (INT13185)

Chương trình C++ dựng mã Huffman cho một bảng gồm tối đa 8 ký tự, mỗi ký tự có tần suất nguyên trong khoảng 1..100. Kết quả gồm bảng mã, tổng số bit và kết quả kiểm tra tính không tiền tố. Chương trình có thể mã hóa rồi giải mã một chuỗi thử. Hai nút nhẹ nhất ở mỗi bước được tìm bằng cách quét tuyến tính danh sách, không dùng heap.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue) ![Compiler](https://img.shields.io/badge/Compiler-g%2B%2B%20(MinGW--w64%20%2F%20GCC)-lightgrey) ![Tests](https://img.shields.io/badge/Ki%E1%BB%83m%20th%E1%BB%AD-12%2F12%20ca-green)

---

## 1. Thông tin và căn cứ đề tài

| Mục | Nội dung |
|---|---|
| Sinh viên | Đặng Thế Phú — MSSV `N24DCDT070` — Lớp `D24CQDT01-N` — Nhóm `03` |
| Đề tài | `DT058` — Tạo mã Huffman từ bảng tần suất nhỏ |
| Giáo trình | `Giao-trinh-Cau-truc-du-lieu-va-Giai-thuat-INT1306.pdf`, Chương 4 «Các mô hình thuật toán» |
| Bài tập gốc | Bài tập chương 4, **câu 38** «Cài đặt mã Huffman», trang in 112, trang PDF 124 |
| Nền tảng lý thuyết | Mục 4.2 «Mô hình tham lam», 4.2.1 «Sơ đồ chung», trang in 93, trang PDF 105 |
| Ngôn ngữ | C++17, biên dịch bằng `g++` |

### Phạm vi

Câu 38 trong giáo trình có hai phần: (a) cài đặt mã Huffman bằng cách ghép tham lam hai ký tự có tần suất thấp nhất; (b) so sánh kích thước tệp nén với tệp gốc trên một văn bản tiếng Việt. Đề tài DT058 chỉ làm phần (a) trên một **bảng tần suất cho sẵn**:

- **Có:** dựng cây, sinh mã, tính tổng bit, kiểm tra không tiền tố, mã hóa và giải mã một chuỗi thử ngắn.
- **Không có:** đọc tệp văn bản, đếm tần suất từ văn bản, ghi tệp nén nhị phân, đo tỉ lệ nén.

---

## 2. Thuật toán và cài đặt

Toàn bộ thuật toán nằm trong `src/huffman.cpp`. Cây được lưu trong một mảng `vector<Nut>`. Lá có chỉ số `0..m-1` theo thứ tự nhập, nút cha được thêm vào cuối mảng.

| Hàm | Chức năng |
|---|---|
| `lay_hai_nho_nhat(cay, ds, vt1, vt2)` | Quét danh sách `ds` hai lượt để lấy vị trí của nút nhỏ nhất (`vt1`) và nhỏ nhì (`vt2`) |
| `tao_cay_huffman(ky_tu, f, cay)` | Lặp `m − 1` lần: lấy hai nút nhỏ nhất, tạo nút cha có trọng số bằng tổng hai nút, xóa hai nút khỏi `ds`, thêm nút cha vào cuối `ds`. Trả về chỉ số gốc |
| `tao_ma(cay, nut, tien_to, ma)` | Duyệt đệ quy từ gốc: sang trái thêm `0`, sang phải thêm `1` |
| `tong_so_bit(ma, f)` | Tính Σ `f[i] × độ dài(ma[i])` |
| `khong_tien_to(ma)` | So từng cặp mã để kiểm tra không mã nào là tiền tố của mã khác |
| `ma_hoa`, `giai_ma` | Ghép mã của từng ký tự; giải mã bằng cách đi trên cây theo từng bit |
| `kiem_tra_hop_le(m, ky_tu, f)` | Kiểm tra `m ∈ [1..8]`, ký tự không trùng, `f[i] ∈ [1..100]` |

**Quy ước cố định** (mã sinh ra phụ thuộc vào các quy ước này):

- Khi hai nút có trọng số bằng nhau, chọn nút đứng **trước** trong danh sách `ds`.
- Nút nhỏ nhất làm con **trái**, nút nhỏ nhì làm con **phải**.
- Nút cha được thêm vào **cuối** danh sách.
- Nếu chỉ có một ký tự (`m = 1`), mã của ký tự đó là `"0"`.

Mã Huffman không duy nhất: nếu đổi quy ước hòa hoặc đổi trái/phải thì mã có thể khác. Tuy vậy tổng số bit luôn là giá trị nhỏ nhất và không đổi. Tổng số bit cũng bằng tổng trọng số của mọi nút trong.

**Ví dụ tính tay** (Ví dụ 1 của đề, `A B C D`, f = `5 9 12 13`):

| Bước | Hai nút nhỏ nhất | Nút cha |
|---|---|---|
| 1 | 5 (A), 9 (B) | 14 |
| 2 | 12 (C), 13 (D) | 25 |
| 3 | 14, 25 | 39 (gốc) |

Tổng bit = 14 + 25 + 39 = **78**. Mã nhận được là A=`00`, B=`01`, C=`10`, D=`11`.

---

## 3. Độ phức tạp

Gọi `m` là số ký tự (`m ≤ 8`) và `L` là độ dài mã lớn nhất (`L ≤ m − 1`).

| Thành phần | Thời gian | Ghi chú |
|---|---|---|
| Dựng cây | O(m²) | `m − 1` lần ghép, mỗi lần quét O(k) với k là số nút còn trong `ds` |
| Số phép so sánh trọng số | đúng **(m − 1)²** | Với k nút cần (k − 1) + (k − 2) = 2k − 3 phép; tổng từ k = 2 đến m là (m − 1)² |
| Sinh mã | O(m · L) | Mỗi lá nhận một chuỗi dài tối đa L |
| Kiểm tra tiền tố | O(m² · L) | So mọi cặp mã |
| Bộ nhớ | O(m) | Cây có 2m − 1 nút |

Chương trình có đếm số phép so sánh (`dem_so_sanh`). Số đếm thực tế khớp với công thức (m − 1)² ở mọi ca đã chạy: m = 1 → 0, m = 2 → 1, m = 3 → 4, m = 4 → 9, m = 5 → 16, m = 8 → 49. Ca `KT10` kiểm tra công thức này một cách tự động.

Vì `m ≤ 8`, việc dùng heap (O(m log m)) không đem lại lợi ích đáng kể. Cách quét tuyến tính bám sát mô tả của đề bài và dễ đếm phép so sánh hơn.

---

## 4. Kiểm thử

Bộ kiểm thử có 12 ca độc lập, cài trong hàm `chay_kiem_tra()` (`src/huffman.cpp`). Chạy bằng `test/runtests.exe` hoặc chọn mục `1` trong `src/huffman.exe`. Giá trị mong đợi được ghi cố định trong mã. Một ca chỉ được tính PASS khi mọi phép kiểm (assert) của ca đó đều đúng.

| Mã ca | Đối tượng | Đầu vào | Giá trị mong đợi chính | Nguồn |
|---|---|---|---|---|
| VD01 | Ví dụ 1 | `A B C D`, f = `5 9 12 13` | tổng bit 78, gốc 39 | Ví dụ 1 / ca 5 của phiếu |
| VD02 | Ví dụ 2 | `A B`, f = `2 3`, chuỗi `ABBA` | mã `0`, `1`; tổng bit 5 | Ví dụ 2 / ca 2 của phiếu |
| TK01 | Tần suất không sắp tăng | `A B C D`, f = `1 4 2 5`, chuỗi `ACDB` | tổng bit 22, gốc 12, chuỗi bit `100101011` | Tự thiết kế |
| TK02 | Nhiều trọng số hòa | `A..E`, f = `2 2 4 4 8` | tổng bit 44, gốc 20 | Tự thiết kế |
| BIEN01 | Biên m = 1 | `A`, f = `7`, chuỗi `AAA` | mã `0`, tổng bit 7, 0 phép so sánh | Ca 1 của phiếu |
| BIEN02 | Biên m = 8 | `A..H`, f = `5` (bằng nhau) | tổng bit 120, gốc 40 | Tự thiết kế |
| BIEN03 | Biên f = 1, hòa toàn bộ | `A B C`, f = `1 1 1` | tổng bit 5, gốc 3 | Ca 3 của phiếu |
| LOI01 | Kiểm tra đầu vào | m = 0; ký tự trùng; f = 120 | cả ba bị từ chối | Điều kiện phiếu |
| KT09 | Tần suất lệch mạnh | `A B C`, f = `1 1 8` | tổng bit 12, C có mã 1 bit | Ca 4 của phiếu |
| KT10 | Biên f = 100 | `A..H`, f = `100` | tổng bit 2400, gốc 800, 49 phép so sánh | Tự thiết kế |
| KT11 | Hàm `lay_hai_nho_nhat` khi hòa | trọng số `4 2 2 5` | vt1 = 1, vt2 = 2, 5 phép so sánh | Kiểm tra đơn vị |
| KT12 | Hàm `khong_tien_to` | `{0, 01, 11}` và `{0, 10, 11}` | sai / đúng | Kiểm tra đơn vị |

**Kết quả** khi chạy `test/runtests.exe`:

```
So ca kiem thu doc lap : 12/12 PASS (0 FAIL)
Tong so phep kiem (assert): 38/38 PASS (0 FAIL)
```

Bản web trong thư mục `web/` có bộ kiểm riêng gồm 25 ca, cho kết quả 25/25 ca, 105/105 phép kiểm (chạy bằng `web\chay_web.bat`, kết quả lưu ở `web/tests/ket_qua_kiem_tra.txt`).

**Lưu ý về độ tin cậy:**
- Các ca chỉ kiểm tra những bảng có tối đa 8 ký tự. Chưa có kiểm thử ngẫu nhiên hay đối chiếu tự động với một cài đặt Huffman độc lập.
- Ở các ca có trọng số hòa, mã cụ thể phụ thuộc vào quy ước ở mục 2. Vì vậy các ca này kiểm tổng bit và tính không tiền tố, không kiểm từng mã.

---

## 5. Cấu trúc thư mục

```
N24DCDT070_DT058_DangThePhu_MaNguon/
├── src/
│   ├── huffman.cpp        # Thuật toán, menu console, bộ 12 ca kiểm thử
│   └── huffman.exe        # Tệp thực thi của huffman.cpp
├── test/
│   └── runtests.cpp       # Nạp src/huffman.cpp và gọi chay_kiem_tra()
├── web/                   # Web trực quan hóa và đánh giá theo đề xuất của giảng viên (xem web/README.md)
│   ├── huffman_core.hpp   # Thuật toán tham lam có ghi vết mô phỏng
│   ├── bo_kiem_tra.hpp    # Bộ kiểm tra tính đúng (25 ca, có nguồn đối chiếu độc lập)
│   ├── server.cpp         # Máy chủ C++ (API, đo thời gian, lịch sử tệp/PostgreSQL)
│   ├── upstash_cache.hpp  # Bộ nhớ đệm Upstash Redis (tùy chọn; cấu hình trong upstash.txt hoặc biến môi trường)
│   ├── httplib.h          # cpp-httplib 0.18.3 (Yuji Hirose, giấy phép MIT)
│   ├── public/            # Giao diện HTML/CSS/JS
│   ├── tests/kiem_tra.cpp # Chạy bộ kiểm tra từ dòng lệnh
│   ├── chay_web.bat       # Windows: kiểm tra + biên dịch + mở trình duyệt
│   └── Dockerfile         # Triển khai Render
├── render.yaml            # Render Blueprint: web Docker + PostgreSQL
├── docs/anh_ket_qua/      # 11 ảnh minh chứng A01–A11
├── .vscode/tasks.json     # Task VS Code để biên dịch và chạy bản web
└── README.md
```

Các tệp `.exe` là sản phẩm biên dịch. Nếu sửa tệp `.cpp` thì phải biên dịch lại, nếu không `.exe` vẫn chạy theo mã cũ.

---

## 6. Biên dịch và chạy

Yêu cầu: `g++` hỗ trợ C++17 (MinGW-w64 trên Windows hoặc GCC trên Linux). Mở PowerShell tại thư mục gốc của kho mã:

```powershell
g++ --version
g++ -O2 -std=c++17 src/huffman.cpp -o src/huffman.exe
g++ -O2 -std=c++17 test/runtests.cpp -o test/runtests.exe
```

**Chạy bộ kiểm thử tự động:**

```powershell
.\test\runtests.exe
```

**Chạy chương trình tương tác:**

```powershell
.\src\huffman.exe
```

- `1`: chạy 5 ca của phiếu rồi in bảng tổng kết 12 ca.
- `2`: nhập bảng tần suất từ bàn phím (số ký tự → các ký tự trên một dòng → các tần suất trên một dòng → chuỗi thử, có thể để trống).
- `0`: thoát.

**Web trực quan hóa và đánh giá** (dựa theo bản đề xuất nâng cấp của giảng viên): bấm đúp `web\chay_web.bat`, hoặc xem hướng dẫn đầy đủ (chạy trên máy, triển khai Render) trong [`web/README.md`](web/README.md). Web có mô phỏng từng bước đồng bộ (cây, `ds`, mã giả, ngăn xếp lời gọi, nhật ký), chế độ luyện tập, thực nghiệm theo m có biểu đồ, bộ kiểm tra tính đúng và lịch sử có nút mở lại.

Toàn bộ phần web nằm trong thư mục `web/`, độc lập với `src/`.

---

## 7. Dữ liệu mẫu và kết quả thực tế

Mỗi khối dưới đây được dán vào dấu nhắc `Chon:` của `.\src\huffman.exe`. Cột kết quả là kết quả thực tế khi chạy chương trình.

| Ca | Đầu vào | Mã | Tổng bit | Chuỗi bit |
|---|---|---|---|---|
| Ví dụ 1 | `A B C D` / `5 9 12 13` / `ABCD` | A=00, B=01, C=10, D=11 | 78 | `00011011` |
| Ví dụ 2 | `A B` / `2 3` / `ABBA` | A=0, B=1 | 5 | `0110` |
| Tự thiết kế 1 | `A B C D` / `1 4 2 5` / `ACDB` | A=100, B=11, C=101, D=0 | 22 | `100101011` (9 bit) |
| Tự thiết kế 2 | `A..E` / `2 2 4 4 8` / `ABCDE` | A=100, B=101, C=00, D=01, E=11 | 44 | `100101000111` |
| Biên 1 | `A` / `7` / `AAA` | A=0 | 7 | `000` |
| Biên 2 | `A..H` / `5`×8 / `ABCDEFGH` | mọi mã dài 3 bit (000…111) | 120 | 24 bit |
| Biên 3 | `A B C` / `1 1 1` / `ABC` | A=10, B=11, C=0 | 5 | `10110` |

Ví dụ khối nhập cho Ví dụ 1:

```text
2
4
A B C D
5 9 12 13
ABCD
```

**Ca nhập sai** (A08): mỗi giá trị sai bị từ chối và chương trình yêu cầu nhập lại, sau đó dữ liệu đúng được chấp nhận.

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

Kết quả: từ chối `m = 0`, `m = 9`, `A A B` (trùng ký tự), `1 120 5` (ngoài 1..100). Sau đó chấp nhận `A B C` / `2 3 5`, cho tổng bit 15 và mã A=10, B=11, C=0.

---

## 8. Giới hạn đã biết

- **Miền dữ liệu nhỏ theo phiếu:** `m ≤ 8`, `f ∈ [1..100]`. Thuật toán vẫn đúng với m lớn hơn, nhưng chương trình không cho nhập ngoài miền này.
- **Ký tự một byte:** mỗi ký tự là một `char`. Bản console đọc mọi ký tự khác khoảng trắng; bản web chỉ nhận ký tự ASCII in được (mã 33..126). Chữ tiếng Việt có dấu (UTF-8 nhiều byte) và khoảng trắng không dùng được làm ký tự.
- **Chuỗi thử:** nếu chuỗi có ký tự ngoài bảng, bản console bỏ qua bước mã hóa và thông báo.
- **Không nén tệp:** chuỗi bit được in dưới dạng văn bản `0`/`1`, không được đóng gói thành byte, nên không đo được tỉ lệ nén thực tế (phần (b) của câu 38 nằm ngoài phạm vi).
- **Mã không duy nhất:** một cài đặt khác dùng quy ước hòa khác có thể cho mã khác nhưng cùng tổng bit.

---

## 9. Clone sạch và chạy lại (A10)

```powershell
git clone https://github.com/Enderghas999/N24DCDT070_DT058_Huffman.git kiem_tra_ban_nop_N24DCDT070
cd kiem_tra_ban_nop_N24DCDT070
git rev-parse HEAD
g++ --version
g++ -O2 -std=c++17 test/runtests.cpp -o test/runtests.exe
.\test\runtests.exe
```

Kết quả mong đợi: `12/12 PASS (0 FAIL)` và `38/38 PASS (0 FAIL)`.

# Web trực quan hóa và đánh giá thuật toán Huffman (DT058)

Ứng dụng web cho đề tài **DT058 — Tạo mã Huffman từ bảng tần suất nhỏ** (INT13185), viết dựa theo bản *Đề xuất nâng cấp web* ngày 19/10/2026 của giảng viên. Toàn bộ thuật toán chạy ở máy chủ **C++17**. Trình duyệt chỉ hiển thị kết quả và các sự kiện do máy chủ ghi lại.

- **Sinh viên:** Đặng Thế Phú — N24DCDT070 — D24CQDT01-N — Nhóm 03
- **Thuật toán:** chỉ dùng **tham lam**. Mỗi vòng tìm hai nút có trọng số nhỏ nhất bằng cách quét tuyến tính danh sách `ds` (không dùng heap), ghép chúng thành nút cha, lặp đến khi còn một gốc. Thuật toán giống `src/huffman.cpp`.
- **Căn cứ:** Giáo trình INT1306, Chương 4, bài tập câu 38 «Cài đặt mã Huffman» (trang in 112, trang PDF 124).

---

## 1. Đối chiếu với đề xuất của giảng viên

| Mục trong đề xuất | Cách thực hiện cho bài Huffman | Trạng thái |
|---|---|---|
| **1. Mô phỏng từng bước** | Mỗi bước hiển thị đồng bộ: rừng/cây Huffman (nút đang xét, nhỏ nhất, nhỏ nhì, vừa tạo), mảng `ds` (vị trí `i`, `vt1`, `vt2`), mã giả có tô sáng dòng, ngăn xếp lời gọi (`tao_cay_huffman` → `lay_hai_nho_nhat`, các lời gọi đệ quy `tao_ma`), bộ đếm, lời giải thích và nhật ký. Có nút tự chạy, tạm dừng, chỉnh tốc độ, lùi, về đầu, đến cuối và thanh kéo. Mỗi sự kiện chứa ảnh chụp đầy đủ trạng thái, nên khi lùi thì cây, mảng, ngăn xếp và nhật ký cùng trở về một trạng thái. Dòng “Hệ thống chưa chạy...” được cập nhật theo bước. | Có |
| **2. Bài toán có ràng buộc, hai phương pháp** | Không áp dụng. Theo yêu cầu của sinh viên, web chỉ có thuật toán tham lam. Để kiểm tra tính đúng thay cho phần đối chứng, bộ kiểm tra dùng một nguồn đối chiếu độc lập (mục 7). | Không làm |
| **3. Đánh giá thuật toán** | Tab “Đánh giá thuật toán” có các bộ đếm với định nghĩa cố định (số phép so sánh, số lần ghép, số nút, số lời gọi `tao_ma`, khối lượng đầu ra) và thời gian tách thành 5 nhiệm vụ: tính tổng bit, sinh bảng mã, ghi vết, truyền dữ liệu, hiển thị. Phần thực nghiệm theo m có bảng số liệu, 2 biểu đồ và nhận xét tính từ số đo. Mỗi cấu hình chạy lặp (mặc định 10 lần), lấy trung vị và ghi rõ môi trường đo. | Có |
| **4. Tách chế độ sử dụng** | Ba chế độ: *Tính kết quả*, *Học từng bước* và *Luyện tập*. Phần học dùng giới hạn của phiếu (m ≤ 8). Riêng phần thực nghiệm cho m đến 4096. Số sự kiện mô phỏng có giới hạn, và thực nghiệm có nút Dừng. | Có |
| **5. Lịch sử và mở lại cấu hình** | Mỗi lần chạy lưu thời điểm, ký tự, tần suất, chuỗi thử, chế độ, thuật toán, tổng bit, các bộ đếm và thời gian. Có nút “Mở lại” và “Chạy lại”. Lịch sử dùng chung cho mọi người truy cập. Chạy trên máy thì lưu vào tệp; trên Render thì lưu vào PostgreSQL. | Có |
| **6. Chế độ luyện tập** | Người học phải trả lời trước khi xem bước tiếp. Các câu hỏi: hai nút nào được ghép tiếp, trọng số nút cha, vì sao giữ nút đứng trước khi bằng nhau, mã của một ký tự, tổng số bit, vì sao m = 1 có mã “0”. Mỗi câu có gợi ý, lời giải gắn với trạng thái đang hiển thị, và tính điểm. | Có |
| **7. Bộ kiểm tra tính đúng** | Tab “Kiểm tra tính đúng” và chương trình `tests/kiem_tra.cpp` chạy 25 ca: ví dụ của đề, ca tự thiết kế, ca biên, hàm con, khớp vết mô phỏng, 10 ca dữ liệu sai, và 8 tính chất kiểm trên 2000 bảng ngẫu nhiên. | Có |

## 2. Kiểm tra tính đúng

Nguồn đối chiếu độc lập là hàm `chi_phi_toi_uu()` trong `bo_kiem_tra.hpp`. Theo bất đẳng thức Kraft, một bộ độ dài mã có mã tiền tố tương ứng khi và chỉ khi Σ 2^−lᵢ ≤ 1. Hàm này duyệt mọi bộ độ dài thỏa điều kiện đó và trả về tổng bit nhỏ nhất. Hàm không dùng quy tắc ghép tham lam, chỉ dùng trong kiểm tra.

Các tính chất được kiểm trên mỗi bảng ngẫu nhiên:

1. Có đúng m mã, không mã nào rỗng.
2. Không mã nào là tiền tố của mã khác.
3. Σ 2^−độ dài = 1 (cây đầy đủ).
4. Tổng bit = tổng trọng số các nút cha.
5. Tổng bit = giá trị tối ưu của nguồn đối chiếu.
6. Giải mã(mã hóa(s)) = s.
7. Số phép so sánh = (m − 1)².
8. Trọng số gốc = Σ f.

Kết quả chạy `tests/kiem_tra.exe` (2000 bảng, hạt giống 2026): **25/25 ca đạt, 105/105 phép kiểm đạt**. Khi build Docker, bộ kiểm tra cũng chạy; nếu có ca sai thì build dừng lại.

## 3. Cấu trúc thư mục `web/`

```
web/
├── huffman_core.hpp   # Thuật toán tham lam + ghi vết sự kiện (cùng một đoạn mã cho mô phỏng và đo thời gian)
├── bo_kiem_tra.hpp    # Bộ kiểm tra tính đúng + nguồn đối chiếu (Kraft)
├── server.cpp         # Máy chủ HTTP (cpp-httplib), API JSON, đo thời gian, lưu lịch sử (tệp / PostgreSQL)
├── httplib.h          # cpp-httplib 0.18.3 (MIT)
├── public/            # Giao diện: index.html, style.css, app.js
├── tests/kiem_tra.cpp # Chương trình chạy bộ kiểm tra từ dòng lệnh
├── chay_web.bat       # Windows: chạy kiểm tra, biên dịch, mở trình duyệt
├── Dockerfile         # Ảnh Docker để triển khai Render
└── README.md
```

File `render.yaml` nằm ở thư mục gốc của kho mã.

## 4. Chạy trên máy (Windows, MinGW-w64)

Cách nhanh: bấm đúp `web\chay_web.bat`. Hoặc chạy từng lệnh trong thư mục `web`:

```powershell
g++ -std=c++17 -O2 tests\kiem_tra.cpp -o tests\kiem_tra.exe
.\tests\kiem_tra.exe
g++ -std=c++17 -O2 server.cpp -o huffman_app.exe -lws2_32
.\huffman_app.exe
```

Sau đó mở `http://localhost:8080`. Chương trình phải chạy từ thư mục `web` để tìm thấy `public/`. Lịch sử được lưu ở `web/data/lich_su.jsonl` và vẫn còn sau khi khởi động lại. Trên Linux, thay `-lws2_32` bằng `-lpthread`.

| Biến môi trường | Mặc định | Ý nghĩa |
|---|---|---|
| `PORT` | `8080` | Cổng máy chủ |
| `PUBLIC_DIR` | `./public` | Thư mục giao diện |
| `HISTORY_FILE` | `data/lich_su.jsonl` | Tệp lịch sử khi không dùng PostgreSQL |
| `DATABASE_URL` | (trống) | Chuỗi kết nối PostgreSQL; chỉ có tác dụng khi biên dịch với `-DUSE_PG -lpq` |

## 5. Triển khai lên Render

1. Commit và push thư mục `web/` cùng `render.yaml` lên GitHub.
2. Trên Render chọn **New → Blueprint**, chọn repo này. Render đọc `render.yaml` và tạo:
   - web service `dt058-huffman` (Docker, gói Free);
   - cơ sở dữ liệu `dt058-huffman-db` (PostgreSQL, gói Free). Biến `DATABASE_URL` được Render gán tự động.
3. Chờ build xong (bước build có chạy bộ kiểm tra), rồi mở địa chỉ `https://dt058-huffman….onrender.com`.
4. **Kiểm tra nghiệm thu mục 5 của đề xuất:** chạy một bảng, bấm *Manual Deploy* để triển khai lại, sau đó kiểm tra lịch sử và nút “Mở lại”.

Lưu ý về gói Free, theo [Render — Deploy for Free](https://render.com/docs/free) như trong đề xuất:
- Web ngủ sau 15 phút không có truy cập, nên lần mở đầu tiên chậm.
- PostgreSQL Free hết hạn sau 30 ngày.
- Đổi sang PostgreSQL không bỏ được thời gian khởi động chậm của web Free.

## 6. API

| Đường dẫn | Mô tả |
|---|---|
| `GET /api/chay?kt=A B C D&f=5 9 12 13&s=ABCD&che_do=hoc` | Chạy thuật toán, trả về cây, bảng mã, bộ đếm, thời gian, danh sách sự kiện; lưu lịch sử |
| `GET /api/danh_gia?m=2,4,8,…&lap=10&hat=1` | Thực nghiệm theo m (1..4096, tối đa 16 giá trị) |
| `GET /api/kiem_tra?n=2000&hat=2026` | Chạy bộ kiểm tra tính đúng |
| `GET /api/lich_su?gioi_han=20` | Lịch sử, mới nhất trước |
| `GET /api/moi_truong` | Môi trường đo, nơi lưu lịch sử |

## 7. Giới hạn đã biết

- **Miền dữ liệu:** ký tự là ASCII in được (mã 33..126); m ≤ 8; f ∈ [1..100] theo phiếu. Chữ tiếng Việt có dấu không dùng được làm ký tự.
- **Thời gian đo:** số đo phụ thuộc máy và mức tải lúc đo. Với m ≤ 8, thời gian thuật toán chỉ khoảng vài trăm nano giây, nên các số đo dao động; vì thế web hiển thị cả trung vị, min và max.
- **Lịch sử chung:** lịch sử dùng chung cho mọi người truy cập, không phân biệt người dùng.
- **Không nén tệp:** web không nén tệp văn bản thật; phần (b) của câu 38 nằm ngoài phạm vi đề DT058.
- **Phần chưa kiểm thử:** `Dockerfile` và `render.yaml` được viết theo tài liệu Render nhưng chưa build thử trên Render. Phần lưu PostgreSQL đã thử với PostgreSQL 16 cục bộ: lịch sử vẫn còn sau khi khởi động lại máy chủ.

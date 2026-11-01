# Kế hoạch 11 ảnh A01–A11 cho đề tài cá nhân

Khung chưa có ảnh và chưa có logic chạy đạt. Tự hoàn thành, chạy và chụp bằng
chứng thật. Kế hoạch này là cùng bộ ảnh trong hướng dẫn Classroom và mẫu báo
cáo cá nhân, không yêu cầu chụp thêm một bộ ảnh khác.

| Mã ảnh | Hình trong báo cáo | Tên tệp gợi ý | Mục tiêu |
|---|---|---|---|
| A01 | 4.1 | `01_vi_du_1.png` | Ví dụ 1 của đúng phiếu cá nhân |
| A02 | 4.2 | `02_vi_du_2.png` | Ví dụ 2 của đúng phiếu cá nhân |
| A03 | 4.3 | `03_tu_thiet_ke_1.png` | Ca tự thiết kế thứ nhất, có expected độc lập |
| A04 | 4.4 | `04_tu_thiet_ke_2.png` | Ca tự thiết kế thứ hai, có mục tiêu khác |
| A05 | 4.5 | `05_bien_1.png` | Ca biên hợp lệ thứ nhất của đúng đề |
| A06 | 4.6 | `06_bien_2.png` | Ca biên hợp lệ thứ hai của đúng đề |
| A07 | 4.7 | `07_bien_3.png` | Ca biên hợp lệ thứ ba của đúng đề |
| A08 | 4.8 | `08_input_khong_hop_le.png` | Input vi phạm điều kiện phiếu và phản hồi từ chối của validator |
| A09 | 4.9 | `09_tong_ket_kiem_thu.png` | Lệnh chạy và tổng kết ít nhất 12 ca độc lập; số ca và số assert ghi riêng |
| A10 | 4.10 | `10_clone_sach_chay_lai.png` | Clone C vào thư mục mới, kiểm HEAD, môi trường rồi chạy ví dụ/test theo README |
| A11 | 4.11 | `11_github_repo_commit.png` | Trang repo/commit GitHub thật của sinh viên tại C |

Với miền chương trình chính nhỏ, tám mục tiêu A01–A08 có thể gồm ca hàm con,
validator, bộ kiểm kết quả hoặc bất biến đúng đề; ghi rõ đối tượng, expected
và lý do thay thế. Không mở rộng miền dữ liệu trái phiếu chỉ để đủ số ảnh.

Mỗi ảnh có mã ca, input hoặc vị trí khối input đầy đủ, lệnh, output đọc được,
số hình/chú thích và ít nhất hai câu phân tích đối chiếu actual với expected.
Lưu ảnh gốc tại thư mục này và chèn ảnh vào báo cáo. Tên tệp có thể thay đổi
nhưng phải đồng bộ trong mã, báo cáo và danh mục ảnh.

Bảng truy vết tay, kiểm nghiệm, đối chiếu và khảo sát ba quy mô/cấu hình hợp
lệ có thể dùng chung các ca phù hợp, trình bày thêm bằng bảng/log. Sơ đồ thuật
toán hoặc ảnh đoạn mã không thay thế ảnh thực thi. Một lần chạy chụp nhiều ảnh
không được tính thành nhiều ca độc lập.

A10 có thể chụp từ clone C trước khi thêm ảnh/báo cáo. Sau khi chốt S vẫn phải
clone S chạy lại trước nộp; ghi kết quả trong TXT nộp rời. Không tạo vòng commit
chỉ để đưa ảnh chứa SHA của chính S vào S. A11 phải là GitHub thật của sinh
viên; bằng chứng Git local của mẫu giảng viên không thay bước này.

Không tạo ảnh repo, URL, commit hoặc kết quả clone giả. Khi mã/test/input
thay đổi và ảnh hưởng kết quả, chạy lại phần liên quan rồi cập nhật bằng chứng.

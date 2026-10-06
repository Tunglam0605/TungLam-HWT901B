# Lịch sử thay đổi

## 0.1.1 - 2026-10-06

- Chuẩn hóa toàn bộ comment và Doxygen trong thư viện sang tiếng Việt có dấu.
- Bổ sung mô tả chi tiết cho enum, trường dữ liệu, getter, hàm cấu hình và API thanh ghi.
- Bổ sung chú thích tiếng Việt cho luồng parser, checksum, resync, giải mã đơn vị và offset góc trong file triển khai.
- Viết lại chú thích của toàn bộ 7 ví dụ theo hướng giải thích từng bước, phù hợp cho người học Arduino và robotics.
- Chuẩn hóa README và tài liệu giao thức sang tiếng Việt.
- Đổi mô tả GitHub About sang tiếng Việt.
- Không thay đổi API công khai hoặc logic giao tiếp so với v0.1.0.

## 0.1.0 - 2026-10-06

- Phát hành công khai lần đầu.
- Bộ phân tích frame WIT 11 byte không chặn, có kiểm tra checksum và tự đồng bộ lại.
- Giải mã gia tốc, gyro, góc Euler, từ kế và quaternion.
- API tương thích HardwareSerial và SoftwareSerial.
- Zero góc bằng phần mềm và gán tư thế hiện tại thành một góc bất kỳ.
- Hỗ trợ đọc/ghi thanh ghi WIT.
- Có các hàm cấu hình nhanh cho robot và bộ đếm chẩn đoán.
- Kèm ví dụ Arduino và GitHub Actions CI.

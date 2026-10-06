# Ghi chú giao thức HWT901B

Thư viện sử dụng **WIT Standard Protocol** để giao tiếp với HWT901B qua TTL/UART.

## Khung dữ liệu đo lường

Mỗi frame dữ liệu chuẩn dài 11 byte:

```text
0x55 | TYPE | DATA0..DATA7 | CHECKSUM
```

Trong đó:

- `0x55` là byte đầu khung cố định.
- `TYPE` xác định loại dữ liệu.
- `DATA0..DATA7` là 8 byte payload.
- `CHECKSUM` là tổng của 10 byte đầu, chỉ giữ 8 bit thấp.

Nếu checksum sai, thư viện không chặn để chờ frame mới. Bộ phân tích tìm lại byte `0x55` trong chính buffer hiện tại để đồng bộ nhanh hơn khi UART mất hoặc lệch byte.

## Các frame đang được giải mã

| Loại frame | Nội dung |
|---|---|
| `0x51` | Gia tốc X/Y/Z và nhiệt độ |
| `0x52` | Vận tốc góc X/Y/Z và điện áp |
| `0x53` | Roll/Pitch/Yaw và version |
| `0x54` | Từ trường X/Y/Z |
| `0x59` | Quaternion q0/q1/q2/q3 |
| `0x5F` | Phản hồi đọc thanh ghi |

Các frame WIT khác vẫn có thể vượt qua bước kiểm tra checksum. Nếu chưa có bộ giải mã riêng, chúng được tính vào `unknownFrameCount()`.

## Hệ số chuyển đổi

Thư viện sử dụng các hệ số của WIT Standard Protocol:

```text
Gia tốc:
raw * 16 / 32768        -> g
g * 9.80665             -> m/s²

Gyro:
raw * 2000 / 32768      -> độ/giây
độ/giây * pi / 180      -> rad/s

Góc Euler:
raw * 180 / 32768       -> độ

Quaternion:
raw / 32768
```

## Gói lệnh thanh ghi

Gói ghi thanh ghi WIT dài 5 byte:

```text
FF AA ADDR DATA_L DATA_H
```

Thư viện tự gửi chuỗi thao tác cần thiết khi ghi cấu hình.

```cpp
imu.writeRegister(address, value);
```

Với cấu hình thông dụng nên ưu tiên các hàm cấp cao như `setRate()`, `setOutput()`, `setBandwidth()` và `configureRobotMode()`.

## Đọc thanh ghi

`requestRegister(address)` gửi yêu cầu đọc bắt đầu từ địa chỉ chỉ định.

Khi HWT901B trả về frame `0x5F`, thư viện lưu bốn word nhận được:

```cpp
if (imu.hasRegisterResponse()) {
  int16_t word0 = imu.lastRegisterValue(0);
  int16_t word1 = imu.lastRegisterValue(1);
  int16_t word2 = imu.lastRegisterValue(2);
  int16_t word3 = imu.lastRegisterValue(3);
}
```

## Zero phần mềm và zero trên cảm biến

### Zero phần mềm

```cpp
imu.zeroYaw();
imu.setCurrentYaw(180.0f);
```

Hai hàm trên chỉ thay đổi offset trong RAM Arduino:

- không thay đổi dữ liệu góc thô của cảm biến;
- phù hợp khi robot cần đổi mốc tọa độ trong lúc chạy;
- offset mất khi Arduino reset.

Công thức:

```text
offset = góc_raw - góc_mong_muốn
góc_sử_dụng = normalize(góc_raw - offset)
```

Thư viện dùng miền `(-180°, 180°]`, do đó mốc `+180°` được giữ đúng là `+180°`.

### Zero trực tiếp trên HWT901B

```cpp
imu.sensorZeroHeading();
```

Hàm này gửi lệnh hiệu chỉnh heading xuống cảm biến. Đây là thao tác khác với offset phần mềm và chỉ nên dùng khi thật sự muốn thay đổi mốc heading ở phía HWT901B.

## Baud và tần số dữ liệu

Baud mặc định thường dùng của WIT Standard Protocol là 9600 bit/s. Khi cấu hình nhiều frame ở tần số cao, đặc biệt ACC + GYRO + ANGLE ở 100 Hz, nên dùng baud cao hơn như 115200 bit/s để có đủ băng thông UART.

`setSensorBaud()` chỉ thay đổi baud phía cảm biến. Sau khi gọi, UART phía Arduino cũng phải được khởi tạo lại theo baud mới.

# HWT901B protocol notes

Thư viện dùng **WIT Standard Protocol** cho giao tiếp TTL/UART.

## Frame telemetry

Mỗi frame chuẩn dài 11 byte:

```text
0x55 | TYPE | DATA0..DATA7 | CHECKSUM
```

Checksum là tổng 10 byte đầu, lấy 8 bit thấp.

Các frame được decode trong v0.1.0:

- `0x51`: acceleration + temperature
- `0x52`: angular velocity
- `0x53`: roll / pitch / yaw
- `0x54`: magnetic field
- `0x59`: quaternion
- `0x5F`: register response

## Register command

```text
FF AA ADDR DATA_L DATA_H
```

Theo WIT, thao tác ghi cấu hình dùng:

```text
unlock -> write -> save
```

Library đóng gói chuỗi này trong `writeRegister()`.

## Software zero va sensor zero

`zeroYaw()` và `setCurrentYaw()` chỉ thay đổi software offset trong Arduino.
Không ghi flash/config của IMU.

`sensorZeroHeading()` gửi CALSW=0x04 tới chính HWT901B.

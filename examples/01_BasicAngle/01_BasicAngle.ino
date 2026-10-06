#include <TungLam_HWT901B.h>

/**
 * Ví dụ 01 - Đọc góc cơ bản bằng HardwareSerial.
 *
 * Phần cứng minh họa: Arduino Mega 2560 + HWT901B TTL.
 *
 * Nối dây:
 *   HWT901B TX  -> Mega RX1 / D19
 *   HWT901B RX  -> Mega TX1 / D18
 *   HWT901B GND -> Mega GND
 *   HWT901B VCC -> cấp đúng điện áp theo module đang sử dụng
 *
 * HWT901B theo cấu hình mặc định thường dùng 9600 baud, vì vậy ví dụ gọi
 * imu.begin(Serial1) mà không cần truyền tham số baud.
 */

HWT901B imu;

void setup() {
  // Serial USB chỉ dùng để quan sát kết quả trên Serial Monitor.
  Serial.begin(115200);

  // Khởi tạo UART phần cứng Serial1 ở baud mặc định 9600.
  imu.begin(Serial1);
}

void loop() {
  // Phải gọi update() thường xuyên để thư viện lấy hết byte đang có trong UART.
  // update() không dùng delay() và không chờ byte mới nên không chặn loop().
  imu.update();

  // Chỉ in góc sau khi đã nhận ít nhất một frame góc 0x53 hợp lệ.
  if (imu.hasAngle()) {
    static uint32_t lastPrint = 0;

    // Giới hạn tốc độ in xuống 10 Hz để Serial Monitor dễ quan sát.
    if (millis() - lastPrint >= 100) {
      lastPrint = millis();

      Serial.print("Roll: ");
      Serial.print(imu.getRoll(), 2);
      Serial.print("  Pitch: ");
      Serial.print(imu.getPitch(), 2);
      Serial.print("  Yaw: ");
      Serial.println(imu.getYaw(), 2);
    }
  }
}

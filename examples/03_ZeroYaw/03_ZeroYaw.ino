#include <TungLam_HWT901B.h>

/**
 * Ví dụ 03 - Zero Yaw bằng offset phần mềm.
 *
 * Gửi ký tự:
 *   z -> lấy tư thế hiện tại làm Yaw = 0°
 *   c -> xóa offset Yaw và quay về góc gốc của cảm biến
 *
 * zeroYaw() chỉ thay đổi offset trong RAM Arduino, không ghi cấu hình xuống IMU.
 */

HWT901B imu;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  Serial.println("Gửi ký tự z để đặt Yaw hiện tại = 0 độ.");
  Serial.println("Gửi ký tự c để xóa offset Yaw phần mềm.");
}

void loop() {
  imu.update();

  // Nhận lệnh đơn giản từ Serial Monitor.
  if (Serial.available()) {
    const char cmd = (char)Serial.read();

    if (cmd == 'z') {
      if (imu.zeroYaw()) {
        Serial.println("Đã zero Yaw bằng offset phần mềm.");
      } else {
        Serial.println("Chưa nhận được frame góc hợp lệ.");
      }
    }

    if (cmd == 'c') {
      imu.clearYawOffset();
      Serial.println("Đã xóa offset Yaw.");
    }
  }

  // In đồng thời góc thô và góc đã bù để thấy rõ tác dụng của offset.
  static uint32_t lastPrint = 0;
  if (imu.hasAngle() && millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print("Yaw thô=");
    Serial.print(imu.getRawYaw(), 2);
    Serial.print("  Yaw đã bù=");
    Serial.println(imu.getYaw(), 2);
  }
}

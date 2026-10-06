#include <TungLam_HWT901B.h>

/**
 * Ví dụ 06 - Cấu hình HWT901B cho robot.
 *
 * Ví dụ giả định CẢ Arduino và HWT901B đã cùng chạy ở 115200 baud.
 * configureRobotMode(Hz100) sẽ yêu cầu cảm biến:
 *   - chỉ phát ACC + GYRO + ANGLE;
 *   - phát với tần số 100 Hz;
 *   - lưu cấu hình sau khi ghi.
 *
 * Không gọi các hàm cấu hình lặp lại liên tục trong loop().
 */

HWT901B imu;

void setup() {
  Serial.begin(115200);

  // Chỉ dùng 115200 tại đây khi cảm biến đã được cấu hình ở đúng baud này.
  imu.begin(Serial1, 115200);

  // Cấu hình nhanh cho robot: ACC + GYRO + ANGLE ở 100 Hz.
  if (imu.configureRobotMode(TungLamHWT901BRate::Hz100)) {
    Serial.println("Đã gửi cấu hình chế độ robot.");
  }

  // Một số cấu hình bổ sung, chỉ bỏ comment khi thật sự cần:
  // imu.setBandwidth(TungLamHWT901BBandwidth::Hz20);
  // imu.setAlgorithm(TungLamHWT901BAlgorithm::Axis9);
  // imu.setOrientation(TungLamHWT901BOrientation::Horizontal);
  // imu.setLed(true);
}

void loop() {
  imu.update();

  // Nếu quá 500 ms không có frame hợp lệ, coi IMU đang mất kết nối.
  if (!imu.connected(500)) {
    static uint32_t lastWarn = 0;

    if (millis() - lastWarn >= 1000) {
      lastWarn = millis();
      Serial.println("Không nhận được dữ liệu IMU.");
    }

    return;
  }

  // Ví dụ chỉ in Yaw ở 10 Hz dù cảm biến có thể đang phát 100 Hz.
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.println(imu.getYaw(), 2);
  }
}

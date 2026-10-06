#include <TungLam_HWT901B.h>

/**
 * Ví dụ 04 - Đọc gia tốc và vận tốc góc.
 *
 * Getter gia tốc mặc định trả m/s².
 * Getter gyro mặc định trả độ/giây.
 *
 * Nếu cần đơn vị khác:
 *   getAccelXg()/Yg()/Zg()      -> g
 *   getGyroXRad()/YRad()/ZRad() -> rad/s
 */

HWT901B imu;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);
}

void loop() {
  imu.update();

  static uint32_t lastPrint = 0;

  // connected() giúp tránh dùng dữ liệu cũ khi IMU đã ngừng truyền.
  if (imu.connected() && millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print("Gia tốc [m/s2] X=");
    Serial.print(imu.getAccelX(), 3);
    Serial.print(" Y=");
    Serial.print(imu.getAccelY(), 3);
    Serial.print(" Z=");
    Serial.print(imu.getAccelZ(), 3);

    Serial.print(" | Gyro [độ/s] X=");
    Serial.print(imu.getGyroX(), 3);
    Serial.print(" Y=");
    Serial.print(imu.getGyroY(), 3);
    Serial.print(" Z=");
    Serial.println(imu.getGyroZ(), 3);
  }
}

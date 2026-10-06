#include <SoftwareSerial.h>
#include <TungLam_HWT901B.h>

/**
 * Ví dụ 05 - Dùng SoftwareSerial trên Arduino UNO/Nano AVR.
 *
 * Nối dây minh họa:
 *   Arduino D10 (RX mềm) <- TX của HWT901B
 *   Arduino D11 (TX mềm) -> RX của HWT901B
 *   GND                  <-> GND
 *
 * Với robot phải xử lý nhiều ngắt, encoder, PID hoặc giao tiếp khác,
 * HardwareSerial vẫn là lựa chọn ưu tiên vì SoftwareSerial có thể làm tăng
 * jitter hoặc mất byte ở baud cao.
 */

// Thứ tự constructor SoftwareSerial là RX, TX.
SoftwareSerial imuSerial(10, 11);
HWT901B imu;

void setup() {
  Serial.begin(115200);

  // Cảm biến phải thực sự đang ở 9600 baud.
  // begin() không tự dò và cũng không tự đổi baud hiện tại của HWT901B.
  imu.begin(imuSerial, 9600);
}

void loop() {
  imu.update();

  static uint32_t lastPrint = 0;
  if (imu.hasAngle() && millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print("Yaw: ");
    Serial.println(imu.getYaw(), 2);
  }
}

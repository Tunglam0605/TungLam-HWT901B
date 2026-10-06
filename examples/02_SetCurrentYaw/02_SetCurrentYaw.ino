#include <TungLam_HWT901B.h>

/**
 * Ví dụ 02 - Gán tư thế Yaw hiện tại thành một góc mong muốn.
 *
 * Tình huống thực tế:
 *   Cảm biến đang báo Yaw khoảng 89°, nhưng trong hệ tọa độ của robot,
 *   chính tư thế đó cần được xem là 180°.
 *
 * Thư viện tự tính offset trong RAM Arduino. Không cần tự tạo biến offset
 * trong sketch và không ghi lại mốc góc vào flash của HWT901B.
 */

HWT901B imu;
bool mapped = false;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  Serial.println("Đặt IMU ở tư thế muốn dùng làm mốc.");
  Serial.println("Khi có dữ liệu, tư thế hiện tại sẽ được gán thành 180 độ.");
}

void loop() {
  imu.update();

  // Chỉ đặt mốc một lần, ngay sau khi đã có frame góc hợp lệ đầu tiên.
  if (imu.hasAngle() && !mapped) {
    Serial.print("Yaw thô trước khi đặt mốc: ");
    Serial.println(imu.getRawYaw(), 2);

    // Ví dụ cảm biến đang đọc khoảng 89°.
    // Sau lệnh này, CHÍNH TƯ THẾ HIỆN TẠI được coi là 180°.
    // getRawYaw() vẫn giữ góc cảm biến, còn getYaw() trả góc đã bù offset.
    if (imu.setCurrentYaw(180.0f)) {
      mapped = true;

      Serial.print("Yaw sau khi đặt mốc: ");
      Serial.println(imu.getYaw(), 2);  // Xấp xỉ 180,00°.

      Serial.print("Offset thư viện đang giữ: ");
      Serial.println(imu.getYawOffset(), 2);
    }
  }

  // Sau khi đặt mốc, tiếp tục theo dõi Yaw đã hiệu chỉnh ở 10 Hz.
  if (mapped) {
    static uint32_t lastPrint = 0;

    if (millis() - lastPrint >= 100) {
      lastPrint = millis();
      Serial.print("Yaw: ");
      Serial.println(imu.getYaw(), 2);
    }
  }
}

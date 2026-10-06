#include <TungLam_HWT901B.h>

/**
 * Ví dụ 07 - API thanh ghi nâng cao.
 *
 * Chỉ dùng phần này khi cần thao tác trực tiếp với thanh ghi WIT.
 * Với các cấu hình thông dụng nên ưu tiên setRate(), setOutput(),
 * setBandwidth(), setAlgorithm()... để code rõ ràng và khó ghi nhầm hơn.
 */

HWT901B imu;
bool requested = false;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  // Yêu cầu đọc bắt đầu từ thanh ghi VERSION 0x2E.
  // Theo cơ chế đang dùng, HWT901B trả frame 0x5F chứa bốn word liên tiếp.
  requested = imu.requestRegister(0x2E);
}

void loop() {
  imu.update();

  // Chờ parser nhận được frame phản hồi thanh ghi 0x5F.
  if (requested && imu.hasRegisterResponse()) {
    requested = false;

    Serial.print("Thanh ghi cơ sở: 0x");
    Serial.println(imu.lastRegisterBase(), HEX);

    // Đọc bốn word dữ liệu đã được thư viện lưu lại.
    for (uint8_t i = 0; i < 4; ++i) {
      Serial.print("Word ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(imu.lastRegisterValue(i));
    }
  }

  // Ví dụ ghi nâng cao:
  // imu.writeRegister(address, value);
  //
  // writeRegister() tự thực hiện chuỗi:
  // MỞ KHÓA -> GHI -> LƯU
}

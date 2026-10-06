#include <TungLam_HWT901B.h>

HWT901B imu;

void setup() {
  Serial.begin(115200);

  // Arduino Mega 2560:
  // RX1 = D19 <- TX cua HWT901B
  // TX1 = D18 -> RX cua HWT901B
  imu.begin(Serial1);
}

void loop() {
  imu.update();

  if (imu.hasAngle()) {
    static uint32_t lastPrint = 0;
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

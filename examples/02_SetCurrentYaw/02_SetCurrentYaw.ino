#include <TungLam_HWT901B.h>

HWT901B imu;
bool mapped = false;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  Serial.println("Dat IMU o tu the mong muon.");
  Serial.println("Khi co du lieu, goc hien tai se duoc gan thanh 180 do.");
}

void loop() {
  imu.update();

  if (imu.hasAngle() && !mapped) {
    Serial.print("Yaw raw truoc khi dat moc: ");
    Serial.println(imu.getRawYaw(), 2);

    // Vi du sensor dang doc khoang 89 do.
    // Sau lenh nay, CHINH TU THE HIEN TAI duoc coi la 180 do.
    if (imu.setCurrentYaw(180.0f)) {
      mapped = true;

      Serial.print("Yaw sau khi dat moc: ");
      Serial.println(imu.getYaw(), 2);       // ~180.00
      Serial.print("Offset library dang giu: ");
      Serial.println(imu.getYawOffset(), 2);
    }
  }

  if (mapped) {
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint >= 100) {
      lastPrint = millis();
      Serial.print("Yaw: ");
      Serial.println(imu.getYaw(), 2);
    }
  }
}

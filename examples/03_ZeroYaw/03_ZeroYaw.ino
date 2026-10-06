#include <TungLam_HWT901B.h>

HWT901B imu;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  Serial.println("Gui ky tu z de dat yaw hien tai = 0 do.");
  Serial.println("Gui ky tu c de xoa software offset.");
}

void loop() {
  imu.update();

  if (Serial.available()) {
    const char cmd = (char)Serial.read();

    if (cmd == 'z') {
      if (imu.zeroYaw()) {
        Serial.println("Da zero yaw bang software.");
      } else {
        Serial.println("Chua co frame angle.");
      }
    }

    if (cmd == 'c') {
      imu.clearYawOffset();
      Serial.println("Da xoa yaw offset.");
    }
  }

  static uint32_t lastPrint = 0;
  if (imu.hasAngle() && millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print("Raw=");
    Serial.print(imu.getRawYaw(), 2);
    Serial.print("  Yaw=");
    Serial.println(imu.getYaw(), 2);
  }
}

#include <TungLam_HWT901B.h>

HWT901B imu;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);
}

void loop() {
  imu.update();

  static uint32_t lastPrint = 0;
  if (imu.connected() && millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print("A[m/s2] X=");
    Serial.print(imu.getAccelX(), 3);
    Serial.print(" Y=");
    Serial.print(imu.getAccelY(), 3);
    Serial.print(" Z=");
    Serial.print(imu.getAccelZ(), 3);

    Serial.print(" | G[deg/s] X=");
    Serial.print(imu.getGyroX(), 3);
    Serial.print(" Y=");
    Serial.print(imu.getGyroY(), 3);
    Serial.print(" Z=");
    Serial.println(imu.getGyroZ(), 3);
  }
}

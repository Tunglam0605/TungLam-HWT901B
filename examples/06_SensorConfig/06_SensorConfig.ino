#include <TungLam_HWT901B.h>

HWT901B imu;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1, 115200);

  // Goi cac ham cau hinh trong setup(), KHONG goi lap lai trong loop().
  // Robot mode: chi output ACC + GYRO + ANGLE o 100 Hz.
  if (imu.configureRobotMode(TungLamHWT901BRate::Hz100)) {
    Serial.println("Da gui cau hinh robot mode.");
  }

  // Vi du bo sung:
  // imu.setBandwidth(TungLamHWT901BBandwidth::Hz20);
  // imu.setAlgorithm(TungLamHWT901BAlgorithm::Axis9);
  // imu.setOrientation(TungLamHWT901BOrientation::Horizontal);
  // imu.setLed(true);
}

void loop() {
  imu.update();

  if (!imu.connected(500)) {
    static uint32_t lastWarn = 0;
    if (millis() - lastWarn >= 1000) {
      lastWarn = millis();
      Serial.println("IMU timeout.");
    }
    return;
  }

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.println(imu.getYaw(), 2);
  }
}

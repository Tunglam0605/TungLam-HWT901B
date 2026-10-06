#include <SoftwareSerial.h>
#include <TungLam_HWT901B.h>

// UNO/Nano AVR example.
// Arduino RX D10 <- TX cua HWT901B
// Arduino TX D11 -> RX cua HWT901B
SoftwareSerial imuSerial(10, 11);
HWT901B imu;

void setup() {
  Serial.begin(115200);

  // SoftwareSerial nen dung baud vua phai neu MCU con xu ly nhieu viec.
  imu.begin(imuSerial, 9600);
}

void loop() {
  imu.update();

  static uint32_t lastPrint = 0;
  if (imu.hasAngle() && millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.println(imu.getYaw(), 2);
  }
}

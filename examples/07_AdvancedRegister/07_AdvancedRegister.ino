#include <TungLam_HWT901B.h>

HWT901B imu;
bool requested = false;

void setup() {
  Serial.begin(115200);
  imu.begin(Serial1);

  // Doc VERSION register 0x2E.
  // WIT tra frame 0x5F gom 4 word bat dau tu dia chi da request.
  requested = imu.requestRegister(0x2E);
}

void loop() {
  imu.update();

  if (requested && imu.hasRegisterResponse()) {
    requested = false;

    Serial.print("Base register: 0x");
    Serial.println(imu.lastRegisterBase(), HEX);

    for (uint8_t i = 0; i < 4; ++i) {
      Serial.print("Word ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(imu.lastRegisterValue(i));
    }
  }

  // Advanced write:
  // imu.writeRegister(address, value);
}

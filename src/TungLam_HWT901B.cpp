#include "TungLam_HWT901B.h"

#include <string.h>

const float TungLamHWT901B::GRAVITY_MPS2 = 9.80665f;
const float TungLamHWT901B::DEG_TO_RAD_F = 0.01745329251994329577f;

TungLamHWT901B::TungLamHWT901B()
    : _stream(nullptr),
      _baud(0),
      _frameIndex(0),
      _lastRegisterBase(0),
      _registerResponseReady(false) {
  resetRuntime();
}

void TungLamHWT901B::resetRuntime() {
  memset(_frame, 0, sizeof(_frame));
  _frameIndex = 0;

  memset(&_data, 0, sizeof(_data));

  _lastRegisterBase = 0;
  memset(_lastRegisterValues, 0, sizeof(_lastRegisterValues));
  _registerResponseReady = false;
}

void TungLamHWT901B::attach(Stream &stream) {
  _stream = &stream;
  _baud = 0;
  resetRuntime();
}

bool TungLamHWT901B::update() {
  if (_stream == nullptr) {
    return false;
  }

  bool decoded = false;

  while (_stream->available() > 0) {
    const int value = _stream->read();
    if (value < 0) {
      break;
    }

    _data.bytes_received++;
    if (feedByte(static_cast<uint8_t>(value))) {
      decoded = true;
    }
  }

  return decoded;
}

bool TungLamHWT901B::feedByte(uint8_t value) {
  if (_frameIndex == 0) {
    if (value != FRAME_HEAD) {
      return false;
    }

    _frame[0] = value;
    _frameIndex = 1;
    return false;
  }

  _frame[_frameIndex++] = value;

  if (_frameIndex < FRAME_SIZE) {
    return false;
  }

  uint8_t checksum = 0;
  for (uint8_t i = 0; i < (FRAME_SIZE - 1); ++i) {
    checksum = static_cast<uint8_t>(checksum + _frame[i]);
  }

  if (checksum != _frame[FRAME_SIZE - 1]) {
    _data.checksum_errors++;
    resyncAfterBadFrame();
    return false;
  }

  decodeFrame(_frame);
  _frameIndex = 0;
  return true;
}

void TungLamHWT901B::resyncAfterBadFrame() {
  uint8_t next = 1;

  while ((next < FRAME_SIZE) && (_frame[next] != FRAME_HEAD)) {
    ++next;
  }

  if (next >= FRAME_SIZE) {
    _frameIndex = 0;
    return;
  }

  const uint8_t remaining = static_cast<uint8_t>(FRAME_SIZE - next);
  memmove(_frame, &_frame[next], remaining);
  _frameIndex = remaining;
}

int16_t TungLamHWT901B::readI16(const uint8_t *p) {
  return static_cast<int16_t>(
      static_cast<uint16_t>(p[0]) |
      (static_cast<uint16_t>(p[1]) << 8));
}

uint16_t TungLamHWT901B::readU16(const uint8_t *p) {
  return static_cast<uint16_t>(
      static_cast<uint16_t>(p[0]) |
      (static_cast<uint16_t>(p[1]) << 8));
}

void TungLamHWT901B::decodeFrame(const uint8_t *frame) {
  const uint32_t now = millis();

  _data.valid_frames++;
  _data.last_frame_ms = now;

  switch (frame[1]) {
    case FRAME_ACCEL: {
      const int16_t ax = readI16(&frame[2]);
      const int16_t ay = readI16(&frame[4]);
      const int16_t az = readI16(&frame[6]);
      const int16_t temperature = readI16(&frame[8]);

      _data.ax_g = static_cast<float>(ax) * (16.0f / 32768.0f);
      _data.ay_g = static_cast<float>(ay) * (16.0f / 32768.0f);
      _data.az_g = static_cast<float>(az) * (16.0f / 32768.0f);

      _data.ax_mps2 = _data.ax_g * GRAVITY_MPS2;
      _data.ay_mps2 = _data.ay_g * GRAVITY_MPS2;
      _data.az_mps2 = _data.az_g * GRAVITY_MPS2;

      _data.temperature_c = static_cast<float>(temperature) / 100.0f;
      _data.last_accel_ms = now;
      _data.has_accel = true;
      break;
    }

    case FRAME_GYRO: {
      const int16_t gx = readI16(&frame[2]);
      const int16_t gy = readI16(&frame[4]);
      const int16_t gz = readI16(&frame[6]);
      const uint16_t voltage = readU16(&frame[8]);

      _data.gx_dps = static_cast<float>(gx) * (2000.0f / 32768.0f);
      _data.gy_dps = static_cast<float>(gy) * (2000.0f / 32768.0f);
      _data.gz_dps = static_cast<float>(gz) * (2000.0f / 32768.0f);

      _data.gx_rad_s = _data.gx_dps * DEG_TO_RAD_F;
      _data.gy_rad_s = _data.gy_dps * DEG_TO_RAD_F;
      _data.gz_rad_s = _data.gz_dps * DEG_TO_RAD_F;

      _data.voltage_v = static_cast<float>(voltage) / 100.0f;
      _data.last_gyro_ms = now;
      _data.has_gyro = true;
      break;
    }

    case FRAME_ANGLE: {
      const int16_t roll = readI16(&frame[2]);
      const int16_t pitch = readI16(&frame[4]);
      const int16_t yaw = readI16(&frame[6]);

      _data.roll_raw_deg =
          normalize180(static_cast<float>(roll) * (180.0f / 32768.0f));
      _data.pitch_raw_deg =
          normalize180(static_cast<float>(pitch) * (180.0f / 32768.0f));
      _data.yaw_raw_deg =
          normalize180(static_cast<float>(yaw) * (180.0f / 32768.0f));

      _data.version = readU16(&frame[8]);
      _data.last_angle_ms = now;
      _data.has_angle = true;

      applyAngleOffsets();
      break;
    }

    case FRAME_MAG: {
      _data.mx = readI16(&frame[2]);
      _data.my = readI16(&frame[4]);
      _data.mz = readI16(&frame[6]);
      _data.temperature_c =
          static_cast<float>(readI16(&frame[8])) / 100.0f;

      _data.last_mag_ms = now;
      _data.has_mag = true;
      break;
    }

    case FRAME_QUATERNION: {
      _data.q0 = static_cast<float>(readI16(&frame[2])) / 32768.0f;
      _data.q1 = static_cast<float>(readI16(&frame[4])) / 32768.0f;
      _data.q2 = static_cast<float>(readI16(&frame[6])) / 32768.0f;
      _data.q3 = static_cast<float>(readI16(&frame[8])) / 32768.0f;

      _data.last_quaternion_ms = now;
      _data.has_quaternion = true;
      break;
    }

    case FRAME_REGISTER: {
      _lastRegisterValues[0] = readI16(&frame[2]);
      _lastRegisterValues[1] = readI16(&frame[4]);
      _lastRegisterValues[2] = readI16(&frame[6]);
      _lastRegisterValues[3] = readI16(&frame[8]);
      _registerResponseReady = true;
      break;
    }

    default:
      _data.unknown_frames++;
      break;
  }
}

void TungLamHWT901B::applyAngleOffsets() {
  _data.roll_deg =
      normalize180(_data.roll_raw_deg - _data.roll_offset_deg);
  _data.pitch_deg =
      normalize180(_data.pitch_raw_deg - _data.pitch_offset_deg);
  _data.yaw_deg =
      normalize180(_data.yaw_raw_deg - _data.yaw_offset_deg);
}

bool TungLamHWT901B::hasAngle() const {
  return _data.has_angle;
}

bool TungLamHWT901B::connected(uint32_t timeoutMs) const {
  if (_data.valid_frames == 0) {
    return false;
  }

  return static_cast<uint32_t>(millis() - _data.last_frame_ms) <= timeoutMs;
}

uint32_t TungLamHWT901B::ageMs() const {
  if (_data.valid_frames == 0) {
    return 0xFFFFFFFFUL;
  }

  return static_cast<uint32_t>(millis() - _data.last_frame_ms);
}

bool TungLamHWT901B::getData(TungLamHWT901BData &out) const {
  if (_data.valid_frames == 0) {
    return false;
  }

  out = _data;
  return true;
}

float TungLamHWT901B::getRoll() const { return _data.roll_deg; }
float TungLamHWT901B::getPitch() const { return _data.pitch_deg; }
float TungLamHWT901B::getYaw() const { return _data.yaw_deg; }
float TungLamHWT901B::getYaw360() const { return normalize360(_data.yaw_deg); }

float TungLamHWT901B::getRawRoll() const { return _data.roll_raw_deg; }
float TungLamHWT901B::getRawPitch() const { return _data.pitch_raw_deg; }
float TungLamHWT901B::getRawYaw() const { return _data.yaw_raw_deg; }

float TungLamHWT901B::getAccelX() const { return _data.ax_mps2; }
float TungLamHWT901B::getAccelY() const { return _data.ay_mps2; }
float TungLamHWT901B::getAccelZ() const { return _data.az_mps2; }
float TungLamHWT901B::getAccelXg() const { return _data.ax_g; }
float TungLamHWT901B::getAccelYg() const { return _data.ay_g; }
float TungLamHWT901B::getAccelZg() const { return _data.az_g; }

float TungLamHWT901B::getGyroX() const { return _data.gx_dps; }
float TungLamHWT901B::getGyroY() const { return _data.gy_dps; }
float TungLamHWT901B::getGyroZ() const { return _data.gz_dps; }
float TungLamHWT901B::getGyroXRad() const { return _data.gx_rad_s; }
float TungLamHWT901B::getGyroYRad() const { return _data.gy_rad_s; }
float TungLamHWT901B::getGyroZRad() const { return _data.gz_rad_s; }

int16_t TungLamHWT901B::getMagX() const { return _data.mx; }
int16_t TungLamHWT901B::getMagY() const { return _data.my; }
int16_t TungLamHWT901B::getMagZ() const { return _data.mz; }

float TungLamHWT901B::getQ0() const { return _data.q0; }
float TungLamHWT901B::getQ1() const { return _data.q1; }
float TungLamHWT901B::getQ2() const { return _data.q2; }
float TungLamHWT901B::getQ3() const { return _data.q3; }

float TungLamHWT901B::getTemperature() const { return _data.temperature_c; }
float TungLamHWT901B::getVoltage() const { return _data.voltage_v; }

bool TungLamHWT901B::zeroYaw() {
  return setCurrentYaw(0.0f);
}

bool TungLamHWT901B::zeroRoll() {
  return setCurrentRoll(0.0f);
}

bool TungLamHWT901B::zeroPitch() {
  return setCurrentPitch(0.0f);
}

bool TungLamHWT901B::setCurrentYaw(float yawDeg) {
  if (!_data.has_angle) {
    return false;
  }

  const float target = normalize180(yawDeg);
  _data.yaw_offset_deg =
      normalize180(_data.yaw_raw_deg - target);
  applyAngleOffsets();
  return true;
}

bool TungLamHWT901B::setCurrentRoll(float rollDeg) {
  if (!_data.has_angle) {
    return false;
  }

  const float target = normalize180(rollDeg);
  _data.roll_offset_deg =
      normalize180(_data.roll_raw_deg - target);
  applyAngleOffsets();
  return true;
}

bool TungLamHWT901B::setCurrentPitch(float pitchDeg) {
  if (!_data.has_angle) {
    return false;
  }

  const float target = normalize180(pitchDeg);
  _data.pitch_offset_deg =
      normalize180(_data.pitch_raw_deg - target);
  applyAngleOffsets();
  return true;
}

bool TungLamHWT901B::setCurrentAngles(
    float rollDeg,
    float pitchDeg,
    float yawDeg) {
  if (!_data.has_angle) {
    return false;
  }

  _data.roll_offset_deg =
      normalize180(_data.roll_raw_deg - normalize180(rollDeg));
  _data.pitch_offset_deg =
      normalize180(_data.pitch_raw_deg - normalize180(pitchDeg));
  _data.yaw_offset_deg =
      normalize180(_data.yaw_raw_deg - normalize180(yawDeg));

  applyAngleOffsets();
  return true;
}

void TungLamHWT901B::clearAngleOffsets() {
  _data.roll_offset_deg = 0.0f;
  _data.pitch_offset_deg = 0.0f;
  _data.yaw_offset_deg = 0.0f;
  applyAngleOffsets();
}

void TungLamHWT901B::clearYawOffset() {
  _data.yaw_offset_deg = 0.0f;
  applyAngleOffsets();
}

void TungLamHWT901B::setYawOffset(float offsetDeg) {
  _data.yaw_offset_deg = normalize180(offsetDeg);
  applyAngleOffsets();
}

float TungLamHWT901B::getYawOffset() const {
  return _data.yaw_offset_deg;
}

bool TungLamHWT901B::sendCommand(uint8_t address, uint16_t value) {
  if (_stream == nullptr) {
    return false;
  }

  uint8_t command[5];
  command[0] = 0xFF;
  command[1] = 0xAA;
  command[2] = address;
  command[3] = static_cast<uint8_t>(value & 0xFFu);
  command[4] = static_cast<uint8_t>((value >> 8) & 0xFFu);

  return _stream->write(command, sizeof(command)) == sizeof(command);
}

bool TungLamHWT901B::unlock() {
  return sendCommand(REG_KEY, KEY_UNLOCK);
}

bool TungLamHWT901B::save() {
  return sendCommand(REG_SAVE, 0x0000);
}

bool TungLamHWT901B::writeRegisterUnlocked(
    uint8_t address,
    uint16_t value) {
  return sendCommand(address, value);
}

bool TungLamHWT901B::writeRegister(
    uint8_t address,
    uint16_t value,
    bool saveAfter) {
  if (!unlock()) {
    return false;
  }

  if (!writeRegisterUnlocked(address, value)) {
    return false;
  }

  if (saveAfter) {
    return save();
  }

  return true;
}

bool TungLamHWT901B::sensorZeroHeading() {
  if (!writeRegister(REG_CALSW, 0x0004, true)) {
    return false;
  }

  clearYawOffset();
  return true;
}

bool TungLamHWT901B::setOutput(uint16_t outputMask) {
  return writeRegister(REG_RSW, outputMask, true);
}

bool TungLamHWT901B::setRate(TungLamHWT901BRate rate) {
  return writeRegister(
      REG_RRATE,
      static_cast<uint16_t>(static_cast<uint8_t>(rate)),
      true);
}

bool TungLamHWT901B::setBandwidth(
    TungLamHWT901BBandwidth bandwidth) {
  return writeRegister(
      REG_BANDWIDTH,
      static_cast<uint16_t>(static_cast<uint8_t>(bandwidth)),
      true);
}

bool TungLamHWT901B::setAlgorithm(
    TungLamHWT901BAlgorithm algorithm) {
  return writeRegister(
      REG_AXIS6,
      static_cast<uint16_t>(static_cast<uint8_t>(algorithm)),
      true);
}

bool TungLamHWT901B::setOrientation(
    TungLamHWT901BOrientation orientation) {
  return writeRegister(
      REG_ORIENT,
      static_cast<uint16_t>(static_cast<uint8_t>(orientation)),
      true);
}

bool TungLamHWT901B::setLed(bool on) {
  return writeRegister(REG_LEDOFF, on ? 0x0000 : 0x0001, true);
}

bool TungLamHWT901B::configureRobotMode(TungLamHWT901BRate rate) {
  if (_stream == nullptr) {
    return false;
  }

  const uint16_t output =
      HWT901B_OUT_ACCEL |
      HWT901B_OUT_GYRO |
      HWT901B_OUT_ANGLE;

  if (!unlock()) {
    return false;
  }

  if (!writeRegisterUnlocked(REG_RSW, output)) {
    return false;
  }

  if (!writeRegisterUnlocked(
          REG_RRATE,
          static_cast<uint16_t>(static_cast<uint8_t>(rate)))) {
    return false;
  }

  return save();
}

bool TungLamHWT901B::setSensorBaud(TungLamHWT901BBaud baud) {
  return writeRegister(
      REG_BAUD,
      static_cast<uint16_t>(static_cast<uint8_t>(baud)),
      true);
}

uint32_t TungLamHWT901B::baudValue(TungLamHWT901BBaud baud) {
  switch (baud) {
    case TungLamHWT901BBaud::Baud4800:
      return 4800UL;
    case TungLamHWT901BBaud::Baud9600:
      return 9600UL;
    case TungLamHWT901BBaud::Baud19200:
      return 19200UL;
    case TungLamHWT901BBaud::Baud38400:
      return 38400UL;
    case TungLamHWT901BBaud::Baud57600:
      return 57600UL;
    case TungLamHWT901BBaud::Baud115200:
      return 115200UL;
    case TungLamHWT901BBaud::Baud230400:
      return 230400UL;
    default:
      return 0UL;
  }
}

bool TungLamHWT901B::requestRegister(uint8_t address) {
  if (_stream == nullptr) {
    return false;
  }

  _lastRegisterBase = address;
  _registerResponseReady = false;
  return sendCommand(REG_READADDR, static_cast<uint16_t>(address));
}

bool TungLamHWT901B::hasRegisterResponse() const {
  return _registerResponseReady;
}

uint8_t TungLamHWT901B::lastRegisterBase() const {
  return _lastRegisterBase;
}

int16_t TungLamHWT901B::lastRegisterValue(uint8_t index) const {
  if (index >= 4) {
    return 0;
  }

  return _lastRegisterValues[index];
}

uint32_t TungLamHWT901B::frameCount() const {
  return _data.valid_frames;
}

uint32_t TungLamHWT901B::checksumErrorCount() const {
  return _data.checksum_errors;
}

uint32_t TungLamHWT901B::unknownFrameCount() const {
  return _data.unknown_frames;
}

uint32_t TungLamHWT901B::byteCount() const {
  return _data.bytes_received;
}

void TungLamHWT901B::clearStatistics() {
  _data.valid_frames = 0;
  _data.checksum_errors = 0;
  _data.unknown_frames = 0;
  _data.bytes_received = 0;
}

float TungLamHWT901B::normalize180(float deg) {
  while (deg > 180.0f) {
    deg -= 360.0f;
  }

  while (deg <= -180.0f) {
    deg += 360.0f;
  }

  return deg;
}

float TungLamHWT901B::normalize360(float deg) {
  while (deg >= 360.0f) {
    deg -= 360.0f;
  }

  while (deg < 0.0f) {
    deg += 360.0f;
  }

  return deg;
}

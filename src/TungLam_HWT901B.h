#pragma once

#include <Arduino.h>
#include <Stream.h>

/**
 * @file TungLam_HWT901B.h
 * @brief Thu vien UART non-blocking cho WIT Motion HWT901B TTL.
 *
 * Thiet ke:
 * - Khong delay(), khong readBytes(), khong cap phat dong.
 * - Parser frame WIT 11 byte + checksum + resync.
 * - Ho tro HardwareSerial va cac Stream co begin(baud), vi du SoftwareSerial.
 * - Tu quan ly offset goc de zero/dat goc hien tai thanh moc bat ky.
 * - Co register layer cho cac cau hinh WIT Standard Protocol.
 *
 * Tac gia: Nguyen Khac Tung Lam - Tung Lam Automation
 */

enum TungLamHWT901BOutput : uint16_t {
  HWT901B_OUT_TIME       = (1u << 0),
  HWT901B_OUT_ACCEL      = (1u << 1),
  HWT901B_OUT_GYRO       = (1u << 2),
  HWT901B_OUT_ANGLE      = (1u << 3),
  HWT901B_OUT_MAG        = (1u << 4),
  HWT901B_OUT_PORT       = (1u << 5),
  HWT901B_OUT_PRESSURE   = (1u << 6),
  HWT901B_OUT_GPS        = (1u << 7),
  HWT901B_OUT_VELOCITY   = (1u << 8),
  HWT901B_OUT_QUATERNION = (1u << 9),
  HWT901B_OUT_GPS_ACC    = (1u << 10)
};

enum class TungLamHWT901BRate : uint8_t {
  Hz0_2 = 0x01,
  Hz0_5 = 0x02,
  Hz1   = 0x03,
  Hz2   = 0x04,
  Hz5   = 0x05,
  Hz10  = 0x06,
  Hz20  = 0x07,
  Hz50  = 0x08,
  Hz100 = 0x09,
  Hz200 = 0x0B
};

enum class TungLamHWT901BBaud : uint8_t {
  Baud4800   = 0x01,
  Baud9600   = 0x02,
  Baud19200  = 0x03,
  Baud38400  = 0x04,
  Baud57600  = 0x05,
  Baud115200 = 0x06,
  Baud230400 = 0x07
};

enum class TungLamHWT901BBandwidth : uint8_t {
  Hz256 = 0x00,
  Hz188 = 0x01,
  Hz98  = 0x02,
  Hz42  = 0x03,
  Hz20  = 0x04,
  Hz10  = 0x05,
  Hz5   = 0x06
};

enum class TungLamHWT901BAlgorithm : uint8_t {
  Axis9 = 0x00,
  Axis6 = 0x01
};

enum class TungLamHWT901BOrientation : uint8_t {
  Horizontal = 0x00,
  Vertical   = 0x01
};

/** @brief Snapshot du lieu IMU da duoc decode. */
struct TungLamHWT901BData {
  float ax_g;
  float ay_g;
  float az_g;
  float ax_mps2;
  float ay_mps2;
  float az_mps2;

  float gx_dps;
  float gy_dps;
  float gz_dps;
  float gx_rad_s;
  float gy_rad_s;
  float gz_rad_s;

  float roll_raw_deg;
  float pitch_raw_deg;
  float yaw_raw_deg;

  float roll_deg;
  float pitch_deg;
  float yaw_deg;

  float roll_offset_deg;
  float pitch_offset_deg;
  float yaw_offset_deg;

  int16_t mx;
  int16_t my;
  int16_t mz;

  float q0;
  float q1;
  float q2;
  float q3;

  float temperature_c;
  float voltage_v;
  uint16_t version;

  uint32_t last_frame_ms;
  uint32_t last_accel_ms;
  uint32_t last_gyro_ms;
  uint32_t last_angle_ms;
  uint32_t last_mag_ms;
  uint32_t last_quaternion_ms;

  uint32_t valid_frames;
  uint32_t checksum_errors;
  uint32_t unknown_frames;
  uint32_t bytes_received;

  bool has_accel;
  bool has_gyro;
  bool has_angle;
  bool has_mag;
  bool has_quaternion;
};

class TungLamHWT901B {
 public:
  TungLamHWT901B();

  /**
   * @brief Khoi tao IMU va tu khoi tao UART.
   * @tparam SerialT HardwareSerial, SoftwareSerial hoac class tuong thich Stream co begin(baud).
   * @param serial Cong serial dung de giao tiep IMU.
   * @param baud Baud rate host va IMU dang su dung.
   * @return true neu da gan stream thanh cong.
   *
   * Vi du:
   *   imu.begin(Serial1, 115200);
   */
  template <typename SerialT>
  bool begin(SerialT &serial, uint32_t baud = 9600UL) {
    serial.begin(baud);
    attach(serial);
    _baud = baud;
    return true;
  }

  /**
   * @brief Gan mot Stream da duoc khoi tao boi chuong trinh chinh.
   * @param stream Stream UART da begin truoc do.
   */
  void attach(Stream &stream);

  /**
   * @brief Doc het byte hien co va cap nhat parser, khong blocking.
   * @return true neu lan goi nay decode duoc it nhat mot frame hop le.
   */
  bool update();

  /** @brief IMU da co it nhat mot frame angle 0x53 hay chua. */
  bool hasAngle() const;

  /** @brief Kiem tra co frame hop le gan day hay khong. */
  bool connected(uint32_t timeoutMs = 500UL) const;

  /** @brief Tuoi cua frame hop le cuoi cung, don vi ms. */
  uint32_t ageMs() const;

  /** @brief Lay snapshot day du. */
  bool getData(TungLamHWT901BData &out) const;

  float getRoll() const;
  float getPitch() const;
  float getYaw() const;
  float getYaw360() const;

  float getRawRoll() const;
  float getRawPitch() const;
  float getRawYaw() const;

  float getAccelX() const;
  float getAccelY() const;
  float getAccelZ() const;
  float getAccelXg() const;
  float getAccelYg() const;
  float getAccelZg() const;

  float getGyroX() const;
  float getGyroY() const;
  float getGyroZ() const;
  float getGyroXRad() const;
  float getGyroYRad() const;
  float getGyroZRad() const;

  int16_t getMagX() const;
  int16_t getMagY() const;
  int16_t getMagZ() const;

  float getQ0() const;
  float getQ1() const;
  float getQ2() const;
  float getQ3() const;

  float getTemperature() const;
  float getVoltage() const;

  /**
   * @brief Dat yaw hien tai thanh 0 do bang software offset.
   * @return false neu chua nhan duoc frame angle.
   */
  bool zeroYaw();

  /** @brief Dat roll hien tai thanh 0 do bang software offset. */
  bool zeroRoll();

  /** @brief Dat pitch hien tai thanh 0 do bang software offset. */
  bool zeroPitch();

  /**
   * @brief Dat yaw hien tai thanh mot goc bat ky.
   * @param yawDeg Goc mong muon tai vi tri hien tai, vi du 180.0f.
   * @return false neu chua nhan duoc frame angle.
   *
   * Vi du: sensor dang doc 89 do, goi setCurrentYaw(180) thi getYaw()
   * se tra 180 do tai chinh tu the do. Offset duoc library tu quan ly.
   */
  bool setCurrentYaw(float yawDeg);

  /** @brief Dat roll hien tai thanh mot goc bat ky. */
  bool setCurrentRoll(float rollDeg);

  /** @brief Dat pitch hien tai thanh mot goc bat ky. */
  bool setCurrentPitch(float pitchDeg);

  /** @brief Dat dong thoi moc roll/pitch/yaw hien tai. */
  bool setCurrentAngles(float rollDeg, float pitchDeg, float yawDeg);

  /** @brief Xoa toan bo software angle offset. */
  void clearAngleOffsets();

  /** @brief Xoa rieng yaw offset. */
  void clearYawOffset();

  /** @brief Dat truc tiep software yaw offset. */
  void setYawOffset(float offsetDeg);

  /** @brief Lay software yaw offset hien tai. */
  float getYawOffset() const;

  /**
   * @brief Zero heading tren chinh IMU bang CALSW=0x04.
   * @return false neu chua gan Stream.
   *
   * Khac zeroYaw(): ham nay gui lenh WIT vao sensor.
   */
  bool sensorZeroHeading();

  /** @brief Cau hinh mask frame output cua sensor (register RSW 0x02). */
  bool setOutput(uint16_t outputMask);

  /** @brief Cau hinh tan so output cua sensor (register RRATE 0x03). */
  bool setRate(TungLamHWT901BRate rate);

  /** @brief Cau hinh bandwidth cua sensor (register 0x1F). */
  bool setBandwidth(TungLamHWT901BBandwidth bandwidth);

  /** @brief Chon thuat toan 9-axis hoac 6-axis (register AXIS6 0x24). */
  bool setAlgorithm(TungLamHWT901BAlgorithm algorithm);

  /** @brief Chon lap ngang/doc (register ORIENT 0x23). */
  bool setOrientation(TungLamHWT901BOrientation orientation);

  /** @brief Bat/tat LED tren sensor neu model/firmware ho tro. */
  bool setLed(bool on);

  /**
   * @brief Cau hinh nhanh cho robot: ACC + GYRO + ANGLE va output rate.
   * @return false neu chua gan Stream.
   */
  bool configureRobotMode(TungLamHWT901BRate rate = TungLamHWT901BRate::Hz100);

  /**
   * @brief Doi baud phia sensor. Sau lenh nay host UART phai begin lai dung baud moi.
   */
  bool setSensorBaud(TungLamHWT901BBaud baud);

  /** @brief Chuyen enum baud thanh gia tri baud thuc. */
  static uint32_t baudValue(TungLamHWT901BBaud baud);

  /**
   * @brief Ghi mot register WIT theo unlock -> write -> save.
   * @param address Dia chi register.
   * @param value Gia tri 16 bit.
   * @param saveAfter true de gui SAVE sau khi ghi.
   */
  bool writeRegister(uint8_t address, uint16_t value, bool saveAfter = true);

  /**
   * @brief Yeu cau doc register bang READADDR 0x27.
   * Frame 0x5F nhan duoc sau do duoc luu vao lastRegisterValue().
   */
  bool requestRegister(uint8_t address);

  /** @brief Co response register 0x5F ke tu request gan nhat hay khong. */
  bool hasRegisterResponse() const;

  /** @brief Dia chi bat dau cua request register gan nhat. */
  uint8_t lastRegisterBase() const;

  /**
   * @brief Lay word thu index trong frame register response.
   * @param index 0..3.
   */
  int16_t lastRegisterValue(uint8_t index) const;

  uint32_t frameCount() const;
  uint32_t checksumErrorCount() const;
  uint32_t unknownFrameCount() const;
  uint32_t byteCount() const;

  /** @brief Reset counter debug, khong xoa du lieu attitude/offset. */
  void clearStatistics();

  /** @brief Normalize goc ve mien (-180, 180]. */
  static float normalize180(float deg);

  /** @brief Normalize goc ve mien [0, 360). */
  static float normalize360(float deg);

 private:
  static const uint8_t FRAME_HEAD = 0x55;
  static const uint8_t FRAME_SIZE = 11;

  static const uint8_t FRAME_ACCEL = 0x51;
  static const uint8_t FRAME_GYRO = 0x52;
  static const uint8_t FRAME_ANGLE = 0x53;
  static const uint8_t FRAME_MAG = 0x54;
  static const uint8_t FRAME_QUATERNION = 0x59;
  static const uint8_t FRAME_REGISTER = 0x5F;

  static const uint8_t REG_SAVE = 0x00;
  static const uint8_t REG_CALSW = 0x01;
  static const uint8_t REG_RSW = 0x02;
  static const uint8_t REG_RRATE = 0x03;
  static const uint8_t REG_BAUD = 0x04;
  static const uint8_t REG_LEDOFF = 0x1B;
  static const uint8_t REG_BANDWIDTH = 0x1F;
  static const uint8_t REG_ORIENT = 0x23;
  static const uint8_t REG_AXIS6 = 0x24;
  static const uint8_t REG_READADDR = 0x27;
  static const uint8_t REG_KEY = 0x69;

  static const uint16_t KEY_UNLOCK = 0xB588;
  static const float GRAVITY_MPS2;
  static const float DEG_TO_RAD_F;

  Stream *_stream;
  uint32_t _baud;

  uint8_t _frame[FRAME_SIZE];
  uint8_t _frameIndex;

  TungLamHWT901BData _data;

  uint8_t _lastRegisterBase;
  int16_t _lastRegisterValues[4];
  bool _registerResponseReady;

  void resetRuntime();
  bool feedByte(uint8_t value);
  void resyncAfterBadFrame();
  void decodeFrame(const uint8_t *frame);

  static int16_t readI16(const uint8_t *p);
  static uint16_t readU16(const uint8_t *p);

  void applyAngleOffsets();
  bool sendCommand(uint8_t address, uint16_t value);
  bool unlock();
  bool save();
  bool writeRegisterUnlocked(uint8_t address, uint16_t value);
};

// Alias ngan gon cho sketch Arduino thong thuong.
typedef TungLamHWT901B HWT901B;

#pragma once

#include <Arduino.h>
#include <Stream.h>

/**
 * @file TungLam_HWT901B.h
 * @brief Thư viện UART không chặn dành cho cảm biến WIT Motion HWT901B TTL.
 *
 * TungLam_HWT901B được thiết kế để dùng trực tiếp trong các dự án robot Arduino:
 *
 * - Không dùng delay(), không dùng readBytes() và không cấp phát động trên heap.
 * - Đọc luồng UART theo từng byte, kiểm tra checksum và tự đồng bộ lại khi lệch khung.
 * - Hỗ trợ HardwareSerial, SoftwareSerial và các lớp tương thích Arduino Stream.
 * - Giải mã gia tốc, vận tốc góc, góc Euler, từ trường, quaternion và phản hồi thanh ghi.
 * - Tự quản lý độ lệch góc để zero hoặc gán tư thế hiện tại thành một góc bất kỳ.
 * - Cung cấp lớp lệnh thanh ghi WIT để cấu hình tần số xuất, baud, thuật toán, LED...
 *
 * Cách dùng cơ bản:
 *
 * @code
 * HWT901B imu;
 *
 * void setup() {
 *   imu.begin(Serial1);  // HWT901B mặc định thường dùng 9600 baud.
 * }
 *
 * void loop() {
 *   imu.update();        // Gọi càng thường xuyên càng tốt.
 *
 *   if (imu.hasAngle()) {
 *     float yaw = imu.getYaw();
 *   }
 * }
 * @endcode
 *
 * @author Nguyễn Khắc Tùng Lâm - Tùng Lâm Automation
 */

/**
 * @brief Mặt nạ chọn các nhóm dữ liệu mà HWT901B phát tuần hoàn.
 *
 * Các bit này được ghép bằng toán tử OR rồi truyền vào setOutput().
 * Giá trị bit tuân theo thanh ghi RSW của WIT Standard Protocol.
 */
enum TungLamHWT901BOutput : uint16_t {
  HWT901B_OUT_TIME       = (1u << 0),   ///< Xuất dữ liệu thời gian.
  HWT901B_OUT_ACCEL      = (1u << 1),   ///< Xuất gia tốc, frame 0x51.
  HWT901B_OUT_GYRO       = (1u << 2),   ///< Xuất vận tốc góc, frame 0x52.
  HWT901B_OUT_ANGLE      = (1u << 3),   ///< Xuất Roll/Pitch/Yaw, frame 0x53.
  HWT901B_OUT_MAG        = (1u << 4),   ///< Xuất từ trường, frame 0x54.
  HWT901B_OUT_PORT       = (1u << 5),   ///< Xuất trạng thái cổng phụ nếu firmware hỗ trợ.
  HWT901B_OUT_PRESSURE   = (1u << 6),   ///< Xuất áp suất nếu phần cứng hỗ trợ.
  HWT901B_OUT_GPS        = (1u << 7),   ///< Xuất dữ liệu GPS nếu thiết bị hỗ trợ.
  HWT901B_OUT_VELOCITY   = (1u << 8),   ///< Xuất vận tốc GPS nếu thiết bị hỗ trợ.
  HWT901B_OUT_QUATERNION = (1u << 9),   ///< Xuất quaternion, frame 0x59.
  HWT901B_OUT_GPS_ACC    = (1u << 10)   ///< Xuất độ chính xác GPS nếu thiết bị hỗ trợ.
};

/**
 * @brief Các mức tần số xuất dữ liệu của cảm biến.
 *
 * Giá trị enum chính là mã ghi vào thanh ghi RRATE 0x03 theo giao thức WIT.
 * Khả năng hỗ trợ mức cao nhất phụ thuộc model và firmware của cảm biến.
 */
enum class TungLamHWT901BRate : uint8_t {
  Hz0_2 = 0x01,  ///< 0,2 Hz.
  Hz0_5 = 0x02,  ///< 0,5 Hz.
  Hz1   = 0x03,  ///< 1 Hz.
  Hz2   = 0x04,  ///< 2 Hz.
  Hz5   = 0x05,  ///< 5 Hz.
  Hz10  = 0x06,  ///< 10 Hz.
  Hz20  = 0x07,  ///< 20 Hz.
  Hz50  = 0x08,  ///< 50 Hz.
  Hz100 = 0x09,  ///< 100 Hz.
  Hz200 = 0x0B   ///< 200 Hz nếu model/firmware hỗ trợ.
};

/**
 * @brief Các mức baud có thể cấu hình cho phía cảm biến WIT.
 *
 * Lưu ý: setSensorBaud() chỉ đổi baud bên cảm biến. Sau khi đổi,
 * chương trình phải khởi tạo lại UART phía Arduino bằng đúng baud mới.
 */
enum class TungLamHWT901BBaud : uint8_t {
  Baud4800   = 0x01,  ///< 4800 bit/s.
  Baud9600   = 0x02,  ///< 9600 bit/s, thường là mặc định của WIT.
  Baud19200  = 0x03,  ///< 19200 bit/s.
  Baud38400  = 0x04,  ///< 38400 bit/s.
  Baud57600  = 0x05,  ///< 57600 bit/s.
  Baud115200 = 0x06,  ///< 115200 bit/s, phù hợp khi xuất nhiều frame ở tần số cao.
  Baud230400 = 0x07   ///< 230400 bit/s nếu model/firmware hỗ trợ.
};

/**
 * @brief Băng thông bộ lọc số bên trong cảm biến.
 *
 * Giá trị enum được ghi vào thanh ghi BANDWIDTH 0x1F.
 * Băng thông thấp hơn thường giúp dữ liệu mượt hơn nhưng làm đáp ứng chậm hơn.
 */
enum class TungLamHWT901BBandwidth : uint8_t {
  Hz256 = 0x00,  ///< Băng thông 256 Hz.
  Hz188 = 0x01,  ///< Băng thông 188 Hz.
  Hz98  = 0x02,  ///< Băng thông 98 Hz.
  Hz42  = 0x03,  ///< Băng thông 42 Hz.
  Hz20  = 0x04,  ///< Băng thông 20 Hz.
  Hz10  = 0x05,  ///< Băng thông 10 Hz.
  Hz5   = 0x06   ///< Băng thông 5 Hz.
};

/**
 * @brief Chế độ thuật toán hợp nhất cảm biến.
 */
enum class TungLamHWT901BAlgorithm : uint8_t {
  Axis9 = 0x00,  ///< Thuật toán 9 trục, có sử dụng từ kế để hỗ trợ heading.
  Axis6 = 0x01   ///< Thuật toán 6 trục, không sử dụng từ kế cho heading.
};

/**
 * @brief Tư thế lắp đặt cảm biến mà firmware WIT sử dụng.
 */
enum class TungLamHWT901BOrientation : uint8_t {
  Horizontal = 0x00,  ///< Lắp ngang.
  Vertical   = 0x01   ///< Lắp đứng.
};

/**
 * @brief Ảnh chụp đầy đủ trạng thái dữ liệu mới nhất đã giải mã từ HWT901B.
 *
 * Cấu trúc này phù hợp khi cần lấy đồng thời nhiều đại lượng mà không phải
 * gọi hàng loạt getter. Mỗi nhóm dữ liệu có cờ has_* và thời điểm cập nhật
 * riêng để chương trình biết nhóm đó đã từng nhận được frame hợp lệ hay chưa.
 */
struct TungLamHWT901BData {
  float ax_g;       ///< Gia tốc trục X, đơn vị g.
  float ay_g;       ///< Gia tốc trục Y, đơn vị g.
  float az_g;       ///< Gia tốc trục Z, đơn vị g.
  float ax_mps2;    ///< Gia tốc trục X, đơn vị m/s².
  float ay_mps2;    ///< Gia tốc trục Y, đơn vị m/s².
  float az_mps2;    ///< Gia tốc trục Z, đơn vị m/s².

  float gx_dps;     ///< Vận tốc góc trục X, đơn vị độ/giây.
  float gy_dps;     ///< Vận tốc góc trục Y, đơn vị độ/giây.
  float gz_dps;     ///< Vận tốc góc trục Z, đơn vị độ/giây.
  float gx_rad_s;   ///< Vận tốc góc trục X, đơn vị rad/s.
  float gy_rad_s;   ///< Vận tốc góc trục Y, đơn vị rad/s.
  float gz_rad_s;   ///< Vận tốc góc trục Z, đơn vị rad/s.

  float roll_raw_deg;   ///< Roll gốc từ cảm biến trước khi bù offset, đơn vị độ.
  float pitch_raw_deg;  ///< Pitch gốc từ cảm biến trước khi bù offset, đơn vị độ.
  float yaw_raw_deg;    ///< Yaw gốc từ cảm biến trước khi bù offset, đơn vị độ.

  float roll_deg;       ///< Roll sau khi áp dụng offset phần mềm, đơn vị độ.
  float pitch_deg;      ///< Pitch sau khi áp dụng offset phần mềm, đơn vị độ.
  float yaw_deg;        ///< Yaw sau khi áp dụng offset phần mềm, đơn vị độ.

  float roll_offset_deg;   ///< Offset phần mềm hiện tại của Roll, đơn vị độ.
  float pitch_offset_deg;  ///< Offset phần mềm hiện tại của Pitch, đơn vị độ.
  float yaw_offset_deg;    ///< Offset phần mềm hiện tại của Yaw, đơn vị độ.

  int16_t mx;  ///< Từ trường thô trục X theo frame WIT.
  int16_t my;  ///< Từ trường thô trục Y theo frame WIT.
  int16_t mz;  ///< Từ trường thô trục Z theo frame WIT.

  float q0;  ///< Thành phần q0 của quaternion.
  float q1;  ///< Thành phần q1 của quaternion.
  float q2;  ///< Thành phần q2 của quaternion.
  float q3;  ///< Thành phần q3 của quaternion.

  float temperature_c;  ///< Nhiệt độ mới nhất, đơn vị °C.
  float voltage_v;      ///< Điện áp mới nhất theo frame gyro WIT, đơn vị V.
  uint16_t version;     ///< Giá trị version 16 bit đi kèm frame góc.

  uint32_t last_frame_ms;       ///< millis() tại frame hợp lệ gần nhất.
  uint32_t last_accel_ms;       ///< millis() tại frame gia tốc gần nhất.
  uint32_t last_gyro_ms;        ///< millis() tại frame gyro gần nhất.
  uint32_t last_angle_ms;       ///< millis() tại frame góc gần nhất.
  uint32_t last_mag_ms;         ///< millis() tại frame từ trường gần nhất.
  uint32_t last_quaternion_ms;  ///< millis() tại frame quaternion gần nhất.

  uint32_t valid_frames;     ///< Tổng số frame hợp lệ đã giải mã.
  uint32_t checksum_errors;  ///< Tổng số frame sai checksum.
  uint32_t unknown_frames;   ///< Tổng số frame hợp lệ nhưng chưa có bộ giải mã riêng.
  uint32_t bytes_received;   ///< Tổng số byte đã đọc khỏi Stream.

  bool has_accel;       ///< true sau khi đã nhận ít nhất một frame 0x51 hợp lệ.
  bool has_gyro;        ///< true sau khi đã nhận ít nhất một frame 0x52 hợp lệ.
  bool has_angle;       ///< true sau khi đã nhận ít nhất một frame 0x53 hợp lệ.
  bool has_mag;         ///< true sau khi đã nhận ít nhất một frame 0x54 hợp lệ.
  bool has_quaternion;  ///< true sau khi đã nhận ít nhất một frame 0x59 hợp lệ.
};

/**
 * @brief Lớp điều khiển và giải mã dữ liệu HWT901B qua UART.
 *
 * Đối tượng không chiếm timer, không tạo ISR riêng và không tự gọi delay().
 * Hàm update() cần được gọi liên tục trong loop() hoặc task sở hữu UART.
 */
class TungLamHWT901B {
 public:
  /**
   * @brief Tạo đối tượng IMU ở trạng thái chưa gắn cổng UART.
   *
   * Constructor chỉ khởi tạo trạng thái nội bộ, không truy cập phần cứng.
   * Gọi begin() hoặc attach() trước khi dùng update() hay gửi lệnh cấu hình.
   */
  TungLamHWT901B();

  /**
   * @brief Khởi tạo cổng UART và gắn cổng đó vào thư viện.
   *
   * Hàm này gọi serial.begin(baud), sau đó dùng chính đối tượng serial như
   * một Stream để đọc/ghi dữ liệu. Đây là cách dùng đơn giản nhất.
   *
   * @tparam SerialT Kiểu HardwareSerial, SoftwareSerial hoặc lớp tương thích
   *                 Stream có hàm begin(baud).
   * @param serial Đối tượng UART dùng để giao tiếp với HWT901B.
   * @param baud Baud mà cả Arduino và HWT901B đang sử dụng.
   *             Mặc định 9600 bit/s.
   * @return Luôn trả true sau khi đã gắn Stream.
   *
   * @code
   * imu.begin(Serial1);          // 9600 baud.
   * imu.begin(Serial1, 115200);  // Khi cảm biến đã được đặt 115200 baud.
   * @endcode
   */
  template <typename SerialT>
  bool begin(SerialT &serial, uint32_t baud = 9600UL) {
    serial.begin(baud);
    attach(serial);
    _baud = baud;
    return true;
  }

  /**
   * @brief Gắn một Stream đã được chương trình khởi tạo sẵn.
   *
   * Dùng khi phần khác của chương trình chịu trách nhiệm gọi begin() cho UART.
   * attach() không thay đổi baud của phần cứng và sẽ xóa trạng thái runtime cũ.
   *
   * @param stream Stream UART đã được khởi tạo trước đó.
   */
  void attach(Stream &stream);

  /**
   * @brief Đọc tất cả byte hiện đang có trong UART và cập nhật bộ phân tích.
   *
   * Hàm không chờ byte mới đến, không dùng delay() và không dùng readBytes().
   * Có thể gọi liên tục ở mỗi vòng loop(). Nếu trong một lần gọi có nhiều
   * frame hoàn chỉnh, thư viện sẽ xử lý hết các frame đang nằm trong buffer UART.
   *
   * @return true nếu lần gọi này giải mã được ít nhất một frame hợp lệ;
   *         false nếu chưa có frame hoàn chỉnh hoặc chưa gắn Stream.
   */
  bool update();

  /**
   * @brief Kiểm tra đã từng nhận được frame góc 0x53 hợp lệ hay chưa.
   * @return true sau frame góc hợp lệ đầu tiên; ngược lại false.
   */
  bool hasAngle() const;

  /**
   * @brief Kiểm tra HWT901B có còn gửi dữ liệu hợp lệ trong khoảng thời gian cho phép.
   * @param timeoutMs Thời gian tối đa tính từ frame hợp lệ cuối, đơn vị ms.
   * @return true nếu đã từng có frame hợp lệ và tuổi frame không vượt timeoutMs.
   */
  bool connected(uint32_t timeoutMs = 500UL) const;

  /**
   * @brief Lấy tuổi của frame hợp lệ gần nhất.
   * @return Số mili giây từ frame gần nhất đến hiện tại.
   *         Trả 0xFFFFFFFF nếu chưa từng nhận frame hợp lệ.
   */
  uint32_t ageMs() const;

  /**
   * @brief Sao chép toàn bộ snapshot dữ liệu hiện tại ra biến người dùng.
   * @param out Biến nhận TungLamHWT901BData.
   * @return false nếu chưa có bất kỳ frame hợp lệ nào; true nếu đã sao chép dữ liệu.
   */
  bool getData(TungLamHWT901BData &out) const;

  /** @brief Lấy Roll đã bù offset. @return Góc Roll trong miền (-180, 180], đơn vị độ. */
  float getRoll() const;

  /** @brief Lấy Pitch đã bù offset. @return Góc Pitch trong miền (-180, 180], đơn vị độ. */
  float getPitch() const;

  /** @brief Lấy Yaw đã bù offset. @return Góc Yaw trong miền (-180, 180], đơn vị độ. */
  float getYaw() const;

  /** @brief Lấy Yaw đã bù offset theo miền dương. @return Góc Yaw trong miền [0, 360), đơn vị độ. */
  float getYaw360() const;

  /** @brief Lấy Roll gốc từ cảm biến, chưa bù offset phần mềm. */
  float getRawRoll() const;

  /** @brief Lấy Pitch gốc từ cảm biến, chưa bù offset phần mềm. */
  float getRawPitch() const;

  /** @brief Lấy Yaw gốc từ cảm biến, chưa bù offset phần mềm. */
  float getRawYaw() const;

  /** @brief Lấy gia tốc trục X. @return Giá trị m/s². */
  float getAccelX() const;

  /** @brief Lấy gia tốc trục Y. @return Giá trị m/s². */
  float getAccelY() const;

  /** @brief Lấy gia tốc trục Z. @return Giá trị m/s². */
  float getAccelZ() const;

  /** @brief Lấy gia tốc trục X theo đơn vị g. */
  float getAccelXg() const;

  /** @brief Lấy gia tốc trục Y theo đơn vị g. */
  float getAccelYg() const;

  /** @brief Lấy gia tốc trục Z theo đơn vị g. */
  float getAccelZg() const;

  /** @brief Lấy vận tốc góc trục X. @return Giá trị độ/giây. */
  float getGyroX() const;

  /** @brief Lấy vận tốc góc trục Y. @return Giá trị độ/giây. */
  float getGyroY() const;

  /** @brief Lấy vận tốc góc trục Z. @return Giá trị độ/giây. */
  float getGyroZ() const;

  /** @brief Lấy vận tốc góc trục X. @return Giá trị rad/s. */
  float getGyroXRad() const;

  /** @brief Lấy vận tốc góc trục Y. @return Giá trị rad/s. */
  float getGyroYRad() const;

  /** @brief Lấy vận tốc góc trục Z. @return Giá trị rad/s. */
  float getGyroZRad() const;

  /** @brief Lấy giá trị từ trường thô trục X từ frame 0x54. */
  int16_t getMagX() const;

  /** @brief Lấy giá trị từ trường thô trục Y từ frame 0x54. */
  int16_t getMagY() const;

  /** @brief Lấy giá trị từ trường thô trục Z từ frame 0x54. */
  int16_t getMagZ() const;

  /** @brief Lấy thành phần q0 của quaternion mới nhất. */
  float getQ0() const;

  /** @brief Lấy thành phần q1 của quaternion mới nhất. */
  float getQ1() const;

  /** @brief Lấy thành phần q2 của quaternion mới nhất. */
  float getQ2() const;

  /** @brief Lấy thành phần q3 của quaternion mới nhất. */
  float getQ3() const;

  /** @brief Lấy nhiệt độ mới nhất. @return Nhiệt độ theo °C. */
  float getTemperature() const;

  /** @brief Lấy điện áp mới nhất giải mã từ frame gyro. @return Điện áp theo V. */
  float getVoltage() const;

  /**
   * @brief Đặt Yaw tại chính tư thế hiện tại thành 0° bằng offset phần mềm.
   *
   * Hàm không ghi cấu hình vào HWT901B. Offset chỉ nằm trong RAM Arduino
   * và mất khi reset vi điều khiển.
   *
   * @return false nếu chưa nhận được frame góc; true nếu đã cập nhật offset.
   */
  bool zeroYaw();

  /**
   * @brief Đặt Roll tại chính tư thế hiện tại thành 0° bằng offset phần mềm.
   * @return false nếu chưa có frame góc; true nếu thành công.
   */
  bool zeroRoll();

  /**
   * @brief Đặt Pitch tại chính tư thế hiện tại thành 0° bằng offset phần mềm.
   * @return false nếu chưa có frame góc; true nếu thành công.
   */
  bool zeroPitch();

  /**
   * @brief Gán Yaw tại tư thế hiện tại thành một giá trị bất kỳ.
   *
   * Ví dụ cảm biến đang đọc Yaw thô khoảng 89°:
   *
   * @code
   * imu.setCurrentYaw(180.0f);
   * @endcode
   *
   * Sau lời gọi trên, getYaw() sẽ trả xấp xỉ 180° ngay tại tư thế đó,
   * trong khi getRawYaw() vẫn giữ xấp xỉ 89°. Thư viện tự tính:
   *
   * offset = rawYaw - targetYaw
   *
   * và tự xử lý việc quấn góc qua biên -180°/+180°.
   *
   * @param yawDeg Góc Yaw mong muốn tại tư thế hiện tại, đơn vị độ.
   * @return false nếu chưa nhận frame góc; true nếu đã cập nhật offset.
   */
  bool setCurrentYaw(float yawDeg);

  /**
   * @brief Gán Roll tại tư thế hiện tại thành một giá trị bất kỳ.
   * @param rollDeg Góc Roll mong muốn, đơn vị độ.
   * @return false nếu chưa nhận frame góc; true nếu thành công.
   */
  bool setCurrentRoll(float rollDeg);

  /**
   * @brief Gán Pitch tại tư thế hiện tại thành một giá trị bất kỳ.
   * @param pitchDeg Góc Pitch mong muốn, đơn vị độ.
   * @return false nếu chưa nhận frame góc; true nếu thành công.
   */
  bool setCurrentPitch(float pitchDeg);

  /**
   * @brief Gán đồng thời Roll, Pitch và Yaw hiện tại thành ba mốc mới.
   * @param rollDeg Mốc Roll mong muốn, đơn vị độ.
   * @param pitchDeg Mốc Pitch mong muốn, đơn vị độ.
   * @param yawDeg Mốc Yaw mong muốn, đơn vị độ.
   * @return false nếu chưa có frame góc; true nếu đã cập nhật cả ba offset.
   */
  bool setCurrentAngles(float rollDeg, float pitchDeg, float yawDeg);

  /**
   * @brief Xóa toàn bộ offset phần mềm của Roll, Pitch và Yaw.
   *
   * Sau khi gọi, các getter góc trở lại trùng với góc gốc của cảm biến
   * sau bước chuẩn hóa miền góc.
   */
  void clearAngleOffsets();

  /** @brief Xóa riêng offset phần mềm của Yaw. */
  void clearYawOffset();

  /**
   * @brief Đặt trực tiếp giá trị offset Yaw phần mềm.
   * @param offsetDeg Offset cần dùng, đơn vị độ; được chuẩn hóa về (-180, 180].
   */
  void setYawOffset(float offsetDeg);

  /** @brief Lấy offset Yaw phần mềm hiện tại, đơn vị độ. */
  float getYawOffset() const;

  /**
   * @brief Yêu cầu chính cảm biến thực hiện zero heading bằng CALSW=0x04.
   *
   * Khác với zeroYaw(), hàm này gửi lệnh cấu hình WIT xuống HWT901B và có
   * thể làm thay đổi mốc heading lưu trong cảm biến. Sau khi lệnh thành công,
   * offset Yaw phần mềm của thư viện được xóa.
   *
   * @return false nếu chưa gắn Stream hoặc không ghi đủ lệnh; ngược lại true.
   */
  bool sensorZeroHeading();

  /**
   * @brief Cấu hình các nhóm frame HWT901B được phép phát.
   * @param outputMask Mặt nạ ghép từ các giá trị HWT901B_OUT_*.
   * @return true khi chuỗi UNLOCK -> WRITE RSW -> SAVE đã được gửi đủ.
   */
  bool setOutput(uint16_t outputMask);

  /**
   * @brief Cấu hình tần số xuất dữ liệu của cảm biến qua thanh ghi RRATE 0x03.
   * @param rate Mức tần số cần dùng.
   * @return true khi chuỗi lệnh cấu hình đã được gửi đủ.
   */
  bool setRate(TungLamHWT901BRate rate);

  /**
   * @brief Cấu hình băng thông bộ lọc qua thanh ghi BANDWIDTH 0x1F.
   * @param bandwidth Mức băng thông cần dùng.
   * @return true khi chuỗi lệnh cấu hình đã được gửi đủ.
   */
  bool setBandwidth(TungLamHWT901BBandwidth bandwidth);

  /**
   * @brief Chọn thuật toán 9 trục hoặc 6 trục qua thanh ghi AXIS6 0x24.
   * @param algorithm Thuật toán cần dùng.
   * @return true khi chuỗi lệnh cấu hình đã được gửi đủ.
   */
  bool setAlgorithm(TungLamHWT901BAlgorithm algorithm);

  /**
   * @brief Cấu hình tư thế lắp ngang/đứng qua thanh ghi ORIENT 0x23.
   * @param orientation Kiểu lắp cảm biến.
   * @return true khi chuỗi lệnh cấu hình đã được gửi đủ.
   */
  bool setOrientation(TungLamHWT901BOrientation orientation);

  /**
   * @brief Bật hoặc tắt LED trên cảm biến nếu model/firmware hỗ trợ.
   * @param on true để bật LED, false để tắt LED.
   * @return true khi chuỗi lệnh cấu hình đã được gửi đủ.
   */
  bool setLed(bool on);

  /**
   * @brief Cấu hình nhanh chế độ thường dùng cho robot.
   *
   * Hàm đặt dữ liệu đầu ra thành ACC + GYRO + ANGLE và đặt tần số theo rate.
   * Với 100 Hz hoặc cao hơn, nên dùng baud đủ lớn, điển hình 115200 bit/s,
   * để tránh nghẽn băng thông UART khi phát đồng thời ba frame 11 byte.
   *
   * @param rate Tần số đầu ra, mặc định 100 Hz.
   * @return false nếu chưa gắn Stream hoặc có lệnh không gửi đủ; ngược lại true.
   */
  bool configureRobotMode(TungLamHWT901BRate rate = TungLamHWT901BRate::Hz100);

  /**
   * @brief Đổi baud ở phía HWT901B.
   *
   * Sau lệnh này, UART phía Arduino vẫn giữ baud cũ. Người dùng phải tự
   * khởi tạo lại cổng UART bằng baud mới trước khi tiếp tục giao tiếp.
   *
   * @param baud Mã baud WIT cần đặt.
   * @return true khi chuỗi lệnh đổi baud và lưu cấu hình đã được gửi đủ.
   */
  bool setSensorBaud(TungLamHWT901BBaud baud);

  /**
   * @brief Chuyển mã baud của WIT thành giá trị baud thực.
   * @param baud Giá trị enum TungLamHWT901BBaud.
   * @return Baud tính theo bit/s; trả 0 nếu enum không hợp lệ.
   */
  static uint32_t baudValue(TungLamHWT901BBaud baud);

  /**
   * @brief Ghi một thanh ghi WIT theo chuỗi UNLOCK -> WRITE -> SAVE.
   *
   * Đây là API nâng cao. Người dùng thông thường nên ưu tiên các hàm
   * setRate(), setOutput(), setBandwidth()... để tránh ghi sai địa chỉ.
   *
   * @param address Địa chỉ thanh ghi 8 bit.
   * @param value Giá trị 16 bit cần ghi, gửi theo thứ tự byte thấp trước.
   * @param saveAfter true để gửi lệnh SAVE sau khi ghi; false để bỏ qua SAVE.
   * @return false nếu chưa gắn Stream hoặc có gói lệnh không ghi đủ 5 byte.
   */
  bool writeRegister(uint8_t address, uint16_t value, bool saveAfter = true);

  /**
   * @brief Yêu cầu đọc thanh ghi bắt đầu từ địa chỉ chỉ định.
   *
   * Thư viện gửi READADDR 0x27. Khi cảm biến trả frame 0x5F, bốn word dữ liệu
   * gần nhất được lưu và có thể đọc bằng lastRegisterValue(0..3).
   *
   * @param address Địa chỉ thanh ghi bắt đầu cần đọc.
   * @return false nếu chưa gắn Stream; true nếu gói yêu cầu đã gửi đủ.
   */
  bool requestRegister(uint8_t address);

  /**
   * @brief Kiểm tra đã nhận phản hồi thanh ghi 0x5F sau request gần nhất hay chưa.
   * @return true khi đã có phản hồi; false khi chưa có.
   */
  bool hasRegisterResponse() const;

  /** @brief Lấy địa chỉ cơ sở của requestRegister() gần nhất. */
  uint8_t lastRegisterBase() const;

  /**
   * @brief Lấy một word trong frame phản hồi thanh ghi 0x5F gần nhất.
   * @param index Chỉ số word từ 0 đến 3.
   * @return Giá trị signed 16 bit; trả 0 nếu index lớn hơn 3.
   */
  int16_t lastRegisterValue(uint8_t index) const;

  /** @brief Lấy tổng số frame hợp lệ đã giải mã kể từ lần reset thống kê. */
  uint32_t frameCount() const;

  /** @brief Lấy tổng số lỗi checksum đã phát hiện. */
  uint32_t checksumErrorCount() const;

  /** @brief Lấy số frame hợp lệ nhưng chưa có bộ giải mã chuyên biệt. */
  uint32_t unknownFrameCount() const;

  /** @brief Lấy tổng số byte đã đọc từ UART. */
  uint32_t byteCount() const;

  /**
   * @brief Xóa các bộ đếm chẩn đoán.
   *
   * Hàm chỉ đặt lại valid_frames, checksum_errors, unknown_frames và
   * bytes_received. Dữ liệu cảm biến, các cờ has_* và offset góc vẫn giữ nguyên.
   */
  void clearStatistics();

  /**
   * @brief Chuẩn hóa góc về miền (-180, 180].
   * @param deg Góc đầu vào, đơn vị độ.
   * @return Góc tương đương trong miền (-180, 180].
   */
  static float normalize180(float deg);

  /**
   * @brief Chuẩn hóa góc về miền [0, 360).
   * @param deg Góc đầu vào, đơn vị độ.
   * @return Góc tương đương trong miền [0, 360).
   */
  static float normalize360(float deg);

 private:
  // ------------------------- Định dạng frame WIT -------------------------
  static const uint8_t FRAME_HEAD = 0x55;  ///< Byte đầu cố định của frame telemetry.
  static const uint8_t FRAME_SIZE = 11;    ///< Tổng số byte của một frame chuẩn.

  static const uint8_t FRAME_ACCEL = 0x51;       ///< Frame gia tốc + nhiệt độ.
  static const uint8_t FRAME_GYRO = 0x52;        ///< Frame vận tốc góc + điện áp.
  static const uint8_t FRAME_ANGLE = 0x53;       ///< Frame Roll/Pitch/Yaw + version.
  static const uint8_t FRAME_MAG = 0x54;         ///< Frame từ trường.
  static const uint8_t FRAME_QUATERNION = 0x59;  ///< Frame quaternion.
  static const uint8_t FRAME_REGISTER = 0x5F;    ///< Frame phản hồi đọc thanh ghi.

  // ------------------------- Địa chỉ thanh ghi WIT ----------------------
  static const uint8_t REG_SAVE = 0x00;       ///< Lưu cấu hình.
  static const uint8_t REG_CALSW = 0x01;      ///< Lệnh hiệu chuẩn/zero.
  static const uint8_t REG_RSW = 0x02;        ///< Chọn nhóm dữ liệu đầu ra.
  static const uint8_t REG_RRATE = 0x03;      ///< Tần số xuất dữ liệu.
  static const uint8_t REG_BAUD = 0x04;       ///< Baud của cảm biến.
  static const uint8_t REG_LEDOFF = 0x1B;     ///< Điều khiển LED.
  static const uint8_t REG_BANDWIDTH = 0x1F;  ///< Băng thông bộ lọc.
  static const uint8_t REG_ORIENT = 0x23;     ///< Tư thế lắp.
  static const uint8_t REG_AXIS6 = 0x24;      ///< Chọn thuật toán 6/9 trục.
  static const uint8_t REG_READADDR = 0x27;   ///< Địa chỉ bắt đầu khi đọc thanh ghi.
  static const uint8_t REG_KEY = 0x69;        ///< Thanh ghi khóa/mở khóa cấu hình.

  static const uint16_t KEY_UNLOCK = 0xB588;  ///< Giá trị mở khóa cấu hình WIT.
  static const float GRAVITY_MPS2;            ///< Gia tốc trọng trường tiêu chuẩn.
  static const float DEG_TO_RAD_F;            ///< Hệ số chuyển độ sang radian.

  Stream *_stream;  ///< Stream UART hiện đang được thư viện sử dụng.
  uint32_t _baud;   ///< Baud đã truyền vào begin(); bằng 0 nếu dùng attach().

  uint8_t _frame[FRAME_SIZE];  ///< Buffer ghép frame 11 byte.
  uint8_t _frameIndex;         ///< Vị trí byte tiếp theo trong buffer frame.

  TungLamHWT901BData _data;  ///< Trạng thái dữ liệu công khai mới nhất.

  uint8_t _lastRegisterBase;       ///< Địa chỉ cơ sở của request đọc gần nhất.
  int16_t _lastRegisterValues[4];  ///< Bốn word từ frame 0x5F gần nhất.
  bool _registerResponseReady;     ///< Cờ đã có phản hồi 0x5F.

  // Xóa trạng thái runtime khi tạo đối tượng hoặc đổi Stream.
  void resetRuntime();

  // Nạp một byte vào bộ phân tích; trả true khi vừa hoàn thành frame hợp lệ.
  bool feedByte(uint8_t value);

  // Tìm lại byte 0x55 bên trong buffer sau một frame sai checksum.
  void resyncAfterBadFrame();

  // Giải mã frame hợp lệ vào _data dựa trên byte loại frame.
  void decodeFrame(const uint8_t *frame);

  // Đọc số 16 bit little-endian có dấu/không dấu từ payload WIT.
  static int16_t readI16(const uint8_t *p);
  static uint16_t readU16(const uint8_t *p);

  // Áp dụng ba offset phần mềm lên góc raw và chuẩn hóa miền góc.
  void applyAngleOffsets();

  // Gửi gói lệnh WIT 5 byte: FF AA ADDR DATA_L DATA_H.
  bool sendCommand(uint8_t address, uint16_t value);

  // Các primitive nội bộ cho chuỗi cấu hình UNLOCK -> WRITE -> SAVE.
  bool unlock();
  bool save();
  bool writeRegisterUnlocked(uint8_t address, uint16_t value);
};

/**
 * @brief Tên rút gọn để sketch Arduino dễ đọc hơn.
 *
 * Hai cách khai báo dưới đây hoàn toàn tương đương:
 *
 * @code
 * TungLamHWT901B imu;
 * HWT901B imu;
 * @endcode
 */
typedef TungLamHWT901B HWT901B;

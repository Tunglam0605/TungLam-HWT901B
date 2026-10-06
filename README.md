# TungLam_HWT901B

**TungLam_HWT901B** là thư viện Arduino giao tiếp UART **không chặn** dành cho **WIT Motion HWT901B TTL** và các thiết bị tương thích **WIT Standard Protocol**.

Tác giả: **Nguyễn Khắc Tùng Lâm — Tùng Lâm Automation**

## Triết lý sử dụng

API và identifier giữ tên tiếng Anh chuẩn kỹ thuật để tương thích hệ sinh thái Arduino; **toàn bộ comment, Doxygen, ví dụ và tài liệu hướng dẫn được viết bằng tiếng Việt**.

Mục tiêu của thư viện:

- API đơn giản cho người mới: `begin() -> update() -> getYaw()`.
- Không `delay()`, không `readBytes()`, không chặn `loop()`.
- Tự kiểm tra checksum và tự đồng bộ lại frame khi UART nhiễu/lệch byte.
- Tự quản lý offset góc bên trong thư viện.
- Cho phép đặt góc hiện tại thành **bất kỳ giá trị nào**.
- Có API cấu hình trực tiếp các thanh ghi WIT khi cần.
- Dùng được với `HardwareSerial`, `SoftwareSerial` và các lớp tương thích Arduino `Stream`.

---

## Bắt đầu nhanh — Arduino Mega 2560

### Nối dây

Ví dụ dùng `Serial1`:

| HWT901B TTL | Arduino Mega 2560 |
|---|---|
| TX | RX1 / D19 |
| RX | TX1 / D18 |
| GND | GND |
| VCC | Cấp đúng điện áp ghi trên module/tài liệu của phiên bản HWT901B đang dùng |

> TX của IMU nối vào RX của Arduino, RX của IMU nối vào TX của Arduino. Hai thiết bị phải chung GND.

### Chương trình tối thiểu

```cpp
#include <TungLam_HWT901B.h>

HWT901B imu;

void setup() {
  Serial.begin(115200);
  // 9600 là baud mặc định của WIT Standard Protocol.
  imu.begin(Serial1);
}

void loop() {
  imu.update();

  if (imu.hasAngle()) {
    Serial.println(imu.getYaw());
  }
}
```

Điểm quan trọng: gọi `imu.update()` thường xuyên trong `loop()`. Hàm này không chặn và chỉ xử lý các byte UART đang có.

---

# Trường hợp quan trọng: IMU đang là 89°, nhưng tôi muốn vị trí đó là 180°

Không cần tự tạo biến offset ở sketch.

```cpp
if (imu.hasAngle()) {
  imu.setCurrentYaw(180.0f);
}
```

Ví dụ trước khi gọi:

```text
raw yaw = 89°
```

sau:

```cpp
imu.setCurrentYaw(180.0f);
```

thì ngay tại chính tư thế đó:

```cpp
imu.getYaw();       // ~180°
imu.getRawYaw();    // vẫn ~89°
```

Thư viện tự tính và giữ:

```text
offset = rawYaw - targetYaw
yaw    = normalize(rawYaw - offset)
```

Với ví dụ trên:

```text
offset = 89 - 180 = -91°
yaw    = 89 - (-91) = 180°
```

Thư viện tự xử lý việc quấn góc qua biên `-180 / +180`.

Offset phần mềm này chỉ nằm trong RAM của Arduino. Khi reset Arduino, offset trở về mặc định.

---

# Đặt yaw hiện tại thành 0°

```cpp
imu.zeroYaw();
```

Tương đương:

```cpp
imu.setCurrentYaw(0.0f);
```

Để bỏ offset và quay về góc raw của sensor:

```cpp
imu.clearYawOffset();
```

---

# Zero phần mềm và zero trên cảm biến khác nhau thế nào?

## Khuyến nghị cho robot: zero phần mềm

```cpp
imu.zeroYaw();
imu.setCurrentYaw(180.0f);
```

Ưu điểm:

- tức thời;
- không ghi cấu hình IMU;
- không cần chờ cảm biến lưu;
- có thể gọi nhiều lần khi robot đổi mốc;
- phù hợp cho heading, odometry và điều khiển robot.

## Zero trực tiếp trên HWT901B

```cpp
imu.sensorZeroHeading();
```

Hàm này gửi lệnh WIT tới cảm biến để thực hiện zero heading bằng thanh ghi `CALSW`.

Không nên dùng nó thay cho zero phần mềm nếu mục tiêu chỉ là đổi hệ quy chiếu của robot trong lúc chạy.

---

# Đọc góc

```cpp
float roll  = imu.getRoll();
float pitch = imu.getPitch();
float yaw   = imu.getYaw();
```

Miền của `getYaw()`:

```text
(-180°, +180°]
```

Nếu cần `0...360°`:

```cpp
float yaw360 = imu.getYaw360();
```

Góc gốc chưa bù offset:

```cpp
imu.getRawRoll();
imu.getRawPitch();
imu.getRawYaw();
```

---

# Đặt cả ba góc hiện tại

```cpp
imu.setCurrentAngles(
    0.0f,    // roll hiện tại được coi là 0°
    0.0f,    // pitch hiện tại được coi là 0°
    180.0f   // yaw hiện tại được coi là 180°
);
```

Hoặc riêng từng trục:

```cpp
imu.setCurrentRoll(0.0f);
imu.setCurrentPitch(0.0f);
imu.setCurrentYaw(180.0f);
```

---

# Đọc gia tốc

Đơn vị mặc định của getter là **m/s²**:

```cpp
float ax = imu.getAccelX();
float ay = imu.getAccelY();
float az = imu.getAccelZ();
```

Nếu muốn đơn vị **g**:

```cpp
imu.getAccelXg();
imu.getAccelYg();
imu.getAccelZg();
```

---

# Đọc gyro

Đơn vị mặc định là **deg/s**:

```cpp
imu.getGyroX();
imu.getGyroY();
imu.getGyroZ();
```

Nếu thuật toán điều khiển cần **rad/s**:

```cpp
imu.getGyroXRad();
imu.getGyroYRad();
imu.getGyroZRad();
```

---

# Từ kế

```cpp
imu.getMagX();
imu.getMagY();
imu.getMagZ();
```

Giá trị trả về là số nguyên có dấu 16 bit thô theo frame WIT.

---

# Quaternion

Nếu HWT901B đang được cấu hình phát frame quaternion:

```cpp
imu.getQ0();
imu.getQ1();
imu.getQ2();
imu.getQ3();
```

---

# Ảnh chụp dữ liệu đầy đủ

```cpp
TungLamHWT901BData data;

if (imu.getData(data)) {
  Serial.println(data.yaw_deg);
  Serial.println(data.gz_rad_s);
  Serial.println(data.checksum_errors);
}
```

Dùng cấu trúc snapshot khi cần lấy nhiều trường dữ liệu trong cùng một lần.

---

# Kiểm tra IMU còn online không

```cpp
if (!imu.connected(500)) {
  robot.stop();
}
```

`500` là timeout theo ms.

Tuổi frame cuối:

```cpp
uint32_t age = imu.ageMs();
```

Các bộ đếm chẩn đoán:

```cpp
imu.frameCount();
imu.checksumErrorCount();
imu.unknownFrameCount();
imu.byteCount();
```

---

# Dùng HardwareSerial

Mega2560 nên ưu tiên UART phần cứng. Với cảm biến đang ở baud mặc định WIT:

```cpp
imu.begin(Serial1);       // 9600 baud
```

Nếu cảm biến đã được cấu hình 115200:

```cpp
imu.begin(Serial1, 115200);
```

Tương tự có thể dùng `Serial2` hoặc `Serial3`.

Với robot chạy encoder/PID/PS2/stepper đồng thời, **HardwareSerial là lựa chọn khuyến nghị**.

---

# Dùng SoftwareSerial

Ví dụ UNO/Nano AVR:

```cpp
#include <SoftwareSerial.h>
#include <TungLam_HWT901B.h>

SoftwareSerial imuSerial(10, 11);
HWT901B imu;

void setup() {
  Serial.begin(115200);

  // HWT901B phải đang được cấu hình đúng 9600 baud.
  imu.begin(imuSerial, 9600);
}
```

> `begin(..., baud)` chỉ cấu hình UART phía Arduino. Hàm không tự dò và cũng không tự đổi baud hiện tại của IMU.

SoftwareSerial ở baud cao có thể làm tăng jitter hoặc mất byte khi MCU còn xử lý nhiều ngắt. Với robot thật nên ưu tiên HardwareSerial.

---

# Dùng Stream đã được khởi tạo sẵn

Nếu UART đã được một phần khác của chương trình cấu hình:

```cpp
Serial1.begin(115200);
imu.attach(Serial1);
```

---

# Cấu hình nhanh cho robot

```cpp
imu.configureRobotMode(TungLamHWT901BRate::Hz100);
```

Cấu hình:

```text
ACC + GYRO + ANGLE
100 Hz
```

> Với 3 frame 11 byte ở 100 Hz, hãy dùng baud đủ cao, khuyến nghị 115200. Ví dụ này giả định HWT901B và UART phía Arduino đã cùng ở 115200; không nên đặt 100 Hz khi vẫn dùng 9600.

Chỉ gọi các lệnh cấu hình trong `setup()` hoặc khi thật sự cần đổi cấu hình.

Không gọi liên tục trong `loop()`.

---

# Tần số xuất dữ liệu

```cpp
imu.setRate(TungLamHWT901BRate::Hz10);
imu.setRate(TungLamHWT901BRate::Hz20);
imu.setRate(TungLamHWT901BRate::Hz50);
imu.setRate(TungLamHWT901BRate::Hz100);
imu.setRate(TungLamHWT901BRate::Hz200);
```

---

# Chọn dữ liệu cảm biến xuất ra

Ví dụ chỉ ACC + GYRO + ANGLE:

```cpp
imu.setOutput(
    HWT901B_OUT_ACCEL |
    HWT901B_OUT_GYRO |
    HWT901B_OUT_ANGLE
);
```

Nếu cần quaternion:

```cpp
imu.setOutput(
    HWT901B_OUT_ACCEL |
    HWT901B_OUT_GYRO |
    HWT901B_OUT_ANGLE |
    HWT901B_OUT_QUATERNION
);
```

---

# Băng thông, thuật toán và tư thế lắp

```cpp
imu.setBandwidth(TungLamHWT901BBandwidth::Hz20);

imu.setAlgorithm(
    TungLamHWT901BAlgorithm::Axis9
);

imu.setOrientation(
    TungLamHWT901BOrientation::Horizontal
);
```

---

# Đổi baud của cảm biến

Ví dụ đổi cảm biến sang 9600:

```cpp
imu.setSensorBaud(
    TungLamHWT901BBaud::Baud9600
);
```

Sau khi cảm biến đổi baud, UART phía Arduino cũng phải được khởi tạo lại đúng baud mới.

Khuyến nghị: chỉ đổi baud trong một sketch cấu hình riêng, sau đó chạy chương trình chính với baud đã biết.

---

# API thanh ghi nâng cao

Ghi thanh ghi:

```cpp
imu.writeRegister(address, value);
```

Thư viện tự đóng gói chuỗi:

```text
UNLOCK
  ↓
WRITE
  ↓
SAVE
```

Yêu cầu đọc thanh ghi:

```cpp
imu.requestRegister(address);
```

Trong `loop()`:

```cpp
imu.update();

if (imu.hasRegisterResponse()) {
  int16_t word0 = imu.lastRegisterValue(0);
}
```

Đây là API nâng cao; phần lớn dự án robot không cần dùng trực tiếp.

---

# Bộ phân tích UART không chặn

Luồng dữ liệu:

```text
UART Stream
    ↓
update()
    ↓
bộ phân tích từng byte
    ↓
tìm 0x55
    ↓
gom 11 byte
    ↓
checksum
    ├── sai → resync
    └── đúng
          ↓
       giải mã
          ↓
     trạng thái nội bộ
          ↓
      getter API
```

Không có:

- `delay()`;
- vòng chờ đủ 11 byte;
- `readBytes()`;
- cấp phát heap;
- biến offset bắt người dùng tự quản lý.

---

# Các frame đã hỗ trợ giải mã

| Frame | Dữ liệu |
|---|---|
| `0x51` | Gia tốc + nhiệt độ |
| `0x52` | Vận tốc góc |
| `0x53` | Roll/Pitch/Yaw + version |
| `0x54` | Từ trường |
| `0x59` | Quaternion |
| `0x5F` | Phản hồi thanh ghi |

Các frame WIT khác vẫn được bộ phân tích xác nhận ở mức frame hợp lệ và được tính vào bộ đếm chẩn đoán nếu chưa có bộ giải mã chuyên biệt.

---

# Các ví dụ

Arduino IDE:

1. **01_BasicAngle** — khởi tạo và đọc Roll/Pitch/Yaw.
2. **02_SetCurrentYaw** — ví dụ trực tiếp góc hiện tại → 180°.
3. **03_ZeroYaw** — zero Yaw bằng offset phần mềm.
4. **04_AccelGyro** — đọc gia tốc và vận tốc góc.
5. **05_SoftwareSerial** — UNO/Nano với UART mềm.
6. **06_SensorConfig** — cấu hình dữ liệu đầu ra và tần số.
7. **07_AdvancedRegister** — đọc/ghi thanh ghi nâng cao.

---

# Lưu ý thời gian thực

Thư viện không chiếm timer và không có ISR riêng.

`update()` nên được gọi thường xuyên:

```cpp
void loop() {
  imu.update();

  ps2.update();
  robot.update();
  // PID / máy trạng thái / cảm biến...
}
```

Nếu hệ dùng RTOS, gọi `update()` từ một task chịu quyền sở hữu UART hoặc bảo vệ Stream khỏi truy cập đồng thời.

---

# Cài đặt

## Arduino Library Manager

Tìm:

```text
TungLam_HWT901B
```

sau khi phiên bản đã được Arduino Library Registry index.

## ZIP

Arduino IDE:

```text
Sketch
→ Include Library
→ Add .ZIP Library...
```

---

# Nguồn giao thức

Thư viện được xây dựng theo tài liệu **WIT Standard Communication Protocol** và thông số HWT901B chính hãng:

- WIT Standard Protocol: https://wit-motion.gitbook.io/witmotion-sdk/wit-standard-protocol/wit-standard-communication-protocol
- WIT SDK: https://www.wit-motion.com/SDK.html
- HWT901B product: https://www.wit-motion.com/9-axis/witmotion-hwt901b-ttl-mpu9250-9-axis-acceleration.html

---

# Trạng thái phát hành

**v0.1.1**

Bản vá chuẩn hóa toàn bộ tài liệu, Doxygen và comment tiếng Việt; logic điều khiển và giao thức giữ nguyên so với v0.1.0.

Trước khi gọi một phiên bản là **ổn định trên phần cứng**, nên chạy kiểm thử hồi quy bằng HWT901B thật ở baud/tần số dự kiến của robot và kiểm tra:

- lỗi checksum khi chạy bền;
- rút/cắm lại cảm biến;
- chuyển góc qua biên `+180/-180`;
- `setCurrentYaw()` ở nhiều tư thế;
- 100 Hz / 200 Hz;
- SoftwareSerial nếu dự án bắt buộc dùng UART mềm.

---

# Giấy phép

MIT License.

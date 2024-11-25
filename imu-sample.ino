#include "mpu_x.h"

// -=| MPU-6500 |=-
const int IMU_ADDR_ON_BUS = 0x68;

MPUx imu(IMU_ADDR_ON_BUS);

void setup() {
  // Initialize UART
  Serial.begin(9600);

  // ESP32 boards have a delay with UART for some reason from a reset after
  // uploading new code to the development board
#if 1  // board was newly programmed
  delay(1000);
#else
  // https://www.arduino.cc/reference/en/language/functions/communication/serial/ifserial/
  while (!Serial) {
    ;  // wait for the serial port to connect. Needed for native USB
  }
#endif

  // Initialize I2C
  Wire.begin();
  delay(200);

  // Initialize the IMU
  imu.init();
  delay(200);

  Serial.println("INFO: IMU initialized");
}

void loop() {
  imu.printMPUData();
  delay(1000);
}

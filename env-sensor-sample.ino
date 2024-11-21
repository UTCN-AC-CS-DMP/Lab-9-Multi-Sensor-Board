// EXAMPLE CODE FOR THE BME-280

#include "bmx_280.h"
// No need to include <Wire.h> or "i2c_utils.h" in this source file, because
// they're already included in the "bmx_280.h" file

// Addr of BME on the I²C bus
const int BME_ADDR_ON_BUS = 0x77;

// Instantiate the env sensor
BMx280 envSensor(BME_ADDR_ON_BUS);

void setup() {
  // Initialize the UART interface
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

  // Initialize the I2C interface
  Wire.begin();
  delay(250);  // Minor delay after starting I2C interface

  // Initialize the env sensor
  envSensor.init();
  delay(200);

  Serial.println("INFO: Env sensor initialized");
}

void loop() {
  // Read the data from the environmental sensor using the helper function
  envSensor.printBMEData();
  delay(2500);
}

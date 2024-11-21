#include "i2c_utils.h"
// i2c_utils.h already includes <Wire.h>, so no need to include it here again

// The addresses on the bus determined from the previouis example
const uint8_t MPU_ADDR_ON_BUS = 0x68;
const uint8_t BMX_ADDR_ON_BUS = 0x77;

// Decimal, not hexa, because that's how it's in the device doc
const uint8_t MPU_WHOAMI_REG_ADDR = 117;

// Env sensor's id address is in hexa
const uint8_t BMX_WHOAMI_REG_ADDR = 0xD0;

void setup() {
  // Initialize the UART interface
  Serial.begin(115200);

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

  // Using the helper function from the utils lib
  identifyDevice(MPU_ADDR_ON_BUS, MPU_WHOAMI_REG_ADDR);
  delay(100);  // Minor delay
  identifyDevice(BMX_ADDR_ON_BUS, BMX_WHOAMI_REG_ADDR);
}

void loop() {}

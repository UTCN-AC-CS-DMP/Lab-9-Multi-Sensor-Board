#include <Wire.h>

void setup() {
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

  // Initialize I²C
  Wire.begin();

  delay(250);

  // Search for all devices on the bus
  printAllI2CDevicesOnBus();
}

void loop() {}

void printAllI2CDevicesOnBus() {
  uint8_t deviceCounter;

  Serial.println("Scanning for devices");
  Serial.println("--------------------");

  // Scan all addresses from 1 to 126
  for (uint8_t i = 1; i < 127; i++) {
    Wire.beginTransmission(i);

    // Check for error at address i
    const uint8_t error = Wire.endTransmission();

    if (error == 0) {
      // If there is no error, print the address
      Serial.printf("I²C device found at address 0x%02x\n", i);
      deviceCounter++;
    } else if (error == 4) {
      // This is in case of a faulty device
      Serial.printf("Unknown error at address 0x%02x\n", i);
    }
  }

  if (deviceCounter == 0)
    Serial.println("No I²C devices found");
  else
    Serial.printf("Found %d device(s) in total\n", deviceCounter);
}

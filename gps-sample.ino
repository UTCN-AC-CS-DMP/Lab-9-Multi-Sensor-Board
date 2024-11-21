/*
!!!ATTENTION!!!
For some reason the ESP32 Arduino library
uses GPIO 4 for RX2 and GPIO 25 for TX2
This is why we're re-defining the "correct"
pins below for 16 & 17
*/

// -=| UART |=-
HardwareSerial HWSerial(2);
const int HW_SERIAL_TX = 17;  // TX2 on DOIT V1
const int HW_SERIAL_RX = 16;  // RX2 on DOIT V1

void setup() {
  // Initialize UART
  HWSerial.begin(9600, SERIAL_8N1, HW_SERIAL_RX, HW_SERIAL_TX);
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

  Serial.println("INFO: Serial port initialized");
}

void loop() {
  while (HWSerial.available()) {
    Serial.print(char(HWSerial.read()));
  }
}

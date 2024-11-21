#include "bmx_280.h"

void BMx280::init() {
  // Perform soft reset on the device
  wakeUpDevice(deviceAddress, BME_REG_RESET, BME_VAL_RESET_SOFT);
  delay(200);

  setCompensationValues();

  // Before setting the configuration registers, the device needs to be in
  // sleep mode
  i2cWriteToRegister(deviceAddress, BME_REG_CONTROL, BME_VAL_CONF_MODE_SLEEP);

  // 101 | 101 | 11
  //  t     p     m   <-- 16x oversampling for temp & pressure, normal mode

  const uint8_t ctrlRegVal = 0xB7;
  const uint8_t humCtrlRegVal = 0x05;
  const uint8_t confRegVal = 0x00;

  i2cWriteToRegister(deviceAddress, BME_REG_CONTROLHUMID, humCtrlRegVal);
  i2cWriteToRegister(deviceAddress, BME_REG_CONFIG, confRegVal);
  i2cWriteToRegister(deviceAddress, BME_REG_CONTROL, ctrlRegVal);

  delay(200);
}

void BMx280::setCompensationValues() {
  populateTemperatureCompensationValues();
  populatePressureCompensationValues();
  populateHumidityCompensationValues();
}

void BMx280::populateTemperatureCompensationValues() {
  compVals.T1 = i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T1);
  compVals.T2 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T2);
  compVals.T3 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T3);
}

void BMx280::populatePressureCompensationValues() {
  compVals.P1 = i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P1);
  compVals.P2 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P2);
  compVals.P3 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P3);
  compVals.P4 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P4);
  compVals.P5 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P5);
  compVals.P6 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P6);
  compVals.P7 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P7);
  compVals.P8 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P8);
  compVals.P9 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P9);
}

void BMx280::populateHumidityCompensationValues() {
  compVals.H1 = i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H1);
  compVals.H2 =
      (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_H2);
  compVals.H3 = i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H3);
  compVals.H4 =
      (int16_t)(((int8_t)i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H4)
                 << 4) |
                (i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H4 + 1) &
                 0xF));
  compVals.H5 =
      (int16_t)(((int8_t)i2cReadByteFromRegister(deviceAddress,
                                                 BME_REG_COMP_H5 + 1)
                 << 4) |
                (i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H5) >> 4));
  compVals.H6 = (int8_t)i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H6);
}

int32_t BMx280::getTFine() {
  int32_t adcT = i2cReadThreeBytesFromRegister(deviceAddress, BME_REG_TEMPDATA);

  // Return zero in case temperature measurement was disabled
  if (adcT == 0x800000) {
    return 0;
  }
  adcT >>= 4;

  int32_t var1, var2;

  var1 = (int32_t)((adcT / 8) - ((int32_t)compVals.T1 * 2));
  var1 = (var1 * ((int32_t)compVals.T2)) / 2048;
  var2 = (int32_t)((adcT / 16) - ((int32_t)compVals.T1));
  var2 = (((var2 * var2) / 4096) * ((int32_t)compVals.T3)) / 16384;

  return var1 + var2;
}

float BMx280::getTemperature() {
  int32_t tempFine = getTFine();

  int32_t T = (tempFine * 5 + 128) / 256;

  return (float)T / 100;
}

float BMx280::getPressure() {
  // Must be done first to get the tFine variable
  // which cannot be done without reading the temperature
  int32_t tFine = getTFine();

  int32_t adcP =
      i2cReadThreeBytesFromRegister(deviceAddress, BME_REG_PRESSUREDATA);

  // Return zero in case pressure measurement was disabled
  if (adcP == 0x800000) {
    return 0;
  }
  adcP >>= 4;

  int64_t var1, var2, P;

  var1 = ((int64_t)tFine) - 128000;
  var2 = var1 * var1 * (int64_t)compVals.P6;
  var2 = var2 + ((var1 * (int64_t)compVals.P5) << 17);
  var2 = var2 + (((int64_t)compVals.P4) << 35);
  var1 = ((var1 * var1 * (int64_t)compVals.P3) >> 8) +
         ((var1 * (int64_t)compVals.P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)compVals.P1) >> 33;

  // Avoid exception caused by division by zero
  if (var1 == 0) {
    return 0;
  }

  P = 1048576 - adcP;
  P = (((P << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)compVals.P9) * (P >> 13) * (P >> 13)) >> 25;
  var2 = (((int64_t)compVals.P8) * P) >> 19;

  P = ((P + var1 + var2) >> 8) + (((int64_t)compVals.P7) << 4);

  return (float)P / 256;
}

float BMx280::getHumidity() {
  // Must be done first to get the tFine variable
  // which cannot be done without reading the temperature
  int32_t tFine = getTFine();

  int32_t adc_H = i2cReadWordFromRegister(deviceAddress, BME_REG_HUMIDDATA);

  // Return zero in case the humidity measurement was disabled
  if (adc_H == 0x8000) {
    return 0;
  }

  int32_t var1, var2, var3, var4, var5;

  var1 = tFine - ((int32_t)76800);
  var2 = (int32_t)(adc_H * 16384);
  var3 = (int32_t)(((int32_t)compVals.H4) * 1048576);
  var4 = ((int32_t)compVals.H5) * var1;
  var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
  var2 = (var1 * ((int32_t)compVals.H6)) / 1024;
  var3 = (var1 * ((int32_t)compVals.H3)) / 2048;
  var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
  var2 = ((var4 * ((int32_t)compVals.H2)) + 8192) / 16384;
  var3 = var5 * var2;
  var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
  var5 = var3 - ((var4 * ((int32_t)compVals.H1)) / 16);
  var5 = (var5 < 0 ? 0 : var5);
  var5 = (var5 > 419430400 ? 419430400 : var5);
  uint32_t H = (uint32_t)(var5 / 4096);

  return (float)H / 1024.0;
}

void BMx280::printBMEData() {
  Serial.println("|--------= BME280 =--------|");
  Serial.printf("| Temperature: %.2f ˚C    |\n", getTemperature());
  Serial.printf("|    Pressure: %.2f hPa  |\n", getPressure() / 100.0F);
  Serial.printf("|    Humidity: %.2f %%     |\n", getHumidity());
}

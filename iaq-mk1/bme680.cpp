#include "bme680.h"

#define CALIBRATED_SEA_LEVEL 120.0
DFRobot_BME680_I2C BME680(0x76);
float SEA_LEVEL;

/*
 * Setup BME680 sensor.
 * IAQ not supported on Arduino Nano RP2040 Connect.
 */
void setup_BME680()
{
  uint8_t rslt = 1;
  while (rslt != 0) {
    rslt = BME680.begin();
    if (rslt != 0) {
      Serial.println("[!] BME680 init failed, please check connection");
      delay(2000);
    }
  }

  Serial.println("[+] BME680 begin successful");

  BME680.startConvert();
  delay(1000);
  BME680.update();
  SEA_LEVEL = BME680.readSeaLevel(CALIBRATED_SEA_LEVEL);
  Serial.print("[+] Sea level :");
  Serial.println(SEA_LEVEL);
}
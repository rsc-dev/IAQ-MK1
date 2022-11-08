#include "ens160.h"
#define I2C_COMMUNICATION

DFRobot_ENS160_I2C ENS160(&Wire, 0x53);

/*
 * Setup ENS160 sensor.
 */
void setup_ENS160(float temperature, float humidity)
{
  while( NO_ERR != ENS160.begin() ){
    Serial.println("[!] ENS160 init failed, please check connection");
    delay(3000);
  }

  /**
   * Set power mode
   * mode Configurable power mode:
   *   ENS160_SLEEP_MODE: DEEP SLEEP mode (low power standby)
   *   ENS160_IDLE_MODE: IDLE mode (low-power)
   *   ENS160_STANDARD_MODE: STANDARD Gas Sensing Modes
   */
  ENS160.setPWRMode(ENS160_STANDARD_MODE);

  /**
   * Users write ambient temperature and relative humidity into ENS160 for calibration and compensation of the measured gas data.
   * ambientTemp Compensate the current ambient temperature, float type, unit: C
   * relativeHumidity Compensate the current ambient temperature, float type, unit: %rH
   */
  
  Serial.print("[ENS160] Setting temperature=");
  Serial.println(temperature, 2);

  Serial.print("[ENS160] Setting humidity=");
  Serial.println(humidity, 2);

  ENS160.setTempAndHum(temperature, humidity);
}
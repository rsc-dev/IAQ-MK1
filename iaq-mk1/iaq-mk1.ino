/*
 * IAQ MK1 firmware.
 * https://github.com/rsc-dev/IAQ-MK1
 */

#include "Wire.h"
#include "bme680.h"
#include "ens160.h"

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  Serial.println("[+] Initiating setup...");

  Serial.println("[+] BME680...");
  setup_BME680();

  Serial.println("[+] ENS160...");
  BME680.startConvert();
  delay(1000);
  BME680.update();
  float temperature = BME680.readTemperature() / 100;
  float humidity = BME680.readHumidity() / 1000;

  setup_ENS160(temperature, humidity);

  Serial.println("[+] Setup finished.");
}

void loop()
{
  BME680.startConvert();
  delay(1000);
  BME680.update();
  Serial.println();
  Serial.print("[BME680] Temperature(C): ");
  Serial.println(BME680.readTemperature() / 100, 2);
  Serial.print("[BME680] Pressure(Pa): ");
  Serial.println(BME680.readPressure());
  Serial.print("[BME680] Humidity(%rh): ");
  Serial.println(BME680.readHumidity() / 1000, 2);
  Serial.print("[BME680] Gas resistance(ohm): ");
  Serial.println(BME680.readGasResistance());
  Serial.print("[BME680] Altitude(m): ");
  Serial.println(BME680.readAltitude());
  Serial.print("[BME680] Calibrated altitude(m): ");
  Serial.println(BME680.readCalibratedAltitude(SEA_LEVEL));

  uint8_t Status = ENS160.getENS160Status();
  Serial.print("[ENS160] Sensor operating status: ");
  Serial.println(Status);

  /**
   * Get the air quality index
   * Return value: 1-Excellent, 2-Good, 3-Moderate, 4-Poor, 5-Unhealthy
   */
  uint8_t AQI = ENS160.getAQI();
  Serial.print("[ENS160] Air quality index: ");
  Serial.println(AQI);

  /**
   * Get TVOC concentration
   * Return value range: 0–65000, unit: ppb
   */
  uint16_t TVOC = ENS160.getTVOC();
  Serial.print("[ENS160] Concentration of total volatile organic compounds: ");
  Serial.print(TVOC);
  Serial.println(" ppb");

  /**
   * Get CO2 equivalent concentration calculated according to the detected data of VOCs and hydrogen (eCO2 – Equivalent CO2)
   * Return value range: 400–65000, unit: ppm
   * Five levels: Excellent(400 - 600), Good(600 - 800), Moderate(800 - 1000), 
   *               Poor(1000 - 1500), Unhealthy(> 1500)
   */
  uint16_t ECO2 = ENS160.getECO2();
  Serial.print("[ENS160] Carbon dioxide equivalent concentration: ");
  Serial.print(ECO2);
  Serial.println(" ppm");


  delay(10*1000); // Sleep for 10 seconds
}
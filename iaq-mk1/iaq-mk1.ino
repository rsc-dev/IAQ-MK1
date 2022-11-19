/*
 * IAQ MK1 firmware.
 * https://github.com/rsc-dev/IAQ-MK1
 */
#include "Wire.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include "bme680.h"
#include "ens160.h"
#include "secrets.h"

#define ENS0129 2

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

WiFiServer server(80);

boolean alreadyConnected = false;

// Last readings
float TEMP = 0.0;
float PRESSURE = 0.0;
float HUMIDITY = 0.0;
float GAS_RESISTANCE = 0.0;
float ALTITUDE = 0.0;
float CAL_ALTITUDE = 0.0;
uint8_t AQI = 0;
uint16_t TVOC = 0;
uint16_t ECO2 = 0;
float LAST_PM1_0, LAST_PM2_5, LAST_PM10_0 = 0.0;
int CO2_LEVEL;

void ENS0219_readings()
{
  while (digitalRead(ENS0129) == LOW) {};
  long t0 = millis();
  
  while (digitalRead(ENS0129) == HIGH) {};
  long t1 = millis();
  
  while (digitalRead(ENS0129) == LOW) {};
  long t2 = millis();

  long tH = t1-t0;
  long tL = t2-t1;
  long ppm = 5000L * (tH - 2) / (tH + tL - 4);
  while (digitalRead(ENS0129) == HIGH) {};
  
  CO2_LEVEL = (int)ppm;
  delay(10);
}

void get_data()
{
  BME680.startConvert();
  delay(1000);
  BME680.update();

  TEMP = BME680.readTemperature() / 100;
  PRESSURE = BME680.readPressure();
  HUMIDITY = BME680.readHumidity() / 1000;
  GAS_RESISTANCE = BME680.readGasResistance();
  ALTITUDE = BME680.readAltitude();
  CAL_ALTITUDE = BME680.readCalibratedAltitude(SEA_LEVEL);

  Serial.println();
  Serial.print("[BME680] Temperature(C): ");
  Serial.println(TEMP, 2);
  Serial.print("[BME680] Pressure(Pa): ");
  Serial.println(PRESSURE);
  Serial.print("[BME680] Humidity(%rh): ");
  Serial.println(HUMIDITY, 2);
  Serial.print("[BME680] Gas resistance(ohm): ");
  Serial.println(GAS_RESISTANCE);
  Serial.print("[BME680] Altitude(m): ");
  Serial.println(ALTITUDE);
  Serial.print("[BME680] Calibrated altitude(m): ");
  Serial.println(CAL_ALTITUDE);

  uint8_t Status = ENS160.getENS160Status();
  Serial.print("[ENS160] Sensor operating status: ");
  Serial.println(Status);

  /**
   * Get the air quality index
   * Return value: 1-Excellent, 2-Good, 3-Moderate, 4-Poor, 5-Unhealthy
   */
  AQI = ENS160.getAQI();
  Serial.print("[ENS160] Air quality index: ");
  Serial.println(AQI);

  /**
   * Get TVOC concentration
   * Return value range: 0–65000, unit: ppb
   */
  TVOC = ENS160.getTVOC();
  Serial.print("[ENS160] Concentration of total volatile organic compounds: ");
  Serial.print(TVOC);
  Serial.println(" ppb");

  /**
   * Get CO2 equivalent concentration calculated according to the detected data of VOCs and hydrogen (eCO2 – Equivalent CO2)
   * Return value range: 400–65000, unit: ppm
   * Five levels: Excellent(400 - 600), Good(600 - 800), Moderate(800 - 1000), 
   *               Poor(1000 - 1500), Unhealthy(> 1500)
   */
  ECO2 = ENS160.getECO2();
  Serial.print("[ENS160] Carbon dioxide equivalent concentration: ");
  Serial.print(ECO2);
  Serial.println(" ppm");

  DumpPMS7003Data();

  Serial.print("[PMS7003] PM1.0: ");
  Serial.println(LAST_PM1_0);
  Serial.print("[PMS7003] PM2.5: ");
  Serial.println(LAST_PM2_5);
  Serial.print("[PMS7003] PM10.0: ");
  Serial.println(LAST_PM10_0);

  ENS0219_readings();
  Serial.print("[SEN0219] CO2: ");
  Serial.println(CO2_LEVEL);
}

void printWifiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup()
{
  Serial.begin(9600);
  //while (!Serial);
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

  WiFi.setHostname("sensor.local");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWifiStatus();
  server.begin();

  Serial.println("[+] Setup finished.");
}

void loop()
{
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    get_data();
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          
          client.print("temp=");
          client.println(TEMP, 2);

          client.print("pressure=");
          client.println(PRESSURE);

          client.print("humidity=");
          client.println(HUMIDITY, 2);
          
          client.print("gas resistance=");
          client.println(GAS_RESISTANCE);

          client.print("altitude=");
          client.println(ALTITUDE);

          client.print("calibrated altitude=");
          client.println(CAL_ALTITUDE);

          client.print("aqi=");
          client.println(AQI);

          client.print("tvoc=");
          client.println(TVOC);

          client.print("eco2=");
          client.println(ECO2);

          client.print("co2=");
          client.println(CO2_LEVEL);

          client.print("pm1.0=");
          client.println(LAST_PM1_0);

          client.print("pm2.5=");
          client.println(LAST_PM2_5);

          client.print("pm10.0=");
          client.println(LAST_PM10_0);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
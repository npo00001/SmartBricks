/*
  MAX31856 Thermocouple Test
  --------------------------
  This sketch connects to an Adafruit MAX31856 thermocouple amplifier using software SPI,
  and reads temperature from a K-type thermocouple in °F.

  ✅ Purpose:
    - This setup was used temporarily during development to validate thermocouple behavior
      and compare amplifier output to other sensor setups.
    - Though not part of the final system, this is archived for future teams needing to
      re-implement thermocouples with precision amplifiers.

  🛠️ Hardware:
    - Thermocouple: K-type
    - Amplifier: MAX31856 breakout (Adafruit)
    - Interface: Software SPI

  📌 Note:
    - One-shot mode is used to trigger a temperature reading every second.
    - Includes fault checking to help diagnose wiring or sensor issues.
*/

#include <Adafruit_MAX31856.h>

// === Configure Software SPI Pins ===
// Arguments: CS, MOSI, MISO, SCK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for USB serial connection (esp. for Leonardo)

  Serial.println("MAX31856 Test - One Shot (°F)");

  // Initialize MAX31856 sensor
  if (!maxthermo.begin()) {
    Serial.println("MAX31856 not found. Check wiring!");
    while (1);  // Halt if sensor not found
  }

  // Set thermocouple type to K
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  delay(100);

  // Confirm selected thermocouple type
  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType()) {
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    default: Serial.println("Unknown"); break;
  }
}

void loop() {
  // === Trigger and Read One-Shot Temperature Conversion ===
  maxthermo.triggerOneShot();  // Start a single temperature conversion
  delay(150);                  // Wait for conversion to complete

  // Read temperatures
  float cjTempC = maxthermo.readCJTemperature();           // Cold junction in °C
  float tcTempC = maxthermo.readThermocoupleTemperature(); // Thermocouple tip in °C

  // Convert to °F
  float cjTempF = cjTempC * 9.0 / 5.0 + 32.0;
  float tcTempF = tcTempC * 9.0 / 5.0 + 32.0;

  // Print readings
  Serial.print("Cold Junction Temp (°F): ");
  Serial.println(cjTempF);

  Serial.print("Thermocouple Temp (°F): ");
  Serial.println(tcTempF);

  // === Fault Detection and Diagnostics ===
  uint8_t fault = maxthermo.readFault();
  if (fault) {
    Serial.println("Fault detected!");

    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  }

  Serial.println("------");
  delay(1000);  // Repeat once per second
}

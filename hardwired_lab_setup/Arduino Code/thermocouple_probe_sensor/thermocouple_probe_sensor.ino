#include <Adafruit_MAX31856.h>

// Software SPI: CS, MOSI, MISO, SCK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("MAX31856 Test - One Shot (°F)");

  if (!maxthermo.begin()) {
    Serial.println("MAX31856 not found. Check wiring!");
    while (1);
  }

  // Set K-type thermocouple
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  delay(100);

  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType()) {
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    default: Serial.println("Unknown"); break;
  }
}

void loop() {
  maxthermo.triggerOneShot();
  delay(150);  // wait for conversion

  float cjTempC = maxthermo.readCJTemperature();
  float tcTempC = maxthermo.readThermocoupleTemperature();

  float cjTempF = cjTempC * 9.0 / 5.0 + 32.0;
  float tcTempF = tcTempC * 9.0 / 5.0 + 32.0;

  Serial.print("Cold Junction Temp (°F): ");
  Serial.println(cjTempF);

  Serial.print("Thermocouple Temp (°F): ");
  Serial.println(tcTempF);

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
  delay(1000);
}

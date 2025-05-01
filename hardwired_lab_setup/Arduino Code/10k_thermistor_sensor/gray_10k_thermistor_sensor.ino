#include <Wire.h>
#define I2C_ADDRESS 8  // Set unique address for each Nano sensor
#define DATA_LENGTH 16

#define THERMISTOR_PIN A0
#define SERIES_RESISTOR 9600  // 10kΩ fixed resistor
#define NOMINAL_RESISTANCE 10000  // Thermistor resistance at 25°C
#define NOMINAL_TEMPERATURE 25  // °C
#define B_COEFFICIENT 3435   // Beta coefficient of thermistor
#define ADC_MAX 1023
#define VCC 4.68

char sensorData[DATA_LENGTH] = "NoData";

void setup() {
  Wire.begin(I2C_ADDRESS);   // Start as I2C slave
  Wire.onRequest(sendData);  // Set callback when master requests data
  Serial.begin(9600);
}

void loop() {
  int adcValue = analogRead(THERMISTOR_PIN);
  float voltage = adcValue * VCC / ADC_MAX;

  // Calculate thermistor resistance
  float resistance = SERIES_RESISTOR * (voltage/(VCC-voltage));
  Serial.println(resistance);
  if (resistance <= 0) {
  Serial.println("Invalid resistance — skipping calculation");
  return;
}
  // Convert to temperature using Steinhart-Hart approximation
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
  steinhart = 1.0 / steinhart;
  float tempC = steinhart - 273.15;

  Serial.print("Thermistor Temp: ");
  Serial.println(tempC);

  dtostrf(tempC, 5, 2, sensorData);  // Format for I2C

  delay(2000);
}

void sendData() {
  Wire.write(sensorData);  // Send temperature string via I2C
}
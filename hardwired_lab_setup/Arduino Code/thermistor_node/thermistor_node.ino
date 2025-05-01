/*
  Thermistor Sensor Node for SmartBricks Project
  ----------------------------------------------
  This Arduino Nano reads a 10kΩ NTC thermistor connected in a voltage divider circuit.
  It calculates temperature in °C using the Steinhart-Hart equation and sends the value
  to an I2C master (Hub Arduino) upon request.

  Key Points:
  - Be sure to measure and set the actual VCC voltage (default here is 4.68V).
  - If the analog pin is left floating or disconnected, resistance will be invalid.
  - Sensor readings are updated every 2 seconds.

  I2C Address: set using #define I2C_ADDRESS
  Data Format: string (e.g., " 25.32")
*/

#include <Wire.h>  // I2C communication library

// === I2C Configuration ===
#define I2C_ADDRESS 8       // Unique I2C address for this sensor node
#define DATA_LENGTH 16      // Max number of characters in I2C data string

// === Thermistor Circuit Constants ===
#define THERMISTOR_PIN A0           // Analog input pin connected to thermistor divider
#define SERIES_RESISTOR 10000       // 10kΩ resistor in series with thermistor
#define NOMINAL_RESISTANCE 10000    // Thermistor resistance at 25°C
#define NOMINAL_TEMPERATURE 25      // Nominal temperature in Celsius
#define B_COEFFICIENT 3435          // Beta value from thermistor datasheet
#define ADC_MAX 1023                // 10-bit ADC on Arduino Nano
#define VCC 4.68                    // Actual measured supply voltage to thermistor circuit (may change)

char sensorData[DATA_LENGTH] = "NoData";  // Buffer to store temperature reading as a string

void setup() {
  Wire.begin(I2C_ADDRESS);          // Initialize as I2C slave with specified address
  Wire.onRequest(sendData);         // Register a callback to send data when master requests
  Serial.begin(9600);               // Start serial monitor for debugging
}

void loop() {
  // === Read and Convert Thermistor Voltage ===
  int adcValue = analogRead(THERMISTOR_PIN);         // Read analog value from thermistor divider
  float voltage = adcValue * VCC / ADC_MAX;          // Convert ADC reading to voltage

  // === Calculate Resistance of Thermistor ===
  float resistance = SERIES_RESISTOR * (voltage / (VCC - voltage));  // Using voltage divider formula

  // Debug print resistance value
  Serial.println(resistance);

  // Avoid invalid resistance readings (can occur due to floating analog pins or disconnection)
  if (resistance <= 0) {
    Serial.println("Invalid resistance — skipping calculation");
    return;  // Exit loop early; don’t update I2C data
  }

  // === Convert Resistance to Temperature using the Steinhart-Hart approximation ===
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;   // R/R₀
  steinhart = log(steinhart);                    // ln(R/R₀)
  steinhart /= B_COEFFICIENT;                    // ln(R/R₀) / β
  steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);  // Add 1/T₀ (in Kelvin)
  steinhart = 1.0 / steinhart;                   // Invert to get T (Kelvin)
  float tempC = steinhart - 273.15;              // Convert Kelvin to °C

  // Debug print temperature
  Serial.print("Thermistor Temp: ");
  Serial.println(tempC);

  // === Format and Store Temperature for I2C Transmission ===
  dtostrf(tempC, 5, 2, sensorData);  // Convert float to string with 2 decimal places

  delay(2000);  // Wait before next reading (limits update rate)
}

void sendData() {
  // === I2C Callback: Send Temperature String ===
  Wire.write(sensorData);  // Transmit latest formatted temperature to I2C master
}
/*
  voltage_node.ino
  ---------------------------
  Simulates an analog voltage sensor for the SmartBricks I2C-based system.

  This node behaves like a real sensor: it responds to I2C requests with a 
  formatted voltage string (e.g., "1.25") that the hub can read.

  Used for:
  - Debugging the hub or base station without requiring real sensor input
  - Placeholder nodes when sensors are unavailable
  - Signal emulation for plotting/validation

  Behavior:
  - Sends a fixed voltage value (can be randomized or made dynamic)
*/

#include <Wire.h>

#define I2C_ADDRESS 0x0A     // Change to a unique I2C address (e.g., 0x0A–0x0C)
#define DATA_LENGTH 16       // Max I2C message size
char sensorData[DATA_LENGTH] = "0.00";  // Buffer to hold the outgoing data

void setup() {
  Wire.begin(I2C_ADDRESS);     // Join I2C bus as a slave
  Wire.onRequest(sendData);    // Set callback function for request event
  Serial.begin(9600);          // Optional: Serial output for debug
}

void loop() {
  // Simulate a voltage (can be fixed, random, or sine-based)
  float voltage = 1.23;  // Change this to a desired static or dynamic value

  // Example of generating a changing voltage:
  // float voltage = 1.0 + 0.5 * sin(millis() / 1000.0);

  // Format float as a string for I2C transmission (e.g., "1.23")
  dtostrf(voltage, 4, 2, sensorData);

  // Optional: print to serial for debugging
  Serial.print("Mock Voltage: ");
  Serial.println(sensorData);

  delay(2000);  // Simulate slow sensor update rate
}

void sendData() {
  // I2C Master requested data → send the current voltage string
  Wire.write(sensorData);
}

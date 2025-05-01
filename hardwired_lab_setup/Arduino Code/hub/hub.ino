/*
  Hub Master Node for Hardwired Lab Setup
  ---------------------------------------
  This Arduino Nano acts as the I2C master.

  It queries five sensor nodes (each with a unique I2C address), receives their data, and 
  prints it over serial for logging or debugging.

  - Sensors 1 and 2 return temperature in °C (from thermistors).
  - Sensors 3, 4, and 5 return simulated thermocouple voltages (in V).
  - Sensor data is expected as ASCII strings.

  Data is later parsed and logged on the connected computer via Python.

  I2C Address Map:
    Sensor 1 → 0x08
    Sensor 2 → 0x09
    Sensor 3 → 0x0A
    Sensor 4 → 0x0B
    Sensor 5 → 0x0C
*/

#include <Wire.h>  // I2C communication library

// === I2C addresses of slave sensor nodes ===
#define SENSOR1_ADDRESS 8
#define SENSOR2_ADDRESS 9
#define SENSOR3_ADDRESS 10
#define SENSOR4_ADDRESS 11
#define SENSOR5_ADDRESS 12

// === Data Buffer ===
uint8_t maxDataSize = 32;         // Max number of bytes to request from sensor
char receivedData[32];            // Buffer for storing received I2C string

// Placeholder variables (not used in this version, but useful for expansion)
float voltage = 0.0;
int temp = 0;

void setup() {
  Wire.begin();                   // Start I2C as master
  Serial.begin(9600);             // Start serial monitor
}

void loop() {
  // === Query each sensor and print data to serial ===

  readSensor(SENSOR1_ADDRESS);
  Serial.print("Sensor 1: ");
  Serial.println(receivedData);

  readSensor(SENSOR2_ADDRESS);
  Serial.print("Sensor 2: ");
  Serial.println(receivedData);

  readSensor(SENSOR3_ADDRESS);
  Serial.print("Sensor 3: ");
  Serial.println(receivedData);

  readSensor(SENSOR4_ADDRESS);
  Serial.print("Sensor 4: ");
  Serial.println(receivedData);

  readSensor(SENSOR5_ADDRESS);
  Serial.print("Sensor 5: ");
  Serial.println(receivedData);

  Serial.println("--------------------");

  delay(5000);  // Wait 5 seconds between polling cycles
}

/*
  readSensor(address)
  -------------------
  Requests data from the sensor at the given I2C address.

  - Reads up to 31 printable characters into `receivedData`
  - Terminates the string with null character ('\0')
*/
void readSensor(uint8_t address) {
  Wire.requestFrom(address, maxDataSize);  // Request up to maxDataSize bytes from sensor

  int i = 0;
  while (Wire.available() && i < 31) {
    char c = Wire.read();
    if (c >= 32 && c <= 126) {             // Only accept printable ASCII characters
      receivedData[i++] = c;
    }
  }
  receivedData[i] = '\0';                  // Null-terminate the received string
}

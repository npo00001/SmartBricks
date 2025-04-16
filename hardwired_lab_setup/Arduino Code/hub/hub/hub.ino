#include <Wire.h>

#define SENSOR1_ADDRESS 8
#define SENSOR2_ADDRESS 9
#define SENSOR3_ADDRESS 10
#define SENSOR4_ADDRESS 11
#define SENSOR5_ADDRESS 12

uint8_t maxDataSize = 32;
char receivedData[32];  // Adjust buffer size as needed
float voltage = 0.0;
int temp = 0;

void setup() {
  Wire.begin();            
  Serial.begin(9600);
}

void loop() {
  // Read from sensor 1
  readSensor(SENSOR1_ADDRESS);
  Serial.print("Sensor 1: ");
  Serial.println(receivedData);
  // Read from sensor 2
  readSensor(SENSOR2_ADDRESS);
  Serial.print("Sensor 2: ");
  Serial.println(receivedData);
    // Read from sensor 3
  readSensor(SENSOR3_ADDRESS);
  Serial.print("Sensor 3: ");
  Serial.println(receivedData);
  // Read from sensor 4
  readSensor(SENSOR4_ADDRESS);
  Serial.print("Sensor 4: ");
  Serial.println(receivedData);

  readSensor(SENSOR5_ADDRESS);
  Serial.print("Sensor 5: ");
  Serial.println(receivedData);

  Serial.println("--------------------");
   delay(5000);
}

void readSensor(uint8_t address) {
  Wire.requestFrom(address, maxDataSize);
  int i = 0;

  while (Wire.available() && i < 31) {
    char c = Wire.read();
    if (c >= 32 && c <= 126) {  // Only keep printable ASCII characters
      receivedData[i++] = c;
    }
  }
  receivedData[i] = '\0';  // Null-terminate
}
#include <ArduinoBLE.h>

// To add an additional sensor:
// 1. Add UUID to char arary named UUIDs
// 2. Increase SENSOR_COUNT

#define SENSOR_COUNT 2

char UUIDs[SENSOR_COUNT][37] = {"01000100-0100-1111-0100-0101ffD0E001", "01000100-0100-1111-0100-0101ffD0E002"};
int UUIDindex;

#define DATA_LENGTH 10 // Length of String received over Bluetooth (includes \0)

#define SEARCH_TIMEOUT 500 // ms Time to search for the current RemoteSensor
#define REBOOT_TIMEOUT 15000 // ms Time without connection before reboot (> RS.SamplingDelay + RS.BROADCAST_DURATION)

unsigned long current_time;
unsigned long lastSearchStart;
unsigned long lastRead;

void setup() {
  Serial.begin(9600);

  // Initialize Bluetooth
  BLE.begin();
  BLE.setLocalName("BaseStation");

  // Set scanning for first UUID
  UUIDindex = 0;
  BLE.scanForUuid(UUIDs[UUIDindex]);

  current_time = millis();
  lastSearchStart = current_time;
  lastRead = current_time;
}

void loop() {
  // Check for timeout
  current_time = millis();
  if (current_time - lastRead >= REBOOT_TIMEOUT) {
    Serial.println("Restarting BaseStation");
    delay(1000); // Give time for message to print to Serial
    NVIC_SystemReset();
  } else if (current_time - lastSearchStart >= SEARCH_TIMEOUT) {
    Serial.print("Timeout connecting to device ");
    Serial.println(UUIDindex + 1);

    // Set scanning for next UUID
    UUIDindex = (UUIDindex + 1) % SENSOR_COUNT;
    BLE.stopScan();
    BLE.scanForUuid(UUIDs[UUIDindex]);
    lastSearchStart = millis();
  }

  // Find remoteSensor
  BLEDevice remoteSensor = BLE.available();
  if (!remoteSensor)
    return;

  if (remoteSensor.localName() != "smartanchor2")
    return;

  BLE.stopScan();

  // Read data from RemoteSensor
  char data[DATA_LENGTH];
  if (readSensor(data, DATA_LENGTH, remoteSensor, UUIDs[UUIDindex])) {
    // Print data <sensor> <voltage> <range>
    Serial.print(UUIDindex + 1);
    Serial.print(' ');
    Serial.println(data);

    // Set scanning for next UUID
    UUIDindex = (UUIDindex + 1) % SENSOR_COUNT;

    // Reset timeout tracking
    current_time = millis();
    lastSearchStart = current_time;
    lastRead = current_time;
  }

  // Resume scanning
  BLE.scanForUuid(UUIDs[UUIDindex]);
}

int readSensor(char *data, int data_length, BLEDevice remoteSensor, char *UUID) {
  if (!remoteSensor.connect()) {
    Serial.print("Couldn't connect to device ");
    Serial.println(UUIDindex + 1);
    return 0;
  }

  if (!remoteSensor.discoverAttributes()) {
    Serial.print("Couldn't discover attributes of device ");
    Serial.println(UUIDindex + 1);
    remoteSensor.disconnect();
    return 0;
  }

  BLECharacteristic stringCharacteristic = remoteSensor.characteristic(UUID);
  if (!stringCharacteristic.canRead()) {
    Serial.print("Couldn't read data from device ");
    Serial.println(UUIDindex + 1);
    return 0;
  }

  int len = stringCharacteristic.readValue(data, data_length - 1);
  data[len] = '\0';

  remoteSensor.disconnect();
  return 1;
}

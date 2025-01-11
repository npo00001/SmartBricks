// Wireless Data-logging system for monitoring SMART bricks
// Base Station
// Steven Andryzcik

// Include Arduino Low Energy Bluetooth Library
#include <ArduinoBLE.h>

//to add new UUID:
//1. add UUID to char arary named UUIDs
//2. change length of UUID to match new length
//3. go to end of void loop and change if statement to match new length (if(UUIDcount == 2))

// Set delay between packet transmission
int delayVar = 1000;

char UUIDs[2][37] = {"01000100-0100-1111-0100-0101ffD0E001", "01000100-0100-1111-0100-0101ffD0E002"};
int UUIDcount = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  // Sets built-in LED pin
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize Bluetooth
  BLE.begin();
  BLE.setLocalName("dlBaseStation");

  // start scanning for wireless node
  BLE.scanForUuid(UUIDs[UUIDcount]);
}

void loop() {

  // check if wireless node has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "Nano 33 IoT") {
      return;
    }//end local name if

    // stop scanning
    BLE.stopScan();

    readVoltage(peripheral);

    //peripheral disconnected, start scanning again
    UUIDcount++;
    if(UUIDcount == 2){
    UUIDcount = 0;
    }
    
    BLE.scanForUuid(UUIDs[UUIDcount]);
  }//end peripheral if

  
}//end void loop




void readVoltage(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
    // Turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the voltage characteristic
  BLECharacteristic voltageCharacteristic = peripheral.characteristic(UUIDs[UUIDcount]);

  if (!voltageCharacteristic.canRead()) {
    Serial.println("Peripheral does not have a readable characteristic!");
    peripheral.disconnect();
    return;
  }


  //while (peripheral.connected()) {

  // while the peripheral is connected

  float voltageRet = readVoltageMA(50, voltageCharacteristic);
  
  Serial.print(UUIDs[UUIDcount]);
  Serial.print("\t");
  Serial.print(voltageRet, 4);
  Serial.println();

  peripheral.disconnect();
  //}//end while connected

  Serial.println("Peripheral disconnected");
  // When the central disconnects, turn off the LED:
  digitalWrite(LED_BUILTIN, LOW);
}//end read voltage


float readVoltageMA(int numPoints, BLECharacteristic voltageCharacteristic)
{
  int readings[numPoints];        // the readings from the analog input stored in an array the length of the MA
  int total = 0;                  // the running total
  int average = 0;                // the average
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numPoints; thisReading++) {
    readings[thisReading] = 0;
  }
  for (int readIndex = 0; readIndex < numPoints; readIndex++){
    voltageCharacteristic.read();
    int upperByte = voltageCharacteristic.value()[1] * 16 * 16;
    int lowerByte = voltageCharacteristic.value()[0];
    int sensor2 = upperByte + lowerByte;
    readings[readIndex] = sensor2;
    total = total + readings[readIndex];
  }
  average = total / numPoints;
  float voltage = average * (3.3 / 1023.0);
  return voltage;
}

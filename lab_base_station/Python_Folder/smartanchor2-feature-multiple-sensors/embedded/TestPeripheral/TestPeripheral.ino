#include <ArduinoBLE.h>

//to upload new UUID to a device simply edit the char array named UUID

char UUID[37] = "01000100-0100-1111-0100-0101ffD0E001";

BLEService voltageService(UUID);

//custom 128-bit UUID, read and writable by central
BLEIntCharacteristic voltageChar(UUID, BLERead);

void setup() {
  Serial.begin(9600);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Nano 33 IoT");
  BLE.setAdvertisedService(voltageService);

  // add the characteristic to the service
  voltageService.addCharacteristic(voltageChar);

  // add service
  BLE.addService(voltageService);

  // set the initial value for the characeristic:
  voltageChar.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");

}//end setup

void loop() {

  int sensorValue;  //output of ADC

  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();
  
  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central");
    
    while (central.connected()) {
      
      sensorValue = analogRead(A2);
      voltageChar.writeValue(sensorValue);
      Serial.println(sensorValue, 4);
      delay(100);
      
    }//end while

    //at this point central has disconnected
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }//end central if
}//end loop

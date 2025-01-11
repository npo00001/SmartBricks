#include <bluefruit.h>

//How to add more peripherals:
//1. Increase SENSOR_COUNT
//2. Add new UUIDs to UUIDs array
//3. Add new service and update bcs array
//4. Add new characteristic and update bcc array

#define SENSOR_COUNT 1 //number of peripheral sensors
#define SEARCH_TIMEOUT 120000 //RS.SLEEP_TIME * 2 to give room for a miss and retry
#define DATA_LENGTH 25 // Length of String received over Bluetooth (includes \0)

char UUIDs[SENSOR_COUNT][37] = {"01000100-0100-1111-0100-0101ffD0E001"};
int UUIDindex = 0;

BLEClientService bcs0 = BLEClientService(UUIDs[0]);
//BLEClientService bcs1 = BLEClientService(UUIDs[1]);
//BLEClientService bcs2 = BLEClientService(UUIDs[2]);
BLEClientService* bcs[SENSOR_COUNT] = {&bcs0};

BLEClientCharacteristic bcc0(0x1235);
//BLEClientCharacteristic bcc1(0x1235);
//BLEClientCharacteristic bcc2(0x1235);
BLEClientCharacteristic* bcc[SENSOR_COUNT] = {&bcc0};


void setup() {
  Serial.begin(9600);

  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

  //.begin services and characteristics
  for (int i = 0; i < SENSOR_COUNT; i++) {
    bcs[i]->begin();
    bcc[i]->begin();
  }

  Bluefruit.setConnLedInterval(250);

  Bluefruit.Central.setConnectCallback(connect_callback); //call this function when connection made
  Bluefruit.Central.setDisconnectCallback(disconnect_callback); //call this function on disconnection

  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);  //true = continue scanning after disconnect; false = stop scanning
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms //original: 160, 80

  //filter for UUIDs
  BLEUuid uuids[SENSOR_COUNT];
  for (int i = 0; i < SENSOR_COUNT; i++) {
    uuids[i] = bcs[i]->uuid;
  }
  Bluefruit.Scanner.filterUuid(uuids, SENSOR_COUNT);

  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
}

/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with uuid put in filterUuid()
  // Connect to device
  Bluefruit.Central.connect(report);
}

/**
   Callback invoked when a connection is made
   @param conn_handle
*/
void connect_callback(uint16_t conn_handle)
{
  // Make sure service exists
  if ( !bcs[UUIDindex]->discover(conn_handle) )
  {
    Serial.println("Could not discover service");
    Bluefruit.disconnect(conn_handle);
    return;
  }

  // Make sure characteristic exists
  if (!bcc[UUIDindex]->discover() )
  {
    Serial.println("Could not discover characteristic");
    Bluefruit.disconnect(conn_handle);
    return;
  }

  if (Bluefruit.connected()) {
    //collect data and prepare for printing
    char data[DATA_LENGTH] = "No Data";
    bcc[UUIDindex]->read(data, DATA_LENGTH);
    data[DATA_LENGTH - 1] = '\0';

    // Print data <sensor> <wheatstone voltage> <range> <log voltage> <battery voltage>
    Serial.print(UUIDindex + 1);
    Serial.print(' ');
    Serial.println(data);

    UUIDindex = (UUIDindex + 1) % SENSOR_COUNT; //update UUIDindex to look for next sensor

    Bluefruit.disconnect(conn_handle);
  }
}

//function is called when disconnect occurs
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;
  Bluefruit.Scanner.filterUuid(bcs[UUIDindex]->uuid);
}

void loop() {
    //if too much time passes start looking for the next sensor
    if (!Bluefruit.connected()) {
      unsigned long start_time = millis();
      while (1) {
        if (millis() - start_time >= SEARCH_TIMEOUT) {
          Serial.println("Sensor Not Found");
          UUIDindex = (UUIDindex + 1) % SENSOR_COUNT;
          Bluefruit.Scanner.filterUuid(bcs[UUIDindex]->uuid);
          break;
        }
        else if (Bluefruit.connected()) {
          break;
        }
      }
    }
}

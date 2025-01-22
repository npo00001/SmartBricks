#include <bluefruit.h>
#include <Wire.h>

// How to add more peripherals:
// 1. Increase SENSOR_COUNT
// 2. Add new UUIDs to UUIDs array
// 3. Add new service and update bcs array
// 4. Add new characteristic and update bcc array

//#define SENSOR_COUNT 6  // number of peripheral sensors along with 3 K-type TC
#define SENSOR_COUNT 3   // without K-type TC
#define SEARCH_TIMEOUT 120000  // RS.SLEEP_TIME * 2 to give room for a miss and retry
#define DATA_LENGTH 25  // Length of String received over Bluetooth (includes \0)
#define SLAVE_ADDR 9
#define ANSWERSIZE 28
#define WAKE_PIN 6

//char UUIDs[SENSOR_COUNT][37] = {"01000100-0100-1111-0100-0101ffD0E001","01000100-0100-1111-0100-0101ffD0E002","01000100-0100-1111-0100-0101ffD0E003","01000100-0100-1111-0100-0101ffD0E004","01000100-0100-1111-0100-0101ffD0E005","01000100-0100-1111-0100-0101ffD0E006"};
char UUIDs[SENSOR_COUNT][37] = {"01000100-0100-1111-0100-0101ffD0E001","01000100-0100-1111-0100-0101ffD0E002","01000100-0100-1111-0100-0101ffD0E003"};
//char UUIDs[SENSOR_COUNT][37] = {"01000100-0100-1111-0100-0101ffD0E001"};
int UUIDindex = 0;

BLEClientService bcs0 = BLEClientService(UUIDs[0]);
BLEClientService bcs1 = BLEClientService(UUIDs[1]);
BLEClientService bcs2 = BLEClientService(UUIDs[2]);
//BLEClientService bcs3 = BLEClientService(UUIDs[3]);
//BLEClientService bcs4 = BLEClientService(UUIDs[4]);
//BLEClientService bcs5 = BLEClientService(UUIDs[5]);
//BLEClientService *bcs[SENSOR_COUNT] = {&bcs0,&bcs1,&bcs2,&bcs3,&bcs4,&bcs5};
BLEClientService *bcs[SENSOR_COUNT] = {&bcs0,&bcs1,&bcs2};

BLEClientCharacteristic bcc0(0x1235);
BLEClientCharacteristic bcc1(0x1235);
BLEClientCharacteristic bcc2(0x1235);
//BLEClientCharacteristic bcc3(0x1235);
//BLEClientCharacteristic bcc4(0x1235);
//BLEClientCharacteristic bcc5(0x1235);
//BLEClientCharacteristic *bcc[SENSOR_COUNT] = {&bcc0,&bcc1,&bcc2,&bcc3,&bcc4,&bcc5};
BLEClientCharacteristic *bcc[SENSOR_COUNT] = {&bcc0,&bcc1,&bcc2};

String my_ans;  // Global variable to store the answer string

unsigned long StartTime = 0;
unsigned long sleepDuration = 120000; // 45 seconds in milliseconds
unsigned long wakeDuration = 25000; // 30 seconds in milliseconds

void setup()
{
  
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  

  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

  //.begin services and characteristics
  for (int i = 0; i < SENSOR_COUNT; i++)
  {
    bcs[i]->begin();
    bcc[i]->begin();
  }

  Bluefruit.setConnLedInterval(1000); //default=250
  Bluefruit.Scanner.setRxCallback(scan_callback);

  Bluefruit.Central.setConnectCallback(connect_callback);       // call this function when connection made
  Bluefruit.Central.setDisconnectCallback(disconnect_callback); // call this function on disconnection

  Bluefruit.Scanner.restartOnDisconnect(true); // true = continue scanning after disconnect; false = stop scanning
  Bluefruit.Scanner.setInterval(320, 80);      // in unit of 0.625 ms //original: 160, 80

  // filter for UUIDs
  BLEUuid uuids[SENSOR_COUNT];
  for (int i = 0; i < SENSOR_COUNT; i++)
  {
    uuids[i] = bcs[i]->uuid;
  }
  Bluefruit.Scanner.filterUuid(uuids, SENSOR_COUNT);

  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
  pinMode(WAKE_PIN, OUTPUT);
}

/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t *report)
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
  if (!bcs[UUIDindex]->discover(conn_handle))
  {
    Serial.println("Could not discover service");
    Bluefruit.disconnect(conn_handle);
    return;
  }

  // Make sure characteristic exists
  if (!bcc[UUIDindex]->discover())
  {
    Serial.println("Could not discover characteristic");
    Bluefruit.disconnect(conn_handle);
    return;
  }

  if (Bluefruit.connected())
  {
    // Collect data and prepare for printing
    char data[DATA_LENGTH] = "No Data";
    bcc[UUIDindex]->read(data, DATA_LENGTH);
    data[DATA_LENGTH - 1] = '\0';

    my_ans = String(UUIDindex + 1) + "-" + " " + "-" + String(data);  // Update the global variable

    Serial.println(my_ans);

    Wire.onRequest(requestEvent);

    UUIDindex = (UUIDindex + 1) % SENSOR_COUNT; // update UUIDindex to look for the next sensor

    Bluefruit.disconnect(conn_handle);
    delay(3500);
  }
}

// Function is called when disconnect occurs
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void)conn_handle;
  (void)reason;
  Bluefruit.Scanner.filterUuid(bcs[UUIDindex]->uuid);
}

void loop()
{
  if (millis() - StartTime <= wakeDuration)
  {
// If too much time passes, start looking for the next sensor
  if (!Bluefruit.connected())
  {
    unsigned long start_time = millis();
    //unsigned long start_time = millis();
    while (1)
    {
      if (millis() - start_time >= SEARCH_TIMEOUT)
      {
        Serial.println("Sensor Not Found");
        UUIDindex = (UUIDindex + 1) % SENSOR_COUNT;
        Bluefruit.Scanner.filterUuid(bcs[UUIDindex]->uuid);
        break;
      }
      else if (Bluefruit.connected())
      {
        //Serial.println(millis() - start_time);
        //Serial.println("connected!!!!!!!!!!!!!!!!!!!");
        break;
      }
    }
  }
  }
  else {
    Serial.println("Entering sleep mode...");
    Bluefruit.Scanner.start(10);
    digitalWrite(WAKE_PIN, LOW);

    delay(sleepDuration);
    Serial.println("Waking up...");
    Bluefruit.Scanner.start(0);
    digitalWrite(WAKE_PIN, HIGH);

    StartTime = millis();


  }
}

void receiveEvent(int howmany)
{
  // Serial.println("yes");
  while (Wire.available())
  {
    byte x = Wire.read();
    // Serial.println(x);
  }
}

void requestEvent()
{
  // Check if the NRF52840 module is awake
  if (isAwake())
  {
    // The NRF52840 module is awake, proceed with sending data
    String answer = my_ans;
    byte response[ANSWERSIZE];
    for (byte i = 0; i < ANSWERSIZE; i++)
    {
      response[i] = (byte)answer.charAt(i);
    }
    Wire.write(response, sizeof(response));
    Serial.println("Data sent to master Lora module.");
  }
  else
  {
    // The NRF52840 module is sleeping, do not send data
    Serial.println("NRF52840 module is sleeping. No data sent.");
  }
}

// Function to check if the NRF52840 module is awake
bool isAwake()
{
  // Add your logic to determine if the module is awake
  // For example, check a pin state, internal timer, or any other mechanism
  // Return true if awake, false if sleeping
  // Replace the following line with your actual logic
  return digitalRead(WAKE_PIN) == HIGH; // Change WAKE_PIN to the actual pin used to detect module's wake state
}

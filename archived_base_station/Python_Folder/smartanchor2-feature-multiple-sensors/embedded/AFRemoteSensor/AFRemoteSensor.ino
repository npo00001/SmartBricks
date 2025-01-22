#include <bluefruit.h>

#define SLEEP_TIME 60000 // ms sleep duration 
#define BROADCAST_DURATION  1500 // ms Time to make data available via Bluetooth
#define CIRCUIT_DELAY 500 //amount of time to leave the circuit powered before taking a sample

#define DATA_LENGTH 25 // Length of String sent over Bluetooth (includes \0)
#define ADC_RESOLUTION 14 //can be 8, 10, 12, 14

#define POWER 6 //LOW turns power off HIGH keeps it on
#define WHEAT_LOG_SEL 9 //select for wheatstone(LOW) or logamp(HIGH) multiplexer
#define S1 10 //select pin for multiplexer
#define S0 11 //select pin for multiplexer

#define WHEATSTONE_OUTPUT A0 //ouput of circuit (analog)
#define LOG_OUTPUT A2 //output of logarithmic amplifier
#define VBAT A6 //output voltage of the attached battery

// Define circuit rail voltages
#define V_MAX 3.3
#define V_MIN 0
#define V_MID 1.65

// Define voltage thresholds for switching to higher/lower range
#define MAX_SWITCH 3.15   // 3.15 V upper cutoff
#define MIN_SWITCH 0.15   // 0.15 V lower cutoff

// Define ranges
#define MAX_RANGE 4
#define MIN_RANGE 1

// Define measurement specifications
#define PRECISION 5
#define SAMPLES_PER_READING 5000

char UUID[37] = "01000100-0100-1111-0100-0101ffD0E001";
char data[DATA_LENGTH] = "NoData";
int currentRange;

BLEService sensorService = BLEService(UUID);
BLECharacteristic sensorCharacteristic = BLECharacteristic(0x1235);

BLEDis bledis; //device information serive (Model, Manufacturer,...)

void setup()
{
  Bluefruit.begin();

  Bluefruit.Periph.setConnectCallback(connect_callback); //set ISR for when connection is made
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback); //set ISR for when disconnection occurs

  //device information
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  setupService();
  startAdv();

  //set digital pins as outputs
  pinMode(S1, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(WHEAT_LOG_SEL, OUTPUT);
  pinMode(POWER, OUTPUT);

  // Set starting range
  currentRange = findBestRange();
  setRange(currentRange);

  digitalWrite(POWER, LOW);

  analogReadResolution(ADC_RESOLUTION); //determines ADC resolution
  analogReference(AR_VDD4); //set reference voltage equal to VDD (3.3V)

  //turn off LEDs
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(PIN_NEOPIXEL, LOW);

}

void setupService(void) {
  sensorService.begin();
  sensorCharacteristic.setFixedLen(DATA_LENGTH);  //sets data length of data to send
  sensorCharacteristic.setProperties(CHR_PROPS_READ); //allows characteristic to be read
  sensorCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN); //(read, write) can read and write

  //Calling .begin on a characteristic will add it to the last service that had .begin called on it
  sensorCharacteristic.begin();
}

void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include HRM Service UUID
  Bluefruit.Advertising.addService(sensorService);

  // Include Name
  Bluefruit.Advertising.addName();

  Bluefruit.Advertising.restartOnDisconnect(false); //false = do not resume advertising after disconnect
  Bluefruit.Advertising.setInterval(32, 244);  //Advertising intervals (fast, slow) (must be specific values)
}

void connect_callback(uint16_t conn_handle)
{
  digitalWrite(LED_BLUE, LOW); //turn off connection LED

  // Read log voltage
  digitalWrite(POWER, HIGH);
  digitalWrite(WHEAT_LOG_SEL, HIGH);
  float measLog = readVoltageAvg(SAMPLES_PER_READING, LOG_OUTPUT);

  //read wheatstone voltage
  digitalWrite(WHEAT_LOG_SEL, LOW);
  delay(CIRCUIT_DELAY);
  float measWheat = readVoltageAvg(SAMPLES_PER_READING, WHEATSTONE_OUTPUT);

  //read battery voltage
  float measVBat = getBatV(SAMPLES_PER_READING);
  digitalWrite(POWER, LOW);

  //write data to characteristic
  snprintf(data, DATA_LENGTH, "%.5f %d %.5f %.5f", measWheat, currentRange, measLog, measVBat);
  sensorCharacteristic.write(data);

  // Update range if needed
  if ((measWheat > MAX_SWITCH) && (currentRange < MAX_RANGE))
    setRange(++currentRange);
  else if ((measWheat < MIN_SWITCH) && (currentRange > MIN_RANGE))
    setRange(--currentRange);

  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  //stop advertising and go to sleep
  Bluefruit.Advertising.stop();
  delay(SLEEP_TIME);
}

void loop()
{
  if (!Bluefruit.connected()) {
    Bluefruit.Advertising.start(BROADCAST_DURATION); //stops broadcasting after BROADCOST_DURATION ms
    unsigned long start_time = millis();

    //stop broadcasting if connection is made
    while (millis() - start_time <=  BROADCAST_DURATION) {
      if (Bluefruit.connected()) {
        break;
      }
    }

    //stop advertising and go to sleep if no central picked up data in time
    Bluefruit.Advertising.stop();
    delay(SLEEP_TIME); //delay is sleep
  }
}

// Find range closest to midrail. Takes 0.5 seconds based on delay of 125
int findBestRange()
{

  digitalWrite(WHEAT_LOG_SEL, LOW);  //select Wheatstone

  int bestRange = MIN_RANGE;
  float minDist = V_MAX;

  //turn on Wheatstone bridge
  digitalWrite(POWER, HIGH);
  digitalWrite(WHEAT_LOG_SEL, LOW);
  delay(150);

  for (int range = MIN_RANGE; range <= MAX_RANGE; range++) {
    // Measure distance between voltage at current range and midrail. Smaller is better
    setRange(range);
   
    float currentDist = abs(readVoltageAvg(1, WHEATSTONE_OUTPUT) - V_MID);

    // Update bestRange if a better range is found
    if (currentDist < minDist) {
      minDist = currentDist;
      bestRange = range;
    }
    delay(125);
  }

  digitalWrite(POWER, LOW);

  return bestRange;
}

// Read the voltage at pin WHEATSTONE_OUTPUT as an average of numSamples number of points
float readVoltageAvg(int numSamples, int analogPin)
{
  delay(100);

  // Sum numSamples readings
  long total = 0;
  for (int i = 0; i < numSamples; i++)
    total += analogRead(analogPin);

  // Compute voltage from average of readings
  float average = (float) total / (float) numSamples;
  float voltage = average * (3.3 / (float)(pow(2, ADC_RESOLUTION) - 1));

  return voltage;
}

float getBatV(int numSamples) {
  //4.2 volts is full and 3.2 is empty

  long total = 0;
  for (int i = 0; i < numSamples; i++)
    total += analogRead(VBAT);
  
  float measuredvbat = (float) total / (float)numSamples;
  measuredvbat *= 2.0;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= (pow(2, ADC_RESOLUTION) - 1); // convert to voltage

  return measuredvbat;
}

// Set the current range (1, 2, 3, or 4) using the multiplexer, with select pins S1 and S0.
void setRange(int range)
{
  // Set the desired range by individually setting the select pins high or low
  if (range == 1)
  {
    digitalWrite(S1, LOW); // 00
    digitalWrite(S0, LOW);
  }
  else if (range == 2)
  {
    digitalWrite(S1, LOW); // 01
    digitalWrite(S0, HIGH);
  }
  else if (range == 3)
  {
    digitalWrite(S1, HIGH); // 10
    digitalWrite(S0, LOW);
  }
  else if (range == 4)
  {
    digitalWrite(S1, HIGH); // 11
    digitalWrite(S0, HIGH);
  }
}

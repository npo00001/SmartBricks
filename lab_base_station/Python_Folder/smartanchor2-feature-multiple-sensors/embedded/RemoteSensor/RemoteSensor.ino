#include <ArduinoBLE.h>
#include <ArduinoLowPower.h>

// Bluetooth UUID
char UUID[37] = "01000100-0100-1111-0100-0101ffD0E002";

#define DATA_LENGTH 10 // Length of String sent over Bluetooth (includes \0)
#define SLEEP_TIME 5000 // ms sleep duration | SamplingDelay (~10000) = Sample Time (5000) + SLEEP_TIME (~5000)
#define BROADCAST_DURATION  1500 // ms Time to make data available via Bluetooth (> BS.SEARCH_TIMEOUT * BS.SENSOR_COUNT)

BLEService remoteSensorService(UUID);
BLECharacteristic stringCharacteristic(UUID, BLERead, DATA_LENGTH);

// Define select pins, can be any two digital pins
#define S1 2
#define S0 3
#define CIRCUIT_OUTPUT A2

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

int currentRange;

void setup()
{
  // Bluetooth preparation
  BLE.setLocalName("smartanchor2");
  remoteSensorService.addCharacteristic(stringCharacteristic);
  BLE.setAdvertisedService(remoteSensorService);

  // Define select pins as outputs
  pinMode(S1, OUTPUT);
  pinMode(S0, OUTPUT);

  // Set starting range
  currentRange = findBestRange();
  setRange(currentRange);
}

void loop()
{
  // Take measurement
  float meas = readVoltageAvg(SAMPLES_PER_READING);
  char data[DATA_LENGTH];
  snprintf(data, DATA_LENGTH, "%.5f %d", meas, currentRange);

  // Update range if needed
  if ((meas > MAX_SWITCH) && (currentRange < MAX_RANGE))
    setRange(++currentRange);
  else if ((meas < MIN_SWITCH) && (currentRange > MIN_RANGE))
    setRange(--currentRange);

  // Enable Bluetooth 
  BLE.begin();
  BLE.addService(remoteSensorService);
  BLE.advertise();

  // Make data available via Bluetooth for BROADCAST_DURATION ms
  unsigned long start_time = millis();
  while (millis() - start_time <= BROADCAST_DURATION) {
    // Check for Bluetooth connection
    BLEDevice central = BLE.central();
    if (central) {
      stringCharacteristic.writeValue(data);
      while (central.connected()); // Wait to disconnect
      break; // Stop broadcasting after 1 successful transmission
    }
  }

  // Disable Bluetooth
  BLE.stopAdvertise();
  BLE.end();

  // Go to sleep then reset
  LowPower.deepSleep(SLEEP_TIME);
  //FIND SOMETHING MORE EFFICIENT THAN RESETTING EVERY TIME
  NVIC_SystemReset();
}

// Find range closest to midrail. Takes 0.5 seconds based on delay of 125
int findBestRange()
{
  int bestRange = MIN_RANGE;
  float minDist = V_MAX;

  for (int range = MIN_RANGE; range <= MAX_RANGE; range++) {
    // Measure distance between voltage at current range and midrail. Smaller is better
    setRange(range);
    float currentDist = abs(readVoltageAvg(1) - V_MID);

    // Update bestRange if a better range is found
    if (currentDist < minDist) {
      minDist = currentDist;
      bestRange = range;
    }

    delay(125);
  }

  return bestRange;
}

// Read the voltage at pin CIRCUIT_OUTPUT as an average of numSamples number of points
float readVoltageAvg(int numSamples)
{
  // Sum numSamples readings
  long total = 0;
  for (int i = 0; i < numSamples; i++)
    total += analogRead(CIRCUIT_OUTPUT);

  // Compute voltage from average of readings
  float average = (float) total / numSamples;
  float voltage = average * (3.3 / 1023.0);

  return voltage;
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

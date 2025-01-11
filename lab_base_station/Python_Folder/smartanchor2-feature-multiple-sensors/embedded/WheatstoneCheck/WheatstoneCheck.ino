#include <bluefruit.h>

#define ADC_RESOLUTION 14 //can be 8, 10, 12, 14
#define POWER 6 //LOW turns power off HIGH keeps it on
#define WHEAT_LOG_SEL 9 //select for wheatstone(LOW) or logamp(HIGH) multiplexer
#define S1 10 //select pin for multiplexer
#define S0 11 //select pin for multiplexer
#define WHEATSTONE_OUTPUT A0 //ouput of circuit (analog)

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

void setup() {
  //set digital pins as outputs
  pinMode(S1, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(WHEAT_LOG_SEL, OUTPUT);
  pinMode(POWER, OUTPUT);

  // Set starting range
  currentRange = findBestRange();
  setRange(currentRange);

  digitalWrite(POWER, HIGH);

  analogReadResolution(ADC_RESOLUTION); //determines ADC resolution
  analogReference(AR_VDD4); //set reference voltage equal to VDD (3.3V)

  digitalWrite(WHEAT_LOG_SEL, LOW);

}

void loop() {

  //read wheatstone voltage
  float measWheat = readVoltageAvg(SAMPLES_PER_READING, WHEATSTONE_OUTPUT);

  Serial.print("Wheatstone V: ");
  Serial.print(measWheat, 4);
  Serial.print("\t Range: ");
  Serial.println(currentRange);

  // Update range if needed
  if ((measWheat > MAX_SWITCH) && (currentRange < MAX_RANGE))
    setRange(++currentRange);
  else if ((measWheat < MIN_SWITCH) && (currentRange > MIN_RANGE))
    setRange(--currentRange);

    delay(1000);
}




// Find range closest to midrail. Takes 0.5 seconds based on delay of 125
int findBestRange()
{

  int bestRange = MIN_RANGE;
  float minDist = V_MAX;

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

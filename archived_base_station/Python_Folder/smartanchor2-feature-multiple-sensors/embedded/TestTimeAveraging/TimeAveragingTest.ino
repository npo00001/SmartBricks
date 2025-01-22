#define CIRCUIT_OUTPUT A2
#define RANGE 1
#define PRECISION 5
#define SAMPLES_PER_READING 5000

void setup() {
  Serial.begin(9600); 
}

void loop() {
  float voltage = readVoltageAvg(SAMPLES_PER_READING);
  Serial.print(voltage, PRECISION);
  Serial.print(" ");
  Serial.println(RANGE);
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

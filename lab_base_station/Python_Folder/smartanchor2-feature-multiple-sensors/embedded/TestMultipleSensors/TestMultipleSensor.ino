#define CIRCUIT_OUTPUT A2
#define RANGE 1
#define PRECISION 5
#define SAMPLES_PER_READING 5000

#define NUM_SENSORS 5
#define SENSOR_OFFSET .2

void setup() {
  Serial.begin(9600); 
}

void loop() {
  float voltage = readVoltageAvg(SAMPLES_PER_READING); // sensors can take measurements in parallel
  for (int sensorID = 1; sensorID <= NUM_SENSORS; sensorID++) {
    float adjustedVoltage = voltage + sensorID * SENSOR_OFFSET; // adjust voltage so each sensor has a unique output
    Serial.print(sensorID);
    Serial.print(" ");
    Serial.print(adjustedVoltage, PRECISION);
    Serial.print(" ");
    Serial.println(RANGE);
    delay(1000); // switching time between sensors
  }
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

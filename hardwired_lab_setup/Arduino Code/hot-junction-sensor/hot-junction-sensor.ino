// Using Adafruit Feather nRF52840
#include <Wire.h>

#define I2C_ADDRESS 8  // Set unique address for each Nano sensor
#define ANALOG_VALUE_SCALE A0

#define CIRCUIT_DELAY 500
#define ADC_RESOLUTION 10  // Arduino Nano default
#define DATA_LENGTH 32
#define SAMPLES_PER_READING 500

char sensorData[DATA_LENGTH] = "NoData";

const int numReducedEntries = 184;
float inputValues[numReducedEntries] = {-0.015, -0.00600000000000001, 0.003, 0.012, 0.021, 0.03, 0.039, 0.048, 0.057, 0.066, 0.083, 0.101, 0.119, 0.137, 0.155, 0.173, 0.191, 0.209, 0.227, 0.245, 0.263, 0.281, 0.299, 0.317, 0.335, 0.353, 0.371, 0.389, 0.407, 0.425, 0.443, 0.461, 0.479, 0.497, 0.515, 0.533, 0.551, 0.569, 0.587, 0.605, 0.623, 0.641000000000001, 0.659000000000001, 0.677000000000001, 0.695000000000001, 0.713000000000001, 0.731000000000001, 0.749000000000001, 0.767000000000001, 0.785000000000001, 0.803000000000001, 0.821000000000001, 0.839000000000001, 0.857000000000001, 0.875000000000001, 0.893000000000001, 0.911000000000001, 0.929000000000001, 0.947000000000001, 0.965000000000001, 0.983000000000001, 1.001, 1.019, 1.037, 1.055, 1.073, 1.091, 1.109, 1.127, 1.145, 1.163, 1.181, 1.199, 1.217, 1.235, 1.253, 1.271, 1.289, 1.307, 1.325, 1.343, 1.361, 1.379, 1.397, 1.415, 1.433, 1.451, 1.469, 1.487, 1.505, 1.523, 1.541, 1.559, 1.577, 1.595, 1.613, 1.631, 1.649, 1.667, 1.685, 1.703, 1.721, 1.739, 1.757, 1.775, 1.793, 1.811, 1.829, 1.847, 1.865, 1.883, 1.901, 1.919, 1.937, 1.955, 1.973, 1.991, 2.009, 2.027, 2.045, 2.063, 2.081, 2.099, 2.117, 2.135, 2.153, 2.171, 2.189, 2.207, 2.225, 2.243, 2.261, 2.279, 2.297, 2.315, 2.333, 2.351, 2.369, 2.387, 2.405, 2.423, 2.441, 2.459, 2.477, 2.495, 2.513, 2.531, 2.549, 2.567, 2.585, 2.603, 2.621, 2.639, 2.657, 2.675, 2.693, 2.711, 2.729, 2.747, 2.765, 2.783, 2.801, 2.819, 2.837, 2.855, 2.873, 2.891, 2.909, 2.927, 2.945, 2.963, 2.981, 2.999, 3.017, 3.035, 3.053, 3.071, 3.089, 3.107, 3.125, 3.143, 3.161, 3.179, 3.197};
float outputValues[numReducedEntries] = {-0.45, -0.36, -0.27, -0.11, -0.02, 0.07, 0.16, 0.25, 0.34, 0.430000000000001, 8, 17, 26, 35, 44, 53, 62, 71, 80, 89, 98, 107, 116, 125, 134, 143, 152, 161, 170, 179, 188, 197, 206, 215, 224, 233, 242, 251, 260, 269, 278, 287, 296, 305, 314, 323, 332, 341, 350, 359, 368, 377, 386, 395, 404, 413, 422, 431, 440, 449, 458, 467, 476, 485, 494, 503, 512, 521, 530, 539, 548, 557, 566, 575, 584, 593, 602, 611, 620, 629, 638, 647, 656, 665, 674, 683, 692, 701, 710, 719, 728, 737, 746, 755, 764, 773, 782, 791, 800, 809, 818, 827, 836, 845, 854, 863, 872, 881, 890, 899, 908, 917, 926, 935, 944, 953, 962, 971, 980, 989, 998, 1007, 1016, 1025, 1034, 1043, 1052, 1061, 1070, 1079, 1088, 1097, 1106, 1115, 1124, 1133, 1142, 1151, 1160, 1169, 1178, 1187, 1196, 1205, 1214, 1223, 1232, 1241, 1250, 1259, 1268, 1277, 1286, 1295, 1304, 1313, 1322, 1331, 1340, 1349, 1358, 1367, 1376, 1385, 1394, 1403, 1412, 1421, 1430, 1439, 1448, 1457, 1466, 1475, 1484, 1493, 1502, 1511, 1520, 1529, 1538, 1547, 1556, 1565};

void setup() {
  Wire.begin(I2C_ADDRESS);   // Start as I2C slave
  Wire.onRequest(sendData);  // Set callback when master requests data
  Serial.begin(9600);
}

void loop() {

  delay(CIRCUIT_DELAY);
  char analogStr[10];
  char thermoStr[10];
  float analogVoltage = readVoltageAvg(SAMPLES_PER_READING, ANALOG_VALUE_SCALE);

  int lowerIdx = 0;
  int upperIdx = numReducedEntries - 1;

  int thermocoupleTemp = 0;  // Placeholder

  for (int i = 1; i < numReducedEntries; i++) {
    if (analogVoltage <= inputValues[i]) {
      upperIdx = i;
      break;
    }
    lowerIdx = i;
  }

  thermocoupleTemp = (outputValues[lowerIdx] + outputValues[upperIdx]) / 2;

  dtostrf(analogVoltage, 5, 2, analogStr);
  sprintf(thermoStr, "%d", thermocoupleTemp);

  snprintf(sensorData, DATA_LENGTH, "%s %s", analogStr, thermoStr);

  delay(2000);  // Adjust as needed
}

void sendData() {
  Wire.write(sensorData);  // Send string as bytes
}

// Read averaged analog voltage
float readVoltageAvg(int numSamples, int analogPin) {
  long total = 0;
  for (int i = 0; i < numSamples; i++) {
    total += analogRead(analogPin);
  }
  float average = total / (float)numSamples;
  return (average * 4.65) / 1023.0;  // Convert to voltage (Nano uses 5V ref)
}
# Legacy_Tests

This folder contains archived test sketches that were used during the development of the SmartBricks sensor network but were not included in the final deployed system.

These files are preserved for reference and may be helpful for future teams interested in expanding or refining sensor capabilities.

---

## Files

### `max31856_test_k_thermocouple.ino`

**Purpose:**  
Test sketch for reading temperature from a K-type thermocouple using the Adafruit MAX31856 amplifier.

**Details:**
- Uses software SPI (pins 10–13) to interface with the breakout board
- Reads both cold junction and thermocouple tip temperatures
- Converts values to degrees Fahrenheit
- Detects and reports common faults (open circuit, over/under voltage, temperature range errors)

**Note:**  
This sketch was used to verify thermocouple amplifier behavior early in the project. In the final implementation, thermocouple inputs were emulated using direct analog voltages for simplicity.

---

## Future Use

If future SmartBricks versions require accurate thermocouple support:
- Use this sketch to validate MAX31856 operation
- Adapt it into the I2C sensor node framework to integrate with the hub system
- Compare results with emulated analog sensors to evaluate performance benefits
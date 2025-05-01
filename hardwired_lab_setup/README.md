# Hardwired Lab Setup

This directory contains the code and setup for a hardwired sensor network used in the SmartBricks project.

## Overview

This system simulates a thermal monitoring network using:
- **2 thermistor-based sensors** (Sensor 1 and 2): These read analog voltages from NTC thermistor dividers and **convert to temperature in °C** on the Arduino.
- **3 voltage-emulating sensors** (Sensor 3, 4, and 5): These simulate amplified thermocouple outputs by directly outputting analog voltages. The voltage is **read and sent as-is**.

All five sensors are connected to **individual Arduino Nano nodes** acting as I2C slaves. A **Hub Arduino Nano** acts as the I2C master and relays the collected sensor data over USB serial to a computer.

A Python script running on the computer:
- Reads serial data from the Hub
- Logs it to a CSV file with a timestamp
- Plots the values live (both voltages and temperatures)

> ⚠️ Important: For voltage-emulating sensors, **floating analog inputs will result in unstable readings**. Ensure all analog pins are either grounded or connected to a voltage source during operation.

## Hardware Summary

- **Sensors 1–2**: 10kΩ NTC thermistors with onboard temp conversion (°C output)
- **Sensors 3–5**: Analog voltage sources simulating thermocouples (V output)
- **Each sensor**: Arduino Nano I2C slave
- **Hub**: Arduino Nano I2C master, connected to computer via USB
- **Computer**: Logs and visualizes data using Python

## Folder Structure

hardwired_lab_setup/ 
├── BaseStation Code/ # Python script for logging and plotting 
│     ├── live_plot_logger.py 
│     └── data_logs/ # Auto-created folder for CSV output 
├── Sensor Code/ # Arduino code for I2C slave nodes 
│     ├── thermistor_node.ino # For Sensors 1 & 2 
│     └── voltage_node.ino # For Sensors 3–5 
├── Hub Code/ # Arduino code for I2C master 
│     └── hub_master.ino 
├── wiring_diagram.png # (Optional, add if available) 
└── README.md # You're here!

## Sensor Map

| Sensor | Arduino Code       | I2C Address | Output Type       |
|--------|--------------------|-------------|-------------------|
| 1      | thermistor_node.ino| 0x08        | Temp in °C        |
| 2      | thermistor_node.ino| 0x09        | Temp in °C        |
| 3      | voltage_node.ino   | 0x10        | Analog Voltage (V)|
| 4      | voltage_node.ino   | 0x11        | Analog Voltage (V)|
| 5      | voltage_node.ino   | 0x12        | Analog Voltage (V)|

## Running the System

1. Upload `thermistor_node.ino` to 2 Arduino Nanos (Sensors 1 & 2), changing the I2C address for each.
2. Upload `voltage_node.ino` to 3 Arduino Nanos (Sensors 3–5), with unique I2C addresses.
3. Upload `hub_master.ino` to the Hub Nano.
4. Plug the Hub into your computer via USB.
5. Run `live_plot_logger.py` from the `BaseStation Code/` folder.
6. The script will auto-log to the `data_logs/` folder and show a live graph.

## Notes for Developers

- Data is logged in CSV format:  
  `Timestamp, Sensor, Voltage, Temperature`
- Only Sensors 1 & 2 return temperature. The rest log `Temperature = 0` by default.
- The Python script filters sensor readings based on expected sensor names.

## Troubleshooting

- **Incorrect/floating voltage readings**: Ensure all analog pins are either connected to known voltage levels or grounded.
- **No serial port found**: Edit `find_serial_port()` in `live_plot_logger.py` to match your OS and connected device.
- **Incorrect data**: Check that I2C addresses are unique and correctly configured in the Arduino code.

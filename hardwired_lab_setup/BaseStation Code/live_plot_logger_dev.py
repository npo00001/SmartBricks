import serial
import csv
import time
import serial.tools.list_ports
from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import os

# === SETTINGS ===
BAUD_RATE = 9600
MAX_POINTS = 20  # Max number of points to show on the graph at a time (panning)
EXPECTED_SENSORS = [f"Sensor {i}" for i in range(1, 6)]
DATA_FOLDER = "data_logs"  # Folder for storing CSV files

# === FIND SERIAL PORT AUTOMATICALLY ===
def find_serial_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "USB" in port.description:  # You can customize this check for your device
            return port.device
    raise Exception("No suitable serial port found")

# === SETUP SERIAL ===
SERIAL_PORT = find_serial_port()  # Automatically find the serial port
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
print(f"Using serial port: {SERIAL_PORT}")

# === CREATE CSV FOLDER IF IT DOESN'T EXIST ===
if not os.path.exists(DATA_FOLDER):
    os.makedirs(DATA_FOLDER)

# === CREATE UNIQUE CSV FILE BASED ON TIMESTAMP ===
def create_unique_filename():
    timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
    return os.path.join(DATA_FOLDER, f"sensor_data_log_{timestamp}.csv")

csv_filename = create_unique_filename()  # Create a unique filename
print(f"Logging data to: {csv_filename}")

# === CREATE CSV FILE AND WRITE HEADER ===
with open(csv_filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Sensor", "Voltage", "Temperature"])

# === DATA STORAGE ===
sensor_data = {
    sensor: {"voltages": [], "temps": [], "times": []}
    for sensor in EXPECTED_SENSORS
}
buffered_readings = {}

# === PARSE ===
def parse_line(line):
    try:
        sensor_label, data = line.split(":")
        voltage_str, temp_str = data.strip().split()
        return sensor_label.strip(), float(voltage_str), int(temp_str)
    except Exception:
        return None, None, None

# === UPDATE ===
def update(frame):
    global buffered_readings

    # Collect all expected sensors before updating plot
    while ser.in_waiting:
        line = ser.readline().decode().strip()
        if not line.startswith("Sensor"):
            continue

        sensor, voltage, temp = parse_line(line)
        if sensor and voltage is not None and temp is not None:
            timestamp = datetime.now().strftime('%H:%M:%S')
            buffered_readings[sensor] = (timestamp, voltage, temp)

            # Append data to the CSV file
            with open(csv_filename, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([timestamp, sensor, voltage, temp])

        # Once we have data from all sensors, update the plot
        if len(buffered_readings) == len(EXPECTED_SENSORS):
            break

    if len(buffered_readings) < len(EXPECTED_SENSORS):
        return  # Wait until full set is available

    # === Add buffered data to storage ===
    for sensor, (timestamp, voltage, temp) in buffered_readings.items():
        sd = sensor_data[sensor]
        sd["voltages"].append(voltage)
        sd["temps"].append(temp)
        sd["times"].append(timestamp)

        # Keep only the last MAX_POINTS points
        if len(sd["voltages"]) > MAX_POINTS:
            sd["voltages"].pop(0)
            sd["temps"].pop(0)
            sd["times"].pop(0)

    buffered_readings = {}  # Clear buffer

    # === Plotting ===
    ax1.clear()
    ax2.clear()
    ax1.set_title("Live Voltage Readings")
    ax2.set_title("Live Temperature Readings")
    ax2.set_xlabel("Timestamp")
    ax1.set_ylabel("Voltage (V)")
    ax2.set_ylabel("Temperature (°C)")

    for sensor in sensor_data:
        sd = sensor_data[sensor]
        ax1.plot(sd["times"], sd["voltages"], label=sensor)
        ax2.plot(sd["times"], sd["temps"], label=sensor)

    ax1.legend(loc="upper left")
    ax2.legend(loc="upper left")
    ax1.tick_params(axis='x', rotation=45)
    ax2.tick_params(axis='x', rotation=45)

# === PLOT SETUP ===
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 6))
plt.tight_layout()
ani = FuncAnimation(fig, update, interval=1000)

plt.show()

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
MAX_POINTS = 20
EXPECTED_SENSORS = [f"Sensor {i}" for i in range(1, 6)]
DATA_FOLDER = "data_logs"

def find_serial_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "USB" in port.description:
            return port.device
    raise Exception("No suitable serial port found")

SERIAL_PORT = find_serial_port()
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
print(f"Using serial port: {SERIAL_PORT}")

# === FILE SETUP ===
if not os.path.exists(DATA_FOLDER):
    os.makedirs(DATA_FOLDER)

csv_filename = os.path.join(
    DATA_FOLDER, f"temp_log_{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
)
with open(csv_filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Sensor", "Temperature"])

# === DATA STORAGE ===
sensor_data = {
    sensor: {"temps": [], "times": []}
    for sensor in EXPECTED_SENSORS
}
buffered_readings = {}

# === PARSE FUNCTION ===
def parse_line(line):
    try:
        sensor_label, data = line.split(":")
        _, temp_str = data.strip().split()
        return sensor_label.strip(), int(temp_str)
    except Exception:
        return None, None

# === PLOT UPDATE ===
def update(frame):
    global buffered_readings

    while ser.in_waiting:
        line = ser.readline().decode().strip()
        if not line.startswith("Sensor"):
            continue

        sensor, temp = parse_line(line)
        if sensor and temp is not None:
            timestamp = datetime.now().strftime('%H:%M:%S')
            buffered_readings[sensor] = (timestamp, temp)

            with open(csv_filename, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([timestamp, sensor, temp])

        if len(buffered_readings) == len(EXPECTED_SENSORS):
            break

    if len(buffered_readings) < len(EXPECTED_SENSORS):
        return

    for sensor, (timestamp, temp) in buffered_readings.items():
        sd = sensor_data[sensor]
        sd["temps"].append(temp)
        sd["times"].append(timestamp)

        if len(sd["temps"]) > MAX_POINTS:
            sd["temps"].pop(0)
            sd["times"].pop(0)

    buffered_readings = {}

    ax.clear()
    ax.set_title("Live Temperature Readings")
    ax.set_xlabel("Timestamp")
    ax.set_ylabel("Temperature (°C)")

    for sensor in sensor_data:
        sd = sensor_data[sensor]
        ax.plot(sd["times"], sd["temps"], label=sensor)

    ax.legend(loc="upper left")
    ax.tick_params(axis='x', rotation=45)

# === PLOT SETUP ===
fig, ax = plt.subplots(figsize=(12, 6))
plt.tight_layout()
ani = FuncAnimation(fig, update, interval=1000)
plt.show()

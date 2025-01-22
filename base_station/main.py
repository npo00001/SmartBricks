from datetime import datetime # as dt, timedelta
# from math import floor
# from statistics import mean
# from traceback import format_exc
# from re import compile
# from collections import defaultdict

# import matplotlib # Added import to use matplotlib backend
# matplotlib.use('TkAgg') # Set the backend to TkAgg, which uses Tkinter
# from matplotlib.dates import DateFormatter, HourLocator, DayLocator
# import matplotlib.pyplot as plt # Ensure pyplot is imported

from basestationreader import BaseStationReader, ReadResult
from csvwriter import CSVWriter
from logger import Logger, LogLevel
from realtimeplotter import RealTimePlotter
from serialreaderwriter import SerialReaderWriter

def establish_serial_connection(logger):
    srw = SerialReaderWriter()
    available_ports = srw.find_ports()
    logger.info(f"Available ports: {available_ports}")
    
    while True:
        if srw.connect():
            logger.info("Serial connection established.")
            return srw
        else:
            logger.error("Failed to establish serial connection. Retrying...")

def main():
    logger = Logger("logs/errors.log", print_level=LogLevel.INFO, write_level=LogLevel.WARNING)
    srw = establish_serial_connection(logger)

    csv_writers = {}
    logger.info("Program started. Listening for data...")

    try:
        while True:
            try:
                data = srw.read()
                if data:
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

                    try:
                        sensor_id, *sensor_data = data.split(',')
                        logger.info(f"Data received from sensor {sensor_id}: {sensor_data}")

                        if sensor_id not in csv_writers:
                            file_name = f"logs/sensor_{sensor_id}_{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
                            csv_writers[sensor_id] = CSVWriter(file_name)
                            csv_writers[sensor_id].write(["Timestamp"] + [f"Value {i+1}" for i in range(len(sensor_data))])
                            logger.info(f"Created new CSV file for sensor {sensor_id}: {file_name}")

                        csv_writers[sensor_id].write([timestamp] + sensor_data)

                    except ValueError:
                        logger.error(f"Malformed data received: {data}")

            except Exception as e:
                logger.error(f"Unexpected error: {e}")
    except KeyboardInterrupt:
        logger.info("Exiting program...")
    finally:
        for writer in csv_writers.values():
            writer.close()
        srw.disconnect()
        logger.info("All resources released. Program terminated.")

if __name__ == "__main__":
    main()

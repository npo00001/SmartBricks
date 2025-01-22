from datetime import datetime # as dt, timedelta
from datetime import timedelta
# from math import floor
# from statistics import mean
from traceback import format_exc
from re import compile
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

    # Establish Serial Connection
    srw = establish_serial_connection(logger)

    # Initialize CSV Writers and Plotter
    csv_writers = {}
    plotter = RealTimePlotter(
        xstart=datetime.now(),
        xrange=timedelta(minutes=10),
        yrange=(0, 5),
        title="Sensor Data Over Time",
        xlabel="Time",
        ylabel="Voltage",
        x_minor_locator=None,  # Replace with appropriate locator
        x_minor_formatter=None,  # Replace with appropriate formatter
        x_major_locator=None,  # Replace with appropriate locator
        x_major_formatter=None  # Replace with appropriate formatter
    )

    # Regex pattern for data validation
    pattern = compile(r"^(\d+),(\d+(?:\.\d+)?),(\d+),(\d+(?:\.\d+)?),(\d+(?:\.\d+)?)$")

    # BaseStationReader for structured reads
    basestation_reader = BaseStationReader(srw, pattern, timeout=timedelta(seconds=10))

    logger.info("Program started. Listening for data...")

    try:
        while True:
            try:
                sensor_id, voltage, mrange, logvoltage, batvoltage, message, status = basestation_reader.read()

                if status == ReadResult.SUCCESS:
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    logger.info(message)

                    # Write to CSV
                    if sensor_id not in csv_writers:
                        file_name = f"logs/sensor_{sensor_id}_{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
                        csv_writers[sensor_id] = CSVWriter(file_name)
                        csv_writers[sensor_id].write(["Timestamp", "Voltage", "Range", "LogVoltage", "BatteryVoltage"])
                        logger.info(f"Created new CSV file for sensor {sensor_id}: {file_name}")

                    csv_writers[sensor_id].write([timestamp, voltage, mrange, logvoltage, batvoltage])

                    # Plot data
                    plotter.plot(datetime.now(), voltage, label=f"Sensor {sensor_id}")
                    plotter.update(0.1)

                elif status == ReadResult.ERROR:
                    logger.error(message)

                elif status == ReadResult.DEBUG:
                    logger.debug(message)

            except Exception as e:
                logger.error(f"Unexpected error: {e}\n{format_exc()}")

    except KeyboardInterrupt:
        logger.info("Exiting program...")

    finally:
        for writer in csv_writers.values():
            writer.close()
        srw.disconnect()
        logger.info("All resources released. Program terminated.")

if __name__ == "__main__":
    main()

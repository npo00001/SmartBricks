from datetime import datetime as dt, timedelta
from math import floor
from statistics import mean
from traceback import format_exc
from re import compile
from collections import defaultdict

import matplotlib  # Added import to use matplotlib backend
matplotlib.use('TkAgg')  # Set the backend to TkAgg, which uses Tkinter

from matplotlib.dates import DateFormatter, HourLocator, DayLocator
import matplotlib.pyplot as plt  # Ensure pyplot is imported

from BaseStationReader import BaseStationReader, ReadResult
from datatools import SerialReaderWriter, CSVWriter, RealTimePlotter, Logger, LogLevel

start_time = last_plot_time = last_file_change_time = dt.now()
t_str = start_time.strftime("%m-%d-%Y %I:%M:%S %p")  # Formatted time string

FOLDER_PATH = f"./logs/{start_time.strftime('%m-%d-%Y_T%H-%M-%S')}"  # Unique folder name
ERROR_LOG_NAME = FOLDER_PATH + "/errors.log"
DATA_LOG_NAME = FOLDER_PATH + "/day{i}-sensor{sensor}.csv"

PLOT_UPDATE_INTERVAL = timedelta(minutes=1)  # Downsampling interval for plot
DATA_LOG_CHANGE_INTERVAL = timedelta(days=1)  # Interval to rotate data log files

V_LOW, V_HIGH = 0.15, 3.15  # Good voltage range
voltages_to_plot = defaultdict(list)  # Buffer voltages between plot times

DATA_LOG_HEADER = [
    "Time (mm-dd-yyyy H:M:S)", "Time (.1s Delta)", "Wheatstone Voltage (V)",
    "Range", "Log Amp Voltage", "Battery Voltage"
]
file_count = 1

CONSOLE_MSG = "{time} | {message}"  # Format for console messages
DATA_PATTERN = compile(
    r"(\d+)\s+(\d+(?:\.\d+)?)\s+(\d)\s+(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)"
)  # Match int float int float float

p = RealTimePlotter(
    xstart=start_time, xrange=timedelta(hours=24), yrange=[0, 3.3],
    title="Voltage vs. Time", xlabel="Time", ylabel="Voltage (V)",
    solid_lines=(V_LOW, V_HIGH), x_minor_locator=HourLocator(interval=4),
    x_minor_formatter=DateFormatter("%I:%M %p"),
    x_major_locator=DayLocator(interval=1),
    x_major_formatter=DateFormatter("\n%m-%d-%Y")
)

bsr = BaseStationReader(
    SerialReaderWriter("ttyACM0", 9600, timeout=360), DATA_PATTERN,
    timeout=timedelta(seconds=360)
)
writers = {}
logger = Logger(ERROR_LOG_NAME)


def write(sensor, data):
    """Write data to the CSV file associated with the given sensor.
    Create a new CSVWriter with a header if one does not already exist.
    """
    if sensor not in writers:
        writers[sensor] = CSVWriter(DATA_LOG_NAME.format(sensor=sensor, i=file_count))
        writers[sensor].write(DATA_LOG_HEADER)  # Start new CSVWriters with the header

    writers[sensor].write(data)


def cleanup():
    """Close resources and wait for user to exit the program."""
    logger.info("The program has ended. Press ENTER to close the window.")
    bsr.close()
    for writer in writers.values():
        writer.close()
    logger.close()
    input()  # Wait for user to press ENTER to close the window


logger.info(CONSOLE_MSG.format(time=t_str, message="Listening for BaseStation..."))

while True:  # Read data from serial connection continuously
    try:
        now = dt.now()  # Get current time as datetime
        t_str = now.strftime("%m-%d-%Y %I:%M:%S %p")  # Formatted time string
        t_delta = floor((now - start_time).total_seconds() * 10)  # Time delta in tenths of seconds

        # Read data and log appropriately
        sensor_id, voltage, mrange, logvoltage, batvoltage, message, status = bsr.read()
        message = CONSOLE_MSG.format(time=t_str, message=message)
        if status == ReadResult.SUCCESS:
            voltages_to_plot[sensor_id].append(voltage)
            if V_LOW <= voltage <= V_HIGH:
                logger.info(message)
            else:
                logger.warn(message)
            write(sensor_id, [
                t_str, t_delta, "%.5f" % voltage, mrange,
                "%.5f" % logvoltage, "%.5f" % batvoltage
            ])
        else:
            logger.log(message, LogLevel(int(status)))

        # If PLOT_UPDATE_INTERVAL has passed and there are unprocessed measurements, plot the average
        if now - last_plot_time >= PLOT_UPDATE_INTERVAL:
            last_plot_time = now
            for sensor_id, voltages in voltages_to_plot.items():
                if voltages:
                    p.plot(now, mean(voltages), f"sensor{sensor_id}")
                    voltages.clear()

        # If DATA_LOG_CHANGE_INTERVAL has passed, switch to a new file
        if now - last_file_change_time >= DATA_LOG_CHANGE_INTERVAL:
            file_count += 1
            last_file_change_time = now
            for s_id, writer in writers.items():
                writer.set_file(DATA_LOG_NAME.format(sensor=s_id, i=file_count))

        p.update(.3)  # Run plot's GUI loop to keep window responsive

    except KeyboardInterrupt:
        # KeyboardInterrupt is triggered when user presses CTRL+C. This is a shortcut to end running console programs.
        logger.critical(CONSOLE_MSG.format(time=t_str, message="Trapped CTRL+C -- Closing Program"))
        cleanup()
        exit(1)

    except (FileNotFoundError, PermissionError):
        logger.critical(CONSOLE_MSG.format(time=t_str, message="File Access Error"))
        logger.critical(format_exc())

    except Exception:
        logger.critical(CONSOLE_MSG.format(time=t_str, message="Unexpected Exception"))
        logger.critical(format_exc())

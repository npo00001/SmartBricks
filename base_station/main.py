# from datetime import datetime as dt, timedelta
# from math import floor
# from statistics import mean
# from traceback import format_exc
# from re import compile
# from collections import defaultdict

import serial
import csv
from datetime import datetime
import os

# import matplotlib # Added import to use matplotlib backend
# matplotlib.use('TkAgg') # Set the backend to TkAgg, which uses Tkinter

# from matplotlib.dates import DateFormatter, HourLocator, DayLocator
# import matplotlib.pyplot as plt # Ensure pyplot is imported

# from BaseStationReader import BaseStationReader, ReadResult
# from datatools import SerialReaderWriter
# from datatools import CSVWriter
# from datatools import RealTimePlotter
# from datatools import Logger, LogLevel

def get_csv_writer():
    """Create a CSV writer with a timestamp-based filename."""
    try:
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        file_path = f"{timestamp}.csv"
        file = open(file_path, 'w', newline='')  # Open a new CSV file for writing
        writer = csv.writer(file)
        writer.writerow(["Timestamp", "Data"])  # Write CSV header
        return file, writer
    except Exception as e:
        print(f"Error creating CSV file: {e}")
        raise

def main():
    try:
        # Initialize serial connection
        ser = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=1)
        print("Serial connection established.")
        
        # Get a CSV writer
        csv_file, csv_writer = get_csv_writer()
        print("CSV file created.")
        
        while True:
            try:
                # Read data from serial port
                data = ser.readline().decode('utf-8').strip()
                if data:  # If data is not empty
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    print(f"{timestamp}: {data}")
                    
                    # Write data to CSV
                    csv_writer.writerow([timestamp, data])
                    csv_file.flush()  # Ensure data is written to disk
            except UnicodeDecodeError as e:
                print(f"Error decoding data: {e}")
            except Exception as e:
                print(f"Unexpected error during data reading: {e}")
                break

    except serial.SerialException as e:
        print(f"Error with serial connection: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        # Clean up resources
        if 'csv_file' in locals() and not csv_file.closed:
            csv_file.close()
            print("CSV file closed.")
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed.")
        print("Resources released.")

if __name__ == "__main__":
    main()

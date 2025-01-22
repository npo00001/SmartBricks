import enum
from datetime import datetime as dt
from datetime import timedelta
from re import Pattern
from typing import Optional

from serial import SerialException

from datatools import SerialReaderWriter


class ReadResult(enum.IntEnum):
    """Status of read operation. To be returned so the caller can quickly determine the success/failure of the read."""

    DEBUG = 1  # debugging information
    INFO = 2  # informational message
    SUCCESS = 3  # valid data read
    ERROR = 4  # error message
    CRITICAL = 5  # unknown message


class BaseStationReader:
    def __init__(self, reader: SerialReaderWriter, pattern: Pattern, timeout: timedelta):
        """
        Args:
            reader: SerialReader to read data from. Will try to reconnect on read if disconnected.
            pattern: regex Pattern used to match data. Group 1: sensor_id, Group 2: voltage, Group 3: range
            timeout: how long to wait for a non-responsive SerialReader before disconnecting
        """
        self.reader = reader
        self.pattern = pattern
        self.timeout = timeout
        self.last_read_time = dt.now()

    def read(self) -> (Optional[int], Optional[float], Optional[int], Optional[float], Optional[float], str, ReadResult):
        """
        Tries to read voltage and range from the provided SerialReaderWriter according to the provided pattern.

        Returns:
            (int | None, float | None, int | None, str, ReadResult):
                int: the sensor id (or None). <br>
                float: the voltage (or None). <br>
                int: the range (or None). <br>
                str: a message summarizing what happened during the read. <br>
                ReadResult: the status of the read.
        """

        current_time = dt.now()
        sensor_id, voltage, mrange, logvoltage, batvoltage, message, status = None, None, None, None, None, "", ReadResult.ERROR

        if not self.reader.is_connected():

            if self.reader.connect():
                message = "BaseStation connected."
                status = ReadResult.INFO
                self.last_read_time = current_time
                return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

            message = "BaseStation not connecting."
            status = ReadResult.DEBUG
            return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

        try:
            if self.reader.has_waiting():
                self.last_read_time = current_time
                data = self.reader.read()
                match = self.pattern.fullmatch(data)

                if match:
                    sensor_id = int(match.group(1))  # convert sensor id to int
                    voltage = float(match.group(2))  # convert voltage to float
                    mrange = int(match.group(3))  # convert range to int
                    logvoltage = float(match.group(4))	#convert logvoltage to float
                    batvoltage = float(match.group(5))	#convert batvoltage to float
                    message = f"sensor: {sensor_id}, wheatstone voltage: {voltage: .5f}, range: {mrange}, log amp voltage: {logvoltage: .5f}, battery voltage: {batvoltage: .5f}"
                    status = ReadResult.SUCCESS
                    return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

                # determine severity of message received
                debug = any([
                    data.startswith("Timeout connecting to device"),
                    data.startswith("Couldn't connect to device"),
                    data.startswith("Couldn't discover attributes of device"),
                    data.startswith("Couldn't read data from device")
                ])

                error = data.startswith("Restarting BaseStation")

                if debug:
                    status = ReadResult.DEBUG
                elif error:
                    status = ReadResult.ERROR
                else:
                    status = ReadResult.CRITICAL

                message = f"Received '{data}'."
                return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

            else:
                if current_time - self.last_read_time >= self.timeout:
                    message = "BaseStation timed out. Disconnecting."
                    status = ReadResult.ERROR
                    self.last_read_time = current_time
                    self.reader.disconnect()
                    return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

                message = "No data was waiting on serial buffer."
                status = ReadResult.DEBUG
                return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

        except SerialException:
            message = "BaseStation connection lost."
            status = ReadResult.ERROR
            self.reader.disconnect()  # release serial port
            return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

        except UnicodeDecodeError:
            message = "Data could not be decoded properly"
            status = ReadResult.ERROR
            return sensor_id, voltage, mrange, logvoltage, batvoltage, message, status

    def close(self):
        self.reader.disconnect()

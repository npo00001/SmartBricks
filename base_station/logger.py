import enum
from os import makedirs, path, fsync
import logging
from logging.handlers import RotatingFileHandler
from datetime import datetime

class LogLevel(enum.IntEnum):
    """Used to tell the Logger how to handle a message."""
    DEBUG = 1
    INFO = PRINT_DEFAULT = 2
    WARNING = 3
    ERROR = WRITE_DEFAULT = 4
    CRITICAL = 5

class Logger:
    def __init__(self, file_path, print_level=LogLevel.PRINT_DEFAULT, write_level=LogLevel.WRITE_DEFAULT):
        self.print_level = print_level  # messages with a level >= to this will be printed
        self.write_level = write_level  # messages with a level >= to this will be written to the log file

        # Create log directory if it doesn't exist
        makedirs(path.dirname(file_path), exist_ok=True)

        # Set up the logging
        self.logger = logging.getLogger("SmartLogger")
        self.logger.setLevel(logging.DEBUG)

        # File handler with rotation
        file_handler = RotatingFileHandler(file_path, maxBytes=5*1024*1024, backupCount=5)
        file_formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        file_handler.setFormatter(file_formatter)
        self.logger.addHandler(file_handler)

        # Console handler
        console_handler = logging.StreamHandler()
        console_formatter = logging.Formatter('%(levelname)s: %(message)s')
        console_handler.setFormatter(console_formatter)
        self.logger.addHandler(console_handler)

    def log(self, msg, log_level):
        """Log a message with the appropriate severity."""
        if log_level >= self.print_level:
            if log_level == LogLevel.DEBUG:
                self.logger.debug(msg)
            elif log_level == LogLevel.INFO:
                self.logger.info(msg)
            elif log_level == LogLevel.WARNING:
                self.logger.warning(msg)
            elif log_level == LogLevel.ERROR:
                self.logger.error(msg)
            elif log_level == LogLevel.CRITICAL:
                self.logger.critical(msg)

    # Convenience methods
    def debug(self, msg):
        self.log(msg, LogLevel.DEBUG)

    def info(self, msg):
        self.log(msg, LogLevel.INFO)

    def warn(self, msg):
        self.log(msg, LogLevel.WARNING)

    def error(self, msg):
        self.log(msg, LogLevel.ERROR)

    def critical(self, msg):
        self.log(msg, LogLevel.CRITICAL)

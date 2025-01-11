import enum
from os import makedirs, path, fsync


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

        makedirs(path.dirname(file_path), exist_ok=True)
        self.file = open(file_path, "w")

    def close(self):
        self.file.close()

    def log(self, msg, log_level):
        prefix = "%-9s" % f"{log_level.name}:"
        text = f"{prefix} {msg}"

        if log_level >= self.print_level:
            print(text)

        if log_level >= self.write_level:
            self.file.write(text + "\n")

        self.file.flush()
        fsync(self.file.fileno())

    # convenience functions
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

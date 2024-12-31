import serial


class SerialReaderWriter:
    def __init__(self, port, baudrate, timeout=60):
        self.com = port
        self.baud = baudrate
        self.timeout = timeout
        self.ser = None

    def is_connected(self):
        # serial object exists and is open
        return self.ser and self.ser.isOpen()

    def connect(self):
        """Try to open serial connection. Returns True if successful, False if not."""

        # check if serial object exits, try to create it if it doesn't
        if self.ser is None:
            try:
                self.ser = serial.Serial(port=self.com, baudrate=self.baud, timeout=self.timeout)
                return True
            except serial.SerialException:
                return False

        # serial object exists, check if it is open
        if self.ser.isOpen():
            return True

        # serial object exists, but is closed. open it
        try:
            self.ser.open()
            return True
        except serial.SerialException:
            return False

    def disconnect(self):
        if self.ser:
            self.ser.close()

    def read(self):
        """
        Return decoded line from serial connection without newline ending.
        May raise serial.SerialException if connection is lost,
        UnicodeDecodeError if bytes could not be decoded properly,
        AttributeError if self.ser is not initialized properly.
        """
        return self.ser.readline().decode('utf-8')[:-2]

    def has_waiting(self):
        return self.ser and self.ser.inWaiting()

    def write(self, msg):
        self.ser.write(str(msg).encode('utf-8'))

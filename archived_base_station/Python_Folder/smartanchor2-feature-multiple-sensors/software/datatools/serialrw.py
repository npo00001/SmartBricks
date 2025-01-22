import serial
import serial.tools.list_ports


class SerialReaderWriter:
    def __init__(self, port='ttyACM0', baudrate=9600, timeout=60):
        self.com = port
        self.baud = baudrate
        self.timeout = timeout
        self.ser = None

    def is_connected(self):
        """Check if the serial object exists and is open."""
        return self.ser and self.ser.isOpen()

    def find_port(self):
        """Attempt to find the specified port or an available alternative."""
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if self.com in ports:
            return self.com
        elif ports:
            print(f"Port {self.com} not found. Available ports: {ports}")
            print(f"Attempting to use port {ports[0]} instead.")
            return ports[0]
        else:
            print("No available serial ports found.")
            return None

    def connect(self):
        """Try to open serial connection. Returns True if successful, False if not."""
        if self.ser is None:
            self.com = self.find_port()
            if not self.com:
                return False
            try:
                self.ser = serial.Serial(port=self.com, baudrate=self.baud, timeout=self.timeout)
                return True
            except serial.SerialException as e:
                print(f"Failed to open port {self.com}: {e}")
                return False

        if self.ser.isOpen():
            return True

        try:
            self.ser.open()
            return True
        except serial.SerialException as e:
            print(f"Failed to reopen port {self.com}: {e}")
            return False

    def disconnect(self):
        """Close the serial connection if it exists."""
        if self.ser:
            self.ser.close()

    def read(self):
        """
        Return a decoded line from the serial connection without the newline ending.
        May raise serial.SerialException, UnicodeDecodeError, or AttributeError.
        """
        return self.ser.readline().decode('utf-8')[:-2]

    def has_waiting(self):
        """Check if there is data waiting in the serial buffer."""
        return self.ser and self.ser.inWaiting()

    def write(self, msg):
        """Write a message to the serial connection."""
        self.ser.write(str(msg).encode('utf-8'))

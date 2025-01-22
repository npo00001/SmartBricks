import serial
from serial.tools import list_ports # import serial.tools.list_ports

class SerialReaderWriter:
    def __init__(self, port='/dev/ttyACM0', baudrate=9600, timeout=1): # Archived code uses "port='ttyACM0'"
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    # def is_connected(self): is located here
    
    # def find_port(self): is located here

    def connect(self):
        # """Try to open serial connection. Returns True if successful, False if not."""
        # if self.ser is None:
        #     self.com = self.find_port()
        #     if not self.com:
        #         return False
        #     try:
        #         self.ser = serial.Serial(port=self.com, baudrate=self.baud, timeout=self.timeout)
        #         return True
        #     except serial.SerialException as e:
        #         print(f"Failed to open port {self.com}: {e}")
        #         return False

        # if self.ser.isOpen():
        #     return True

        # try:
        #     self.ser.open()
        #     return True
        # except serial.SerialException as e:
        #     print(f"Failed to reopen port {self.com}: {e}")
        #     return False

        try:
            self.ser = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=self.timeout)
            return True
        except serial.SerialException as e:
            print(f"Failed to open port {self.port}: {e}")
            return False

    def disconnect(self):
        # """Close the serial connection if it exists."""
        # if self.ser:
        #     self.ser.close()

        if self.ser:
            self.ser.close()
            self.ser = None

    def read(self):
        # """
        # Return a decoded line from the serial connection without the newline ending.
        # May raise serial.SerialException, UnicodeDecodeError, or AttributeError.
        # """
        # return self.ser.readline().decode('utf-8')[:-2]
    
        if not self.ser or not self.ser.is_open:
            raise serial.SerialException("Serial connection is not open.")
        return self.ser.readline().decode('utf-8').strip()

    # def has_waiting(self):
    #     """Check if there is data waiting in the serial buffer."""
    #     return self.ser and self.ser.inWaiting()
    
    def write(self, data):
    # def write(self, msg):
    #     """Write a message to the serial connection."""
    #     self.ser.write(str(msg).encode('utf-8'))

        if not self.ser or not self.ser.is_open:
            raise serial.SerialException("Serial connection is not open.")
        self.ser.write(data.encode('utf-8'))

    def find_ports(self):
        # """Attempt to find the specified port or an available alternative."""
        # ports = [p.device for p in serial.tools.list_ports.comports()]
        # if self.com in ports:
        #     return self.com
        # elif ports:
        #     print(f"Port {self.com} not found. Available ports: {ports}")
        #     print(f"Attempting to use port {ports[0]} instead.")
        #     return ports[0]
        # else:
        #     print("No available serial ports found.")
        #     return None

        return [port.device for port in list_ports.comports()]

    def is_connected(self):
        # """Check if the serial object exists and is open."""
        # return self.ser and self.ser.isOpen()

        return self.ser is not None and self.ser.is_open

# All of the below is new
if __name__ == "__main__":
    srw = SerialReaderWriter()
    print("Available ports:", srw.find_ports())

    if srw.connect():
        print("Connection established.")

        try:
            while True:
                data = srw.read()
                if data:
                    print("Received:", data)
        except KeyboardInterrupt:
            print("Exiting...")
        except Exception as e:
            print("Error:", e)
        finally:
            srw.disconnect()
            print("Connection closed.")
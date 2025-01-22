import serial
from serial.tools import list_ports

class SerialReaderWriter:
    def __init__(self, port='/dev/ttyACM0', baudrate=9600, timeout=1):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    def connect(self):
        try:
            self.ser = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=self.timeout)
            return True
        except serial.SerialException as e:
            print(f"Failed to open port {self.port}: {e}")
            return False

    def disconnect(self):
        if self.ser:
            self.ser.close()
            self.ser = None

    def read(self):
        if not self.ser or not self.ser.is_open:
            raise serial.SerialException("Serial connection is not open.")
        return self.ser.readline().decode('utf-8').strip()

    def write(self, data):
        if not self.ser or not self.ser.is_open:
            raise serial.SerialException("Serial connection is not open.")
        self.ser.write(data.encode('utf-8'))

    def find_ports(self):
        return [port.device for port in list_ports.comports()]

    def is_connected(self):
        return self.ser is not None and self.ser.is_open

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
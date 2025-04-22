import serial
import logging

logger = logging.getLogger(__name__)

class SerialCommunicator:
    def __init__(self, port, baudrate, timeout):
        try:
            self.serialPort = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
        except serial.SerialException as e:
            logger.error(f"Failed to open serial port: {e}")
            raise

    def read_line(self):
        try:
            return self.serialPort.readline().decode("utf-8")
        except serial.SerialException as e:
            logger.error(f"Failed to read from serial port: {e}")
            return None

    def write_command(self, command):
        try:
            self.serialPort.write(command.encode("utf-8"))
        except serial.SerialException as e:
            logger.error(f"Failed to write to serial port: {e}")
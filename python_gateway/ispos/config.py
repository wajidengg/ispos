from datetime import datetime, timezone
import os
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Configuration parameters
SERIAL_PORT = os.getenv("SERIAL_PORT", "COM3")
BAUDRATE = int(os.getenv("BAUDRATE", 115200))
TIMEOUT = int(os.getenv("TIMEOUT", 1))
IMAGE_URL = os.getenv("IMAGE_URL", "http://10.0.0.49/capture")
USE_PVLIB = os.getenv("USE_PVLIB", "false").lower() == "false"
LATITUDE = float(os.getenv("LATITUDE", "43.745418"))
LONGITUDE = float(os.getenv("LONGITUDE", "-79.211029"))
TIMESTAMP = os.getenv("TIMESTAMP", datetime.now(timezone.utc).isoformat())
MQTT_HOST = os.getenv("MQTT_HOST", "3.84.218.133")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
MQTT_TOKEN = os.getenv("MQTT_TOKEN", "8KNJFV3IKDt8fKHqJIoe")
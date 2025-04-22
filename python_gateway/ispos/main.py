import time
import logging
from ispos.utils.image_processing import analyze_brightness
from ispos.utils.serial_communication import SerialCommunicator
from ispos.utils.sun_position import get_sun_position
from ispos.utils.weather_data import WeatherDataClient
from ispos.utils.thingsboard_mqtt import ThingsBoardClient
from ispos.config import (
    SERIAL_PORT,
    BAUDRATE,
    TIMEOUT,
    IMAGE_URL,
    USE_PVLIB,
    LATITUDE,
    LONGITUDE,
    TIMESTAMP,
    MQTT_HOST,
    MQTT_PORT,
    MQTT_TOKEN
)

logging.basicConfig(level=logging.INFO)

def main():
    serial_comm = SerialCommunicator(port=SERIAL_PORT, baudrate=BAUDRATE, timeout=TIMEOUT)
    weather_client = WeatherDataClient(LATITUDE, LONGITUDE)
    thingsboard_client = ThingsBoardClient(MQTT_HOST, MQTT_PORT, MQTT_TOKEN)

    # Initial position
    serial_comm.write_command("AT+SEND=10,7,BTH_OFF\r\n")

    while True:
        line = serial_comm.read_line()
        if line:
            logging.info(line)

        # Fetch live weather data
        weather_data = weather_client.fetch_current_weather()
        logging.info(f"Current Weather: {weather_data}")

        # Analyze brightness
        brightest_part = analyze_brightness(IMAGE_URL)

        # Fetch sun position
        horizon_status, azimuth_status, altitude, azimuth = None, None, None, None
        horizon_status, azimuth_status, altitude, azimuth = get_sun_position(LATITUDE, LONGITUDE, TIMESTAMP)
        logging.info(f"Sun is {horizon_status} and towards the {azimuth_status}")
        logging.info(f"Sun altitude: {altitude}, azimuth: {azimuth}")

        command = None

        # Decision logic based on brightness and sun position
        if USE_PVLIB and horizon_status == "above horizon":
            if azimuth_status == "east":
                if brightest_part == "Middle":
                    command = "AT+SEND=10,7,BTH_LED\r\n"
                    logging.info(f"Sun position is east, but more brightness is in the middle")
                else:
                    command = "AT+SEND=10,7,YEL_LED\r\n"
            elif azimuth_status == "middle":
                command = "AT+SEND=10,7,BTH_LED\r\n"
            elif azimuth_status == "west":
                if brightest_part == "Middle":
                    command = "AT+SEND=10,7,BTH_LED\r\n"
                    logging.info(f"Sun position is west, but more brightness is in the middle")
                else:
                    command = "AT+SEND=10,7,RED_LED\r\n"
        else:
            if brightest_part == "Top":
                command = "AT+SEND=10,7,RED_LED\r\n"
                panel_position = "270"
            elif brightest_part == "Middle":
                command = "AT+SEND=10,7,BTH_LED\r\n"
                panel_position = "0"
            elif brightest_part == "Bottom":
                command = "AT+SEND=10,7,YEL_LED\r\n"
                panel_position = "90"
            logging.info(f"Panel position is {panel_position}")

        # Send command to serial port
        if command:
            serial_comm.write_command(command)

        # Send telemetry to ThingsBoard
        telemetry_data = {
            "temperature": weather_data["temperature"],
            "cloud_cover": weather_data["cloud_cover"],
            "brightness_section": panel_position,
            "sun_azimuth_status": azimuth_status,
            "sun_horizon_status": horizon_status,
            "sun_altitude": altitude,
            "sun_azimuth": azimuth
        }
        thingsboard_client.send_telemetry(telemetry_data)

        time.sleep(10) # (3600) Sleep for 1 hour before the next iteration

if __name__ == "__main__":
    main()
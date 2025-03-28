import serial
import time
import re
import random
import cv2
import numpy as np
import requests
from io import BytesIO
from PIL import Image

# Serial configuration
serialPort = serial.Serial(port="COM3", baudrate=115200, timeout=1)

# Function to calculate brightness
def calculate_brightness(image):
    # Convert image to grayscale (luminance)
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    # Calculate the average brightness (mean pixel value)
    return np.mean(gray_image)

# Function to analyze which part of the image is the brightest
def analyze_brightness(image_url):
    # Download the image from the URL
    response = requests.get(image_url)
    image = Image.open(BytesIO(response.content))
    # Convert the PIL image to a numpy array (OpenCV format)
    image = np.array(image)
    
    # Get the height and width of the image
    height, width, _ = image.shape
    
    # Divide the image into top, middle, and bottom sections
    top_section = image[:height // 3, :]
    middle_section = image[height // 3:2 * height // 3, :]
    bottom_section = image[2 * height // 3:, :]
    
    # Calculate the brightness of each section
    top_brightness = calculate_brightness(top_section)
    middle_brightness = calculate_brightness(middle_section)
    bottom_brightness = calculate_brightness(bottom_section)
    
    # Compare the brightness levels and return the result
    if top_brightness > middle_brightness and top_brightness > bottom_brightness:
        return "Top"
    elif middle_brightness > top_brightness and middle_brightness > bottom_brightness:
        return "Middle"
    else:
        return "Bottom"

# URL of the image from the ESP32 Camera (Change the IP address accordingly)
image_url = "http://10.0.0.49/capture"

while True:
    line = serialPort.readline().decode("utf-8")
    print(line)
    # Analyze the brightness and print the result
    brightest_part = analyze_brightness(image_url)
    
    if brightest_part == "Top":
        command = "AT+SEND=10,7,RED_LED\r\n"
        serialPort.write(command.encode("utf-8"))
        
    if brightest_part == "Bottom":
        command = "AT+SEND=10,7,YEL_LED\r\n"
        serialPort.write(command.encode("utf-8"))
        
    if brightest_part == "Middle":
        command = "AT+SEND=10,7,BTH_LED\r\n"
        serialPort.write(command.encode("utf-8"))
        
    #command = "AT+SEND=10,7,RED_LED\r\n"
    #serialPort.write(command.encode("utf-8"))
    time.sleep(2)
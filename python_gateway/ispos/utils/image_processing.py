import cv2
import numpy as np
import requests
from io import BytesIO
from PIL import Image
import logging

logger = logging.getLogger(__name__)

def calculate_brightness(image):
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    return np.mean(gray_image)

def analyze_brightness(image_url):
    try:
        response = requests.get(image_url)
        response.raise_for_status()
        image = Image.open(BytesIO(response.content))
        image = np.array(image)
        
        height, width, _ = image.shape
        top_section = image[:height // 3, :]
        middle_section = image[height // 3:2 * height // 3, :]
        bottom_section = image[2 * height // 3:, :]
        
        top_brightness = calculate_brightness(top_section)
        middle_brightness = calculate_brightness(middle_section)
        bottom_brightness = calculate_brightness(bottom_section)
        
        if top_brightness > middle_brightness and top_brightness > bottom_brightness:
            return "Top"
        elif middle_brightness > top_brightness and middle_brightness > bottom_brightness:
            return "Middle"
        else:
            return "Bottom"
    except requests.RequestException as e:
        logger.error(f"Failed to download image: {e}")
        return None
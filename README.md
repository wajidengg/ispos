# Intelligent Solar Panel Orientation System

**Author:** Wajid Hussain  
**Master’s Project in Cyber-Physical Systems**  
**Advisors:** Dr. Rolando Herrero, Dr. Haitham Tayyar  

## Introduction
This project aims to enhance solar panel efficiency by developing an **Intelligent Solar Panel Orientation System** using **ESP32-CAM**, **LoRa communication**, and **IoT-based data processing**. By integrating **image analysis**, **sun-tracking algorithms**, and **long-range communication**, the system dynamically adjusts solar panel orientation for optimal energy efficiency.

## System Architecture

### Key Components:
- **ESP32-CAM:** Captures images using Wi-Fi to detect the sun's position.
- **LoRa Modules:** Facilitate long-range communication between the ESP32 and a laptop.
- **Solar Tracking Mechanism:** Adjusts the panel orientation based on sun position data.

### Software:
- **Sun Position Calculation:** Uses libraries like **Skyfield**, **Ephem**, **PySolar**, and **PVlib** for accurate sun positioning.
- **LoRa Communication:** Sends activation commands from the laptop to the ESP32 to adjust the solar panel.
- **Cloud Integration:** Sends data via **MQTT** for real-time monitoring and visualization.

## System Workflow
1. **Image Capture:** ESP32-CAM captures images over Wi-Fi every 30 minutes.
2. **Image Processing:** The laptop verifies the sun's position.
3. **LoRa Transmission:** Activation commands are sent to the ESP32 to adjust the panel.
4. **Cloud Monitoring:** Data is sent to a cloud platform for real-time updates.

## Conclusion
This project integrates IoT, image processing, and sun-tracking algorithms to create an intelligent solar panel system. The system offers real-time optimization and remote monitoring, providing a scalable solution for renewable energy applications.

## Installation & Setup
1. **ESP32-CAM Setup:** Configure with Wi-Fi for image capture.
2. **LoRa Setup:** Enable communication between ESP32 and laptop.
3. **Laptop:** Process image data and calculate sun position.
4. **Solar Tracking:** Adjust panel orientation based on sun data.

### Running the System:
- Power the ESP32-CAM and LoRa modules.
- Use the laptop to monitor and control the system.

# Baby_sensor
Arduino Labratory Final project, An home made baby sensor, using esp32, esp32 cam more.

![Baby Crying](https://media.giphy.com/media/2Qs2hKWMvEzdu/giphy.gif)

# Overview
This project involves creating a baby sensor system using Arduino, various sensors, and an ESP32 microcontroller. The sensor system monitors the baby’s environment and alerts caregivers if certain thresholds are exceeded, ensuring the baby’s safety and comfort.

## Features
- Environmental Monitoring: Monitors temperature and humidity around the baby's bed to ensure a comfortable environment. Alerts caregivers if conditions deviate from safe ranges.
- Video Streaming: Streams live video from the baby's room using the ESP32 CAM, allowing real-time monitoring of the baby's activities.
- Movement Detection: Captures images and detects movement within the baby's bed area using the ESP32 CAM, providing additional visibility and monitoring capabilities.
- Real-Time Alerts: Provides visual alerts through LED lights if critical conditions are detected, such as high temperature or unusual movement.
- Data Visualization: Logs and visualizes environmental data using ThingSpeak, offering a dashboard for tracking and analyzing the baby's environment over time.

## Components
- ESP32 (3 units): Microcontrollers used for processing and communication. They handle data from sensors and manage interactions between components.
- ESP32 CAM: A compact development board with an integrated camera and SD card slot. The camera is mounted directly on top of the ESP32 board, eliminating the need for additional wiring. It’s used for capturing pictures and streaming video.
- Humidity and Temperature Sensor (SHT31): Measures ambient temperature and humidity, providing real-time environmental data to ensure the baby’s comfort.
- Gyroscope (MPU6050): Detects orientation and motion, helping to monitor the baby's movements and detect any unusual activity.
- LED Screen: Displays real-time data, alerts, or status messages. Useful for visual feedback and monitoring system status at a glance.
- Red LED Light: Used as a visual alert for critical warnings.
- Green LED Light: Indicates normal operation or safe conditions. Provides a visual confirmation of system status.
- Batteries (2 units): Power source for two of the ESP32 units, ensuring their operation and portability.
- 330 Ohm Resistors (2 units): Used to limit current and protect components, such as LEDs, from excessive current.

## Software and Services
- Arduino IDE: Software development environment used for writing and uploading code to the ESP32 boards. It includes libraries and tools for interfacing with hardware components.
- ThingSpeak: A cloud-based platform for IoT data logging and visualization. Used to collect and display data from your sensors. Set up an account and configure your channel to integrate with your project.

## Instructions
### Setup

#### Connecting ESP32 to Sensors
- ESP32 Unit 1 (Bear):
Humidity and Temperature Sensor (SHT31):
Wiring Instructions:
Connect the sensor’s VCC to ESP32 3V3.
Connect the sensor’s GND to ESP32 GND.
Connect the sensor’s SDA to ESP32 GPIO 21.
Connect the sensor’s SCL to ESP32 GPIO 22.

-ESP32 Unit 2 (Mattress):
Gyroscope (MPU6050):
Wiring Instructions:
Connect the gyroscope’s VCC to ESP32 3V3.
Connect the gyroscope’s GND to ESP32 GND.
Connect the gyroscope’s SDA to ESP32 GPIO 21.
Connect the gyroscope’s SCL to ESP32 GPIO 22.
ESP32 Unit 3 (Parents):
LED Screen (OLED), red and green LED lights: 
Wiring Instructions:
Connect the OLED’s VCC to ESP32 3V3.
Connect the OLED’s GND to ESP32 GND.
Connect the OLED’s SCK to ESP32 SCK.
Connect the OLED’s SDA to ESP32 SDA.
connect Red LED's short leg to A4.**?**
connect green LED's short leg to D10.**?**
connect each LED's long leg to a 330 ohm resistor. 
connect both leg to ESP32 GND.**?**

Detailed Diagrams:
For detailed diagrams images, visit the Images Repository.  **Add link**

**--------------------------------------------got here**
Install the required libraries for Arduino and ESP32 from the Arduino Library Manager.
Example: Adafruit DHT sensor library for temperature and humidity sensors. 
Upload Code

Open the baby_sensor.ino file in the Arduino IDE.
Select the appropriate board and port.
Upload the code to the Arduino and ESP32.
Configuration
Sensor Calibration

Follow the calibration instructions specific to each sensor.
Thingspeak Configuration

Create an account on Thingspeak.
Set up a channel for data logging.
Update the API keys and channel ID in the config.h file.
Usage
Power on the system and ensure all sensors are properly connected.
Monitor the baby’s environment through the Thingspeak dashboard.
Alerts will be triggered and sent if parameters exceed the predefined thresholds.
Troubleshooting
Sensor not responding: Check connections and calibration.
No data on Thingspeak: Verify API keys and network connection.
Contributing
Feel free to fork the repository and submit pull requests. Any improvements or bug fixes are welcome!

License
This project is licensed under the MIT License - see the LICENSE file for details.

Acknowledgements
Arduino - Open-source electronics platform.
ESP32 - Low-power microcontroller.
Thingspeak - IoT platform for data logging and visualization.

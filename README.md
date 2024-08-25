# Baby_sensor
Arduino Labratory Final project, An home made baby sensor, using esp32, esp32 cam more.

![Baby Crying](https://media.giphy.com/media/2Qs2hKWMvEzdu/giphy.gif)

# Overview
This project involves creating a baby sensor system using Arduino, various sensors, and an ESP32 microcontroller. The sensor system monitors the baby’s environment and alerts caregivers if certain thresholds are exceeded, ensuring the baby’s safety and comfort.

## Features
- Environmental Monitoring: Monitors temperature and humidity around the baby's bed to ensure a comfortable environment.
- Video Streaming: Streams live video from the baby's room using the ESP32 CAM, allowing real-time monitoring of the baby's activities.
- Movement Detection: detects baby's movement using gyroscope providing additional monitoring capabilities.
- Data Visualization: Logs and visualizes environmental data using ThingSpeak, offering a dashboard for tracking and analyzing the baby's environment over time.

## Components
- ESP32 (3 units): Microcontrollers used for processing and communication. They handle data from sensors and manage interactions between components.
- ESP32 CAM: A compact development board with an integrated camera and SD card slot. The camera is mounted directly on top of the ESP32 board, eliminating the need for additional wiring. It’s used for streaming video.
- Humidity and Temperature Sensor (SHT31): Measures ambient temperature and humidity, providing real-time environmental data to ensure the baby’s comfort.
- Gyroscope (MPU6050): Detects orientation and motion, helping to monitor the baby's movements and detect any unusual activity.
- LED Screen: Displays real-time data and status messages. Useful for visual feedback and monitoring system status at a glance.
- Green LED Light: Indicates movements in the baby's bed.
- Batteries (2 units): Power source for two of the ESP32 units, ensuring their operation and portability.
- 330 Ohm Resistor: Used to limit current and protect components, such as LEDs, from excessive current.

## Software and Services
- Arduino IDE: Software development environment used for writing and uploading code to the ESP32 boards. It includes libraries and tools for interfacing with hardware components.
- ThingSpeak: A cloud-based platform for data logging and visualization. Used to collect and display data from your sensors. Set up an account and configure your channel to integrate with your project.

## Instructions
### Setup

#### Adjust ThingSpeak
1. Create an account on Thingspeak.
2. Set up a channel for data logging.
3. Set up 3 fields in the channel:
   - 1, for temperature
   - 2, for humidity
   - 3, for movement 
5. Note yourself the API keys and channel ID.

#### Find ESP32 MAC Addresses
1. One by one connect each unit to the computer.
2. Open the Arduino IDE.
3. Select the appropriate board (FreeBeetle-ESP32) from the Tools > Board menu.
4. Select the correct port from the Tools > Port menu.
5. Open the sketch file named MAC_Address.ino.
6. copy and paste in Arduino IDE.
7. Click the Upload button to upload the code to the ESP32.
8. open the Serial Monitor from the Tools > Serial Monitor menu.
9. Press the RST button on the ESP32 to reset the board.
10. Copy the MAC address displayed in the Serial Monitor and Save it for later.

#### Connecting ESP32 to Sensors
- ESP32 Unit 1 (Bear):
Humidity and Temperature Sensor (SHT31):
Wiring Instructions:
1. Connect the sensor’s VCC to ESP32 3V3.
2. Connect the sensor’s GND to ESP32 GND.
3. Connect the sensor’s SDA to ESP32 GPIO 21.
4. Connect the sensor’s SCL to ESP32 GPIO 22.

- ESP32 Unit 2 (Mattress):
Gyroscope (MPU6050):
Wiring Instructions:
1. Connect the gyroscope’s VCC to ESP32 3V3.
2. Connect the gyroscope’s GND to ESP32 GND.
3. Connect the gyroscope’s SDA to ESP32 GPIO 21.
4. Connect the gyroscope’s SCL to ESP32 GPIO 22.

- ESP32 Unit 3 (Parents):
LED Screen (OLED) and green LED lights: 
Wiring Instructions:
1. Connect the OLED’s VCC to ESP32 3V3.
2. Connect the OLED’s GND to ESP32 GND.
3. Connect the OLED’s SCK to ESP32 SCK.
4. Connect the OLED’s SDA to ESP32 SDA.
5. connect green LED's long leg to A4.
6. connect green LED's short leg to a 330 ohm resistor. 
7. connect the second leg of the resistor to ESP32 GND.

Detailed Diagrams:
For detailed diagrams images, visit the [Images Repository](Pictures/Circuit_diagrams_Pictures).

#### Uploading Code to ESP32 Units
- **Important note** Library Installation: If the Arduino IDE alerts you that you are missing libraries while uploading code, follow these steps to install them:
1. Go to Sketch > Include Library > Manage Libraries in the Arduino IDE.
2. In the Library Manager, use the search bar to find the required library by name.
3. Click the Install button next to the library name to download and install it.
4. Repeat this process for any additional libraries required by your sketches.

- Ensure that you have the Arduino IDE set up and configured for ESP32 development. Follow these steps to upload the correct code to each ESP32 unit:
1. Open the Arduino IDE.
2. Connect the relevant unit to the computer.
3. Select the appropriate board (FreeBeetle-ESP32) from the Tools > Board menu.
4. Select the correct port from the Tools > Port menu.

- ESP32 Unit 1 (Bear):
Code Purpose: Handles the humidity and temperature sensor (SHT31).
Upload Instructions:
1. Open the sketch file named Bear.ino.
2. Copy and paste in Arduino IDE.
3. Change MAC Address to Parents MAC Address in line 9.
4. Change SSID to your WiFi name in line 12.
5. Change Password to your WiFi password in line 13.
6. Change ChannelNumber to your ThingSpeak ChannelNumber in line 14.
7. change apikey to your ThingSpeak apikey in line 15.
8. Click the Upload button to upload the code to the ESP32 Unit 1 (Bear).

- ESP32 Unit 2 (Mattress):
Code Purpose: Manages the gyroscope (MPU6050).
Upload Instructions:
1. Open the sketch file named Mattress.ino.
2. Copy and paste in Arduino IDE.
3. Change MAC Address to Parents MAC Address in line 17.
4. Change SSID to your WiFi name in line 10.
5. Change Password to your WiFi password in line 11.
6. Change ChannelNumber to your ThingSpeak ChannelNumber in line 14.
7. change apikey to your ThingSpeak apikey in line 15.
8. Click the Upload button to upload the code to the ESP32 Unit 2 (Mattress).

- ESP32 Unit 3 (Parents):
Code Purpose: Controls the LED screen (OLED) and handles data display. Manages visual output and user interface.
Upload Instructions:
1. Open the sketch file for named Parents.ino.
2. copy and paste in Arduino IDE.
3. Change MAC Address to Bear's MAC Address in line 10.
4. Change MAC Address to Mattress's MAC Address in line 11.
5. Click the Upload button to upload the code to the ESP32 Unit 3 (Parent).

- ESP32 CAM:
Code Purpose: Streams video from the baby’s room. Manages the camera and SD card operations.
Upload Instructions:
1. Save the sketch files from the ESP32_CAM_Codes folder into one folder.
2. Open the sketch files from this folder.
3. Select the appropriate board (ESP32 DEV module) from the Tools > Board menu.
4. select the appropriate partition scheme (minimal SPIFFS) from the Tools menu.
5. enable PSRAM (minimal SPIFFS) from the Tools menu
6. Select the correct port from the Tools > Port menu.
7. Click the Upload button to upload the code to the ESP32 CAM.
8. Open the Serial Monitor from the Tools > Serial Monitor menu.
9. Press the RST button on the ESP32 CAM to reset the board.
10. Copy the IP address displayed in the Serial Monitor and paste it into your browser to access the video stream.

**--------------------------------------------got here**
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

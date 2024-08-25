#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define MPU6050_ADDRESS 0x68
#define TWO_MINUTES 120000 // 2 minutes in milliseconds

// WiFi credentials
const char* ssid = "Kim";
const char* password = "0526693338";

// ThingSpeak configuration
const char* apiKey = "O6JD5G6007EWZCIV"; // Your ThingSpeak API key
const unsigned long channelID = 2607721; // Your ThingSpeak Channel ID

uint8_t parentAddress[] = {0x34, 0x94, 0x54, 0x5F, 0x4D, 0x0C}; // MAC address of the Parent device

// State variables
bool movementDetected = false;
int consecutiveMovementCount = 0;
int consecutiveStopCount = 0;
int HM = 0; // Variable to count movement started messages within a 2-minute period
unsigned long lastResetTime = 0; // Time of the last reset

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize I2C communication for MPU6050
  Wire.begin();

  // Reset MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);  // Power management register
  Wire.write(0x80);  // Reset MPU6050
  Wire.endTransmission(true);
  delay(100); // Wait for the reset to complete

  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);  // Power management register
  Wire.write(0);     // Wake up MPU6050
  Wire.endTransmission(true);

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true); // Halt here if initialization fails
  }

  // Add peer (Parent device)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, parentAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true); // Halt here if adding peer fails
  }

  // Initialize last reset time
  lastResetTime = millis();
}

void uploadToThingSpeak() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  HTTPClient http;
  String url = "https://api.thingspeak.com/update?api_key=" + String(apiKey) + "&field3=" + String(HM);
  
  http.begin(url); // Start the connection
  int httpCode = http.GET(); // Make the GET request

  if (httpCode == 200) {
    Serial.println("Data uploaded successfully");
  } else {
    Serial.print("Error uploading data, code: ");
    Serial.println(httpCode);
  }

  http.end(); // End the connection
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi");
}

void loop() {
  // MPU6050 gyroscope data reading and processing logic
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x43);  // Starting register for gyroscope data (0x43 to 0x48 for X,Y,Z)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, 6, true);  // Request 6 bytes (X,Y,Z)

  // Read gyroscope data
  int16_t gyroX = (Wire.read() << 8) | Wire.read();
  int16_t gyroY = (Wire.read() << 8) | Wire.read();
  int16_t gyroZ = (Wire.read() << 8) | Wire.read();

  // Calculate magnitude or use Z-axis value
  int16_t gyroZ_abs = abs(gyroZ); // Use absolute value for simplicity
  
  // Define movement threshold (adjust as needed)
  int movementThreshold = 2000; // Example threshold
  
  // Check if there is movement based on Z-axis gyroscope reading
  bool isMovement = (gyroZ_abs > movementThreshold);

  // Movement detection logic
  if (isMovement) {
    consecutiveStopCount = 0; // Reset stop count
    consecutiveMovementCount++;

    // Check for start message condition
    if (consecutiveMovementCount >= 3 && !movementDetected) {
      movementDetected = true;
      Serial.println("Movement started");
      uint8_t data = 1; // Send 1 for movement started
      esp_err_t result = esp_now_send(parentAddress, &data, sizeof(data));

      if (result == ESP_OK) {
        Serial.println("Movement started signal sent successfully");
        HM++; // Increment the count
        Serial.print("movements count is ");
        Serial.println(HM);
      } else {
        Serial.println("Error sending movement started signal");
      }
    }
  } else {
    consecutiveMovementCount = 0; // Reset movement count
    consecutiveStopCount++;

    // Check for stop message condition
    if (consecutiveStopCount >= 400 && movementDetected) {
      movementDetected = false;
      Serial.println("Movement stopped");
      uint8_t data = 0; // Send 0 for movement stopped
      esp_err_t result = esp_now_send(parentAddress, &data, sizeof(data));

      if (result == ESP_OK) {
        Serial.println("Movement stopped signal sent successfully");
      } else {
        Serial.println("Error sending movement stopped signal");
      }
    }
  }

  // Reset HM every 2 minutes
  unsigned long currentMillis = millis();
  if (currentMillis - lastResetTime >= TWO_MINUTES) {
    Serial.print("movements count before reset is ");
    Serial.println(HM);

    // Connect to WiFi, upload to ThingSpeak, and disconnect before resetting HM
    uploadToThingSpeak();
    
    HM = 0; // Reset HM
    lastResetTime = currentMillis; // Update last reset time
    Serial.println("HM reset");
  }

  delay(10); // Adjust delay as needed before next operation
}

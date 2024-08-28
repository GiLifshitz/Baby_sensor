#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define MPU6050_ADDRESS 0x68
#define ONE_HOUR 2*60*1000 // 1 hour in milliseconds (adjust as needed)

// WiFi credentials
const char* ssid = "SSID";
const char* password = "Password";

// ThingSpeak configuration
const char* apiKey = "apikey";
const unsigned long channelID = ChannelID;

// MAC address of the parent ESP32
uint8_t parentAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

// State variables
bool movementDetected = false;
int consecutiveMovementCount = 0;
int consecutiveStopCount = 0;
int HM = 0;  // Movement counter for ThingSpeak upload
unsigned long lastResetTime = 0;
bool isESPNowMode = true;  // Tracks whether the device is in ESP-NOW mode or WiFi mode

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050
  initMPU6050();

  // Initialize ESP-NOW communication
  initESPNow();

  lastResetTime = millis();
}

void initMPU6050() {
  // Reset and wake up the MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);  // Power management register
  Wire.write(0x80);  // Reset MPU6050
  Wire.endTransmission(true);
  delay(100);

  // Wake up the MPU6050 by setting the power management register to 0
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);  // Power management register
  Wire.write(0);     // Wake up MPU6050
  Wire.endTransmission(true);
}

void initESPNow() {
  // Initialize ESP-NOW in station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Start ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();  // Restart the ESP32 if ESP-NOW initialization fails
    return;
  }

  // Add the parent device as a peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, parentAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();  // Restart the ESP32 if adding the peer fails
    return;
  }

  Serial.println("ESP-NOW initialized and peer added successfully");
}

void switchToWiFi() {
  // Switch to WiFi mode for ThingSpeak data upload
  if (isESPNowMode) {
    esp_now_deinit();  // Disable ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    isESPNowMode = false;  // Indicate that the device is now in WiFi mode
  }
}

void switchToESPNow() {
  // Switch back to ESP-NOW mode after uploading data
  if (!isESPNowMode) {
    WiFi.disconnect(true);  // Disconnect from WiFi
    WiFi.mode(WIFI_STA);
    initESPNow();  // Reinitialize ESP-NOW
    isESPNowMode = true;  // Indicate that the device is now in ESP-NOW mode
  }
}

void uploadToThingSpeak() {
  // Upload the movement data to ThingSpeak
  switchToWiFi();

  HTTPClient http;
  String url = "https://api.thingspeak.com/update?api_key=" + String(apiKey) + "&field3=" + String(HM);
  
  http.begin(url);  // Prepare the HTTP GET request
  int httpCode = http.GET();  // Execute the GET request

  if (httpCode == 200) {
    Serial.println("Data uploaded successfully");
  } else {
    Serial.print("Error uploading data, code: ");
    Serial.println(httpCode);
  }

  http.end();  // End the HTTP request
  switchToESPNow();  // Return to ESP-NOW mode
}

void sendESPNowMessage(uint8_t data) {
  // Send a message via ESP-NOW
  if (isESPNowMode) {
    esp_err_t result = esp_now_send(parentAddress, &data, sizeof(data));
    if (result == ESP_OK) {
      Serial.println(data == 1 ? "Movement started signal sent" : "Movement stopped signal sent");
    } else {
      Serial.println("Error sending signal, reinitializing ESP-NOW");
      // Attempt to reinitialize ESP-NOW if sending fails
      initESPNow();
    }
  } else {
    Serial.println("Not in ESP-NOW mode, message not sent");
  }
}

void processMovement(int16_t gyroZ_abs) {
  // Process the gyroscope data to detect movement
  int movementThreshold = 2000;  // Define the threshold for detecting movement
  bool isMovement = (gyroZ_abs > movementThreshold);

  if (isMovement) {
    consecutiveStopCount = 0;
    consecutiveMovementCount++;

    if (consecutiveMovementCount >= 3 && !movementDetected) {
      movementDetected = true;
      Serial.println("Movement started");
      sendESPNowMessage(1);  // Send movement started signal
      HM++;  // Increment movement counter for ThingSpeak
      Serial.print("Movements count is ");
      Serial.println(HM);
    }
  } else {
    consecutiveMovementCount = 0;
    consecutiveStopCount++;

    if (consecutiveStopCount >= 400 && movementDetected) {
      movementDetected = false;
      Serial.println("Movement stopped");
      sendESPNowMessage(0);  // Send movement stopped signal
    }
  }
}

void loop() {
  // Read gyroscope data from MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x43);  // Request starting register for gyroscope data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, 6, true);  // Request 6 bytes of data (X, Y, Z axes)

  int16_t gyroZ = (Wire.read() << 8) | Wire.read();
  int16_t gyroZ_abs = abs(gyroZ);  // Use absolute value for simplicity

  processMovement(gyroZ_abs);  // Process the movement based on the gyroscope data

  // Check if it’s time to reset the movement counter and upload data
  unsigned long currentMillis = millis();
  if (currentMillis - lastResetTime >= ONE_HOUR) {
    Serial.print("Movements count before reset is ");
    Serial.println(HM);

    uploadToThingSpeak();  // Upload data to ThingSpeak
    
    HM = 0;  // Reset movement counter
    lastResetTime = currentMillis;  // Update the last reset time
    Serial.println("HM reset");
  }

  delay(10);  // Short delay to reduce CPU load
}

#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define MPU6050_ADDRESS 0x68
#define ONE_HOUR 2*60*1000 // 1 hour in milliseconds

// WiFi credentials
const char* ssid = "Kim";
const char* password = "0526693338";

// ThingSpeak configuration
const char* apiKey = "O6JD5G6007EWZCIV";
const unsigned long channelID = 2607721;

uint8_t parentAddress[] = {0x34, 0x94, 0x54, 0x5F, 0x4D, 0x0C};

// State variables
bool movementDetected = false;
int consecutiveMovementCount = 0;
int consecutiveStopCount = 0;
int HM = 0;
unsigned long lastResetTime = 0;
bool isESPNowMode = true;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050
  initMPU6050();

  // Initialize ESP-NOW
  initESPNow();

  lastResetTime = millis();
}

void initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0x80);
  Wire.endTransmission(true);
  delay(100);

  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();  // Restart the ESP32 if ESP-NOW init fails
    return;
  }

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, parentAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();  // Restart the ESP32 if adding peer fails
    return;
  }

  Serial.println("ESP-NOW initialized and peer added successfully");
}

void switchToWiFi() {
  if (isESPNowMode) {
    esp_now_deinit();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    isESPNowMode = false;
  }
}

void switchToESPNow() {
  if (!isESPNowMode) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    initESPNow();
    isESPNowMode = true;
  }
}

void uploadToThingSpeak() {
  switchToWiFi();

  HTTPClient http;
  String url = "https://api.thingspeak.com/update?api_key=" + String(apiKey) + "&field3=" + String(HM);
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    Serial.println("Data uploaded successfully");
  } else {
    Serial.print("Error uploading data, code: ");
    Serial.println(httpCode);
  }

  http.end();
  switchToESPNow();
}

void sendESPNowMessage(uint8_t data) {
  if (isESPNowMode) {
    esp_err_t result = esp_now_send(parentAddress, &data, sizeof(data));
    if (result == ESP_OK) {
      Serial.println(data == 1 ? "signal sent to parents" : "signal sent to parents");
    } else {
      Serial.println("Error sending signal");
      // Attempt to reinitialize ESP-NOW
      initESPNow();
    }
  } else {
    Serial.println("Not in ESP-NOW mode, message not sent");
  }
}

void processMovement(int16_t gyroZ_abs) {
  int movementThreshold = 2000;
  bool isMovement = (gyroZ_abs > movementThreshold);

  if (isMovement) {
    consecutiveStopCount = 0;
    consecutiveMovementCount++;

    if (consecutiveMovementCount >= 3 && !movementDetected) {
      movementDetected = true;
      Serial.println("Movement started");
      sendESPNowMessage(1);
      HM++;
      Serial.print("movements count is ");
      Serial.println(HM);
    }
  } else {
    consecutiveMovementCount = 0;
    consecutiveStopCount++;

    if (consecutiveStopCount >= 400 && movementDetected) {
      movementDetected = false;
      Serial.println("Movement stopped");
      sendESPNowMessage(0);
    }
  }
}

void loop() {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, 6, true);

  int16_t gyroZ = (Wire.read() << 8) | Wire.read();
  int16_t gyroZ_abs = abs(gyroZ);

  processMovement(gyroZ_abs);

  unsigned long currentMillis = millis();
  if (currentMillis - lastResetTime >= ONE_HOUR) {
    Serial.print("movements count before reset is ");
    Serial.println(HM);

    uploadToThingSpeak();
    
    HM = 0;
    lastResetTime = currentMillis;
    Serial.println("HM reset");
  }

  delay(10);
}

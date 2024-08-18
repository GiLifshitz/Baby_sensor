#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_SHT31.h>

// MAC address of the Parent ESP32 (Receiver with OLED)
uint8_t parentAddress[] = {0x34, 0x94, 0x54, 0x5F, 0x4D, 0x0C}; // Replace with the actual MAC address of the Parent ESP32

// Structure to send data
typedef struct struct_message {
    float temperature;
    float humidity;
} struct_message;

// Create a struct_message instance
struct_message myData;

// Create an instance of the SHT31 sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
    Serial.begin(115200);

    // Initialize the SHT31 sensor
    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c address
        Serial.println("Couldn't find SHT31");
        while (1) delay(1);
    }

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    } else {
        Serial.println("ESP-NOW initialized");
    }
    
    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, parentAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    } else {
        Serial.println("Peer added");
    }
}

void loop() {
    // Read temperature and humidity from the SHT31 sensor
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    // Check if the readings are valid
    if (!isnan(t)) {
        myData.temperature = t;
    } else {
        Serial.println("Failed to read temperature");
    }

    if (!isnan(h)) {
        myData.humidity = h;
    } else {
        Serial.println("Failed to read humidity");
    }

    // Send the temperature and humidity data to the Parent device
    esp_err_t result = esp_now_send(parentAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
        Serial.println("Data sent successfully");
    } else {
        Serial.println("Error sending data");
    }

    delay(30000); // Send data every 30 seconds
}

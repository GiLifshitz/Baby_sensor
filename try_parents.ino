#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_SHT31.h>
#include <HTTPClient.h>

// ThingSpeak details
const char* thingSpeakServer = "https://api.thingspeak.com/update?api_key=O6JD5G6007EWZCIV&field1=0";
const char* apiKey = "O6JD5G6007EWZCIV"; // Your ThingSpeak write API key

// MAC address of the Bear ESP32
uint8_t bearAddress[] = {0x44, 0x17, 0x93, 0xE0, 0xA4, 0x20};

// Structure to receive data
typedef struct struct_message {
    float temperature;
    float humidity;
    char message[32];
} struct_message;

// Create a struct_message instance
struct_message myData;

// Create an instance of the sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// WiFi credentials
const char* ssid = "Kim";
const char* password = "0526693338";

bool movementDetected = false; // Variable to store movement status

void setup() {
    Serial.begin(115200);

    // Initialize WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    // Initialize the SHT31 sensor
    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c address
        Serial.println("Couldn't find SHT31");
        while (1) delay(1);
    }

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    } else {
        Serial.println("ESP-NOW initialized");
    }
    
    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, bearAddress, 6);
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
    // Read temperature and humidity
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (!isnan(t)) {  // Check if the reading is valid
        myData.temperature = t;
    } else {
        Serial.println("Failed to read temperature");
    }

    if (!isnan(h)) {  // Check if the reading is valid
        myData.humidity = h;
    } else {
        Serial.println("Failed to read humidity");
    }

    // Prepare data
    snprintf(myData.message, sizeof(myData.message), "%.2f C, %.2f %%", myData.temperature, myData.humidity);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(bearAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
        Serial.println("Message sent successfully");
    } else {
        Serial.println("Error sending the message");
    }

    // Upload to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(thingSpeakServer) + "?api_key=" + apiKey + "&field1=" + String(myData.temperature) + "&field2=" + String(myData.humidity) + "&field3=" + (movementDetected ? "1" : "0");

        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("ThingSpeak update: " + payload);
        } else {
            Serial.println("Error on HTTP request");
        }
        http.end();
    } else {
        Serial.println("Error: Not connected to WiFi");
    }

    delay(30000); // Update ThingSpeak every 30 seconds
}

// Callback function to handle received data (if needed for other functionalities)
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
    // Handle incoming data here
    // Example of setting movementDetected based on received data
    if (len == 1) { // Assuming 1 byte for movement detection
        movementDetected = (incomingData[0] == 1);
    }
}

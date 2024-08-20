#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_SHT31.h>
#include <ThingSpeak.h>
#include <EEPROM.h>

// MAC address of the Parent ESP32 (Receiver with OLED)
uint8_t parentAddress[] = {0x34, 0x94, 0x54, 0x5F, 0x4D, 0x0C}; 

// ThingSpeak credentials
const char *ssid = "Kim";
const char *password = "0526693338";
unsigned long channelNumber = 2607721;
const char *apiKey = "O6JD5G6007EWZCIV";
WiFiClient client;

// Structure to send data
typedef struct struct_message {
    float temperature;
    float humidity;
} struct_message;

// Create a struct_message instance
struct_message myData;

// Create an instance of the SHT31 sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// EEPROM address for the cycle flag
const int flagAddress = 0; 

void setup() {
    Serial.begin(115200);
    EEPROM.begin(1); // Initialize EEPROM with 1 byte size

    // Initialize the SHT31 sensor
    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate I2C address
        Serial.println("Couldn't find SHT31");
        while (1) delay(1);
    }

    // Read the cycle flag from EEPROM
    int cycleFlag = EEPROM.read(flagAddress);

    // If cycleFlag is 0, send to ThingSpeak; if 1, send to the parent ESP32
    if (cycleFlag == 0) {
        sendDataToThingSpeak();
        cycleFlag = 1; // Switch to the next cycle
    } else {
        initEspNow();
        sendDataToParent();
        cycleFlag = 0; // Switch back to ThingSpeak cycle
    }

    // Store the updated flag back to EEPROM
    EEPROM.write(flagAddress, cycleFlag);
    EEPROM.commit();

    // Enter deep sleep for 30 seconds
    enterDeepSleep();
}

void loop() {
    // Nothing to do here since everything happens in setup
}

void initEspNow() {
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    Serial.println("ESP-NOW initialized");

    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, parentAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    } else {
        Serial.println("Peer added");
    }
}

void sendDataToThingSpeak() {
    Serial.println("Switching to Wi-Fi mode...");

    // Switch to Wi-Fi mode
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected");

    // Initialize ThingSpeak
    ThingSpeak.begin(client);

    // Read sensor data
    readSensorData();

    // Set the fields and update ThingSpeak
    ThingSpeak.setField(1, myData.temperature);
    ThingSpeak.setField(2, myData.humidity);
    
    int response = ThingSpeak.writeFields(channelNumber, apiKey);
    if (response == 200) {
        Serial.println("Data sent to ThingSpeak successfully");
    } else {
        Serial.printf("Failed to send data to ThingSpeak. HTTP error code: %d\n", response);
    }

    // Disconnect from Wi-Fi
    WiFi.disconnect();
}

void sendDataToParent() {
    // Read sensor data
    readSensorData();

    // Send the temperature and humidity data to the Parent device via ESP-NOW
    esp_err_t result = esp_now_send(parentAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
        Serial.println("Data sent successfully to Parent");
    } else {
        Serial.println("Error sending data");
    }

    // Disable ESP-NOW before entering deep sleep
    esp_now_deinit();
}

void readSensorData() {
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    // Check if the readings are valid
    if (!isnan(t) && !isnan(h)) {
        myData.temperature = t;
        myData.humidity = h;
    } else {
        Serial.println("Invalid sensor data");
    }
}

void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    // Set the timer to wake up after 30 seconds
    esp_sleep_enable_timer_wakeup(30 * 1000000); // 30 seconds
    esp_deep_sleep_start();
}

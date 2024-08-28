#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_SHT31.h>
#include <ThingSpeak.h>
#include <EEPROM.h>

// MAC address of the Parent ESP32 (Receiver with OLED)
uint8_t parentAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; 

// ThingSpeak credentials
const char *ssid = "SSID";
const char *password = "Password";
unsigned long channelNumber = ChannelID;
const char *apiKey = "apikey";
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

// EEPROM address for the variable X
const int xAddress = 0; 

void setup() {
    Serial.begin(115200);
    EEPROM.begin(4); // Initialize EEPROM with space for float (4 bytes)

    // Initialize the SHT31 sensor
    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate I2C address
        Serial.println("Couldn't find SHT31");
        while (1) delay(1);
    }

    // Read the value of X from EEPROM
    float X = 0;
    EEPROM.get(xAddress, X);

    // Check if X is NaN, and if so, set it to 0
    if (isnan(X)) {
        X = 0;
        EEPROM.put(xAddress, X);
        EEPROM.commit();
    }

    Serial.print("X value on wake up: ");
    Serial.println(X);

    // Step 2: Read temperature and humidity
    readSensorData();

    if (X == 0) {
        // Step 3: Send data via ESP-NOW
        sendDataToParent();
        // Update X to 1
        X = 1;
        EEPROM.put(xAddress, X);
        EEPROM.commit();

        // Step 4: Enter deep sleep for 30 seconds
        deepSleepWithTime(30);
    } else if (X == 1) {
        // Step 5: Send data to ThingSpeak
        sendDataToThingSpeak();
        // Update X to 0
        X = 0;
        EEPROM.put(xAddress, X);
        EEPROM.commit();

        // Step 8: Enter deep sleep for 9 minutes and 28 seconds
        deepSleepWithTime(568);  // 9 * 60 + 28 = 568 seconds
    }
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
    initEspNow();  // Initialize ESP-NOW

    // Send the temperature and humidity data to the Parent device via ESP-NOW
    esp_err_t result = esp_now_send(parentAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
        Serial.println("Data sent successfully to Parent");
    } else {
        Serial.println("Error sending data");
    }

    // Disable ESP-NOW after sending data
    esp_now_deinit();
}

void readSensorData() {
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();
    Serial.print("h: ");
    Serial.println(h);
    Serial.print("t: ");
    Serial.println(t);
    // Check if the readings are valid
    if (!isnan(t) && !isnan(h)) {
        myData.temperature = t;
        myData.humidity = h;
    } else {
        Serial.println("Invalid sensor data");
    }
}

void deepSleepWithTime(int seconds) {
    Serial.printf("Entering deep sleep for %d seconds...\n", seconds);
    esp_sleep_enable_timer_wakeup(seconds * 1000000); // Deep sleep for specified seconds
    esp_deep_sleep_start();
}

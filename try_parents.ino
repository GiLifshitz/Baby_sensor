#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <esp_now.h>

// Initialization of the display with hardware I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

// MAC address of the Bear device (Sender)
uint8_t bearAddress[] = {0x44, 0x17, 0x93, 0xE0, 0xA4, 0x20};  // Replace with the actual MAC address of the Bear device

// Structure to receive data from the Bear
typedef struct struct_message {
    float temperature;
    float humidity;
} struct_message;

// Create a struct_message instance
struct_message myData;

unsigned long lastUpdateTime = 0;

void displayTempHumidity() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);

    char tempStr[32];
    char humStr[32];

    snprintf(tempStr, sizeof(tempStr), "%.2f C", myData.temperature);
    snprintf(humStr, sizeof(humStr), "%.2f %%", myData.humidity);

    u8g2.drawStr(20, 30, tempStr);
    u8g2.drawStr(20, 50, humStr);

    u8g2.sendBuffer();
}

void debugPrintData() {
    Serial.print("Temperature: ");
    Serial.print(myData.temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(myData.humidity);
    Serial.println(" %");
}

// Callback function to handle received data
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
    // Assume data is from the Bear
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.println("Received data from Bear:");

    debugPrintData();  // Print data to Serial Monitor for debugging
    displayTempHumidity();  // Display data on OLED
}

void setup() {
    Serial.begin(115200);
    u8g2.begin();

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    } else {
        Serial.println("ESP-NOW initialized");
    }

    esp_now_register_recv_cb(OnDataRecv);

    // Add Bear as peer
    esp_now_peer_info_t bearPeerInfo = {};
    memcpy(bearPeerInfo.peer_addr, bearAddress, 6);
    bearPeerInfo.channel = 0;
    bearPeerInfo.encrypt = false;
    if (esp_now_add_peer(&bearPeerInfo) != ESP_OK) {
        Serial.println("Failed to add Bear peer");
    } else {
        Serial.println("Bear peer added");
    }

    // Initial message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 30, "Waiting for data...");
    u8g2.sendBuffer();
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastUpdateTime >= 300000) { // Update the display every 5 minutes if no new data
        lastUpdateTime = currentTime;
        displayTempHumidity();
    }
}

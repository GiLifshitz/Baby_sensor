#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <esp_now.h>

// Initialization of the display with hardware I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

// MAC addresses of the sender devices
uint8_t bearAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};  // Bear
uint8_t mattressAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};  // Mattress

// Structure to receive temperature and humidity data from the Bear device
typedef struct struct_message {
    float temperature;
    float humidity;
} struct_message;

// Create a struct_message instance
struct_message myData;

const int ledPin = A4; // LED pin
unsigned long lastUpdateTime = 0;
bool displayMovement = false;
unsigned long movementDisplayTime = 0;
unsigned long messageDisplayDuration = 2000;  // Duration to show movement messages

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
    // Check if the data is coming from the mattress
    if (memcmp(info->src_addr, mattressAddress, 6) == 0) {
        // Data from mattress, treat it as a single byte (1 or 0)
        int movementStatus = incomingData[0];
        
        Serial.print("Received from mattress: ");
        if (movementStatus == 1) {
            Serial.println("Movement started");
            digitalWrite(ledPin, HIGH);  // Turn on LED
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(10, 30, "Movement started");
            u8g2.sendBuffer();
        } else if (movementStatus == 0) {
            Serial.println("Movement stopped");
            digitalWrite(ledPin, LOW);   // Turn off LED
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(10, 30, "Movement stopped");
            u8g2.sendBuffer();
        }
        
        displayMovement = true;
        movementDisplayTime = millis();

    } else if (memcmp(info->src_addr, bearAddress, 6) == 0) {
        // Assume data from bear, use the struct
        memcpy(&myData, incomingData, sizeof(myData));
        Serial.println("Received from bear:");
        debugPrintData();  // Print data to Serial Monitor for debugging
        displayTempHumidity();
    }
}

void setup() {
    Serial.begin(115200);
    u8g2.begin();
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

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
    
    // Add Mattress as peer
    esp_now_peer_info_t mattressPeerInfo = {};
    memcpy(mattressPeerInfo.peer_addr, mattressAddress, 6);
    mattressPeerInfo.channel = 0;  
    mattressPeerInfo.encrypt = false;
    if (esp_now_add_peer(&mattressPeerInfo) != ESP_OK) {
        Serial.println("Failed to add Mattress peer");
    } else {
        Serial.println("Mattress peer added");
    }
    
    // Initial message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 30, "Waiting for messages...");
    u8g2.sendBuffer();
}

void loop() {
    unsigned long currentTime = millis();
    
    if (displayMovement && (currentTime - movementDisplayTime >= messageDisplayDuration)) {
        displayMovement = false;
        displayTempHumidity();
    }

    if (!displayMovement && (currentTime - lastUpdateTime >= 300000)) {
        lastUpdateTime = currentTime;
        displayTempHumidity();
    }
}

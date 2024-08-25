#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    // Wait for serial to initialize
    delay(1000);
  
    // Print the MAC address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void loop() {
    // Nothing to do here
}

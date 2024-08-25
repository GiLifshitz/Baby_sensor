#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int ledPin2 = 2;  // GPIO D2
const int ledPin3 = 3;  // GPIO D3

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize LED pins as outputs
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();

  // Display a welcome message
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,10);             
  display.println(F("Hello, ESP32!"));
  display.display();
  delay(2000);
}

void loop() {
  // Turn on LED 2
  digitalWrite(ledPin2, HIGH);
  delay(1000);  // Wait for 1 second
  
  // Turn off LED 2 and turn on LED 3
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, HIGH);
  delay(1000);  // Wait for 1 second
  
  // Turn off LED 3
  digitalWrite(ledPin3, LOW);

  // Update the OLED display
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(F("LED 2 & 3 Test"));
  display.display();
  
  delay(1000);  // Wait for 1 second
}

#define BLYNK_TEMPLATE_ID "TMPL2ihrjZxyl"
#define BLYNK_TEMPLATE_NAME "Sensor"
#define BLYNK_AUTH_TOKEN "TypYiwEMv85BDQq2JM8FVWr0zgzPGRSf"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Blynk Auth Token & WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TP-LINK_41EEBC";
char pass[] = "LINA@2018@";

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MQ2 Sensor & Pins
#define anInput A0
#define co2Zero 55
#define LED_PIN D3
#define BUZZER_PIN D4

// Alert and timing
bool blinking = false;
bool alertState = false;
unsigned long previousMillis = 0;
const long interval = 500;  // 1Hz blinking

// Blynk event flag
bool eventSent = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Start Blynk
  Blynk.begin(auth, ssid, pass);

  // Start OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    while (true);
  }

  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(20, 27);
  display.println("Akram");
  display.display();
  delay(2000);
}

void loop() {
  Blynk.run();  // Required for Blynk to work

  static unsigned long lastReadTime = 0;
  static int co2ppm = 0;

  if (millis() - lastReadTime >= 2000) {
    lastReadTime = millis();

    // Read CO2 sensor values
    int co2now[10], zzz = 0;
    for (int x = 0; x < 10; x++) {
      co2now[x] = analogRead(anInput);
      delay(20);
    }
    for (int x = 0; x < 10; x++) {
      zzz += co2now[x];
    }

    int co2raw = zzz / 10;
    co2ppm = co2raw - co2Zero;

    // Convert to percentage
    int co2Percent = map(co2ppm, 500, 950, 0, 100);
    co2Percent = constrain(co2Percent, 0, 100);

    // Send to Blynk
    Blynk.virtualWrite(V0, co2Percent);

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("CO2 Level");
    display.println(" ");
    display.print(co2Percent);
    display.print(" %");

    Serial.print("AirQuality: ");
    Serial.print(co2Percent);
    Serial.println(" %");

    int grafX = map(co2Percent, 0, 100, 0, 127);
    display.fillRect(0, 52, grafX, 10, SSD1306_WHITE);
    display.display();

    // Blinking if over 45%
    blinking = (co2Percent > 45);
    if (!blinking) {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      alertState = false;
      eventSent = false;  // Reset the event trigger
    }

    // Trigger Blynk Event if gas > 45%
    if (co2Percent > 45 && !eventSent) {
      Blynk.logEvent("high_gas", "Gas level exceeded 45%!");
      eventSent = true;
    }
  }

  // 1Hz blinking
  if (blinking && millis() - previousMillis >= interval) {
    previousMillis = millis();
    alertState = !alertState;
    digitalWrite(LED_PIN, alertState ? HIGH : LOW);
    digitalWrite(BUZZER_PIN, alertState ? HIGH : LOW);
  }
}

// Optional utility screens (not used in loop)
void testscrolltext(void) {
  display.clearDisplay();
  display.drawRect(0, 0, display.width(), display.height(), SSD1306_WHITE);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 27);
  display.println(F("Akram"));
  display.display();
  delay(3000);
  display.clearDisplay();
}

void WarmingSensor(void) {
  display.clearDisplay();
  display.drawRect(0, 0, display.width(), display.height(), SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(15, 27);
  display.setTextColor(SSD1306_WHITE);
  display.println(F("CO2 Meter"));
  display.display();
  delay(3000);
  display.clearDisplay();
}

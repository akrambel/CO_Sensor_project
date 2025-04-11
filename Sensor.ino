#define BLYNK_TEMPLATE_ID "TMPL2ihrjZxyl"
#define BLYNK_TEMPLATE_NAME "Smart CO Sense"
#define BLYNK_AUTH_TOKEN "TypYiwEMv85BDQq2JM8FVWr0zgzPGRSf"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TP-LINK_41EEBC";
char pass[] = "LINA@2018@";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MQ7_PIN A0
#define LED_PIN D3
#define BUZZER_PIN D4

const float ADC_MAX = 1023.0;
const float Vc = 3.3;
const float RL = 10.0;
const float R0 = 10.0;

const int CO_ALARM_THRESHOLD = 15;

bool blinking = false;
bool alertState = false;
unsigned long previousMillis = 0;
const long interval = 500;

bool wifiConnected = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    while (true);
  }

  display.clearDisplay();
  display.drawRect(0, 0, display.width(), display.height(), SSD1306_WHITE);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 27);
  display.println(F("Smart CO    Sense"));
  display.display();
  delay(3000);
  display.clearDisplay();

  WiFi.begin(ssid, pass);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Blynk.config(auth);
    Blynk.connect();
    Serial.println("WiFi connected.");
  } else {
    wifiConnected = false;
    Serial.println("WiFi NOT connected.");
  }
}

void loop() {
  if (wifiConnected && Blynk.connected()) {
    Blynk.run();
  }

  static unsigned long lastReadTime = 0;

  if (millis() - lastReadTime >= 2000) {
    lastReadTime = millis();

    int sensorValue = analogRead(MQ7_PIN);
    float voltage = (sensorValue / ADC_MAX) * Vc;
    float Rs = (Vc - voltage) * RL / voltage;

    float ratio = Rs / R0;

    float m = -0.77;
    float b = 0.30;
    float ppm = pow(10, ((log10(ratio) - b) / m));
    ppm = constrain(ppm, 0, 2000);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("  CO (PPM)");
    display.setTextSize(1);

    if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
      display.println("     Not connected");
    } else {
      display.println("       Connected");
    }

    display.setTextSize(2);
    display.setCursor(10, 35);
    display.print("   ");
    display.print((int)ppm);
    display.println("ppm");

    int grafX = map(ppm, 0, 1000, 0, 127);
    display.fillRect(0, 52, grafX, 10, SSD1306_WHITE);

    display.display();

    Serial.print("CO concentration: ");
    Serial.print(ppm);
    Serial.println(" ppm");

    if (wifiConnected && Blynk.connected()) {
      Blynk.virtualWrite(V0, ppm);
      if (ppm >= CO_ALARM_THRESHOLD) {
        Blynk.logEvent("co_alert", "🚨 WARNING: High Carbon Monoxide Detected!");
      }
    }

    blinking = (ppm >= CO_ALARM_THRESHOLD);
    if (!blinking) {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      alertState = false;
    }
  }

  if (blinking && millis() - previousMillis >= interval) {
    previousMillis = millis();
    alertState = !alertState;
    digitalWrite(LED_PIN, alertState ? HIGH : LOW);
    digitalWrite(BUZZER_PIN, alertState ? HIGH : LOW);
  }
}

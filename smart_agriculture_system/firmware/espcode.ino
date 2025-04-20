// Include required libraries
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ThingSpeak.h>

// Multiple WiFi Networks
const char* ssid[] = {"T2/102_2.4", "OnePlus Nord CE 3 Lite 5G"};
const char* password[] = {"11223344", "12345678"};

// Firebase Credentials
#define FIREBASE_HOST "https://smart-agriculture-system-fd6a6-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "w9esghtEw5robwjm7bp8pTmPcoA8P2abVwgDI5bw"

// ThingSpeak Credentials
#define THINGSPEAK_API_KEY "MU6YI4PPHDEL5H4O"
unsigned long channelID = 2898953;
WiFiClient client;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Sensor & Module Pins
#define DHTPIN 16
#define SOIL_PIN 32
#define LDR_PIN 33
#define PH_PIN 34
#define RELAY_PIN 14

// Relay Logic
#define RELAY_ON HIGH
#define RELAY_OFF LOW

DHT dht(DHTPIN, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pump Timing Control
unsigned long lastPumpRun = 0;
unsigned long pumpStart = 0;
bool pumpRunning = false;

// Startup Delay
unsigned long startupTime = 0;
bool startupDelayCompleted = false;

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  for (int i = 0; i < 2; i++) {
    WiFi.begin(ssid[i], password[i]);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to: " + String(ssid[i]));
      return;
    }
  }
  Serial.println("\nFailed to connect to any WiFi.");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF); // Ensure pump is OFF immediately on boot

  connectWiFi();

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  ThingSpeak.begin(client);

  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(PH_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  startupTime = millis(); // Mark time at boot
}

String getMLPrediction(float temp, float hum, int soil, int light, float ph) {
  HTTPClient http;
  String url = "https://8c89-34-73-150-177.ngrok-free.app/predict"; // 🔄 Update this with your latest ngrok link
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"temperature\": " + String(temp) + 
                    ", \"humidity\": " + String(hum) + 
                    ", \"soil_moisture\": " + String(soil) + 
                    ", \"light\": " + String(light) + 
                    ", \"ph\": " + String(ph) + "}";

  int httpResponseCode = http.POST(jsonData);
  String response = http.getString();
  http.end();

  Serial.println("[ML Prediction] Response: " + response);

  if (response.indexOf("water") != -1) {
    return "water";
  } else {
    return "no_water";
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Startup delay: skip loop logic until 5 minutes passed
  if (!startupDelayCompleted && currentMillis - startupTime < 300000) {
    Serial.println("[Init] Waiting 5 minutes before activating pump logic...");
    lcd.setCursor(0, 0);
    lcd.print("Waiting 5 mins...");
    lcd.setCursor(0, 1);
    lcd.print("Pump locked      ");
    delay(1000);
    return;
  } else {
    startupDelayCompleted = true;
  }

  Serial.println("\n----- New Cycle -----");

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);
  int light = analogRead(LDR_PIN);
  float ph = analogRead(PH_PIN) * (14.0 / 4095.0); // Convert analog to pH scale

  Serial.printf("[Sensors] Temp: %.2f °C, Hum: %.2f %%, Soil: %d, Light: %d, pH: %.2f\n", temp, hum, soil, light, ph);

  Firebase.setFloat(fbdo, "/hydroponic/temp", temp);
  Firebase.setFloat(fbdo, "/hydroponic/hum", hum);
  Firebase.setInt(fbdo, "/hydroponic/soil", soil);
  Firebase.setInt(fbdo, "/hydroponic/light", light);
  Firebase.setFloat(fbdo, "/hydroponic/ph", ph);

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, hum);
  ThingSpeak.setField(3, soil);
  ThingSpeak.setField(4, light);
  ThingSpeak.setField(5, ph);
  ThingSpeak.writeFields(channelID, THINGSPEAK_API_KEY);

  String prediction = getMLPrediction(temp, hum, soil, light, ph);
  static String lastPrediction = "";
  Serial.println("[ML Decision] Prediction: " + prediction);

  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp, 1);
  lcd.print(" H:"); lcd.print(hum, 0);
  lcd.setCursor(0, 1);
  lcd.print("S:"); lcd.print(soil);
  lcd.print(" pH:"); lcd.print(ph, 1);

  // Scheduled Pumping: Every 15 minutes
  if (!pumpRunning && currentMillis - lastPumpRun >= 900000) {
    Serial.println("[Pump] Scheduled watering triggered.");
    digitalWrite(RELAY_PIN, RELAY_ON);
    pumpStart = currentMillis;
    pumpRunning = true;
    lastPumpRun = currentMillis;
  }

  // ML-based Pumping: on prediction change to "water"
  if (prediction != lastPrediction && prediction == "water" && !pumpRunning && currentMillis - lastPumpRun >= 900000) {
    Serial.println("[Pump] ML Model triggered watering (new prediction).");
    digitalWrite(RELAY_PIN, RELAY_ON);
    pumpStart = currentMillis;
    pumpRunning = true;
    lastPumpRun = currentMillis;
    lastPrediction = prediction;
  } else {
    lastPrediction = prediction;
  }

  // Stop pump after 5 seconds
  if (pumpRunning && currentMillis - pumpStart >= 5000) {
    digitalWrite(RELAY_PIN, RELAY_OFF);
    pumpRunning = false;
    Serial.println("[Pump] Watering stopped after 5 seconds.");
  }

  Serial.print("[Pump] Current Status: ");
  Serial.println(pumpRunning ? "ON" : "OFF");

  for (int i = 0; i < 150; i++) {
    delay(100); // Total 15 seconds between loop cycles
    yield();
  }
}

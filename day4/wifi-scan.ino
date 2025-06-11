#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "HTTPClient.h"

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak API
const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "EENA9YAOETOVWXP9";

// Pin configuration
#define ONE_WIRE_BUS 33      // DS18B20 data pin
#define SOIL_PIN 32          // Soil moisture sensor (analog)
#define RELAY_PIN 14         // Relay control pin

// Setup sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  // Start WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Sensor & relay setup
  sensors.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // OFF initially (for active-low relay)
}

void loop() {
  // Read temperature
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // Read soil moisture
  int soilValue = analogRead(SOIL_PIN);

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print(" °C | Soil Moisture: ");
  Serial.println(soilValue);

  // Relay control condition
  if (tempC > 35 && soilValue < 250) {
    digitalWrite(RELAY_PIN, LOW); // ON (active-low)
    Serial.println("Relay ON (Pump ON)");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // OFF
    Serial.println("Relay OFF (Pump OFF)");
  }

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(tempC) +
                 "&field2=" + String(soilValue);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak successfully.");
    } else {
      Serial.println("Failed to send data.");
    }
    http.end();
  }

  delay(15000); // ThingSpeak allows 1 update every 15–20 seconds
}

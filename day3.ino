#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Sensor Definitions
#define DHTPIN 15
#define DHTTYPE DHT22
#define MOISTURE_PIN 34
#define CO2_SENSOR_PIN 35

// OLED Display I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi & ThingSpeak
const char* ssid = "Wokwi-GUEST";
const char* password = "";
String apiKey = "6T77VJ4SUKC2PALP";
const char* server = "http://api.thingspeak.com/update";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // I2C & OLED
  Wire.begin(21, 22); // SDA = 21, SCL = 22
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1);
  }
  display.clearDisplay();
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Wi-Fi
  WiFi.begin(ssid, password);
  dht.begin();

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int moistureValue = analogRead(MOISTURE_PIN);
  int soilADC = map(moistureValue, 0, 4095, 0, 1023);
  String soilCondition;
  if (soilADC < 400) soilCondition = "Very Wet";
  else if (soilADC < 600) soilCondition = "Moist";
  else if (soilADC < 800) soilCondition = "Dry";
  else soilCondition = "Very Dry";

  int co2Value = analogRead(CO2_SENSOR_PIN);
  int co2ADC = map(co2Value, 0, 4095, 0, 1023);
  int co2ppm = map(co2ADC, 0, 1023, 400, 2000);
  String co2Condition;
  if (co2ADC <= 300) co2Condition = "Fresh";
  else if (co2ADC <= 600) co2Condition = "Moderate";
  else if (co2ADC <= 800) co2Condition = "High";
  else co2Condition = "Very High";

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("🌿 Greenhouse Monitor");
  display.println("---------------------");
  display.print("Temp: "); display.print(temperature); display.println(" C");
  display.print("Hum : "); display.print(humidity); display.println(" %");
  display.print("Soil: "); display.print(soilCondition); 
  display.print(" ("); display.print(soilADC); display.println(")");
  display.print("CO2 : "); display.print(co2ppm); display.print(" ppm");
  display.print(" "); display.println(co2Condition);
  display.display();

  // Serial Monitor
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" °C | Humidity: "); Serial.print(humidity);
  Serial.print(" % | Soil: "); Serial.print(soilADC);
  Serial.print(" ("); Serial.print(soilCondition); Serial.print(")");
  Serial.print(" | CO2: "); Serial.print(co2ppm);
  Serial.print(" ppm ("); Serial.print(co2Condition); Serial.println(")");

  // ThingSpeak Upload
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(humidity);
    url += "&field3=" + String(soilADC);
    url += "&field4=" + String(co2ppm);

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak!");
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }
    http.end();
  }

  delay(20000); // 20s delay for ThingSpeak rate limit
}

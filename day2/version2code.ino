#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// CO2 sensor simulated with potentiometer
#define CO2_SENSOR_PIN 33  // Analog pin (ADC1_CH5)

// Read and average multiple ADC readings to reduce noise
int readAverageADC(int pin, int samples = 10) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(5);
  }
  return total / samples;
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);  // Stop here if OLED is not detected
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Configure ADC settings (important for ESP32)
  analogSetAttenuation(ADC_11db);  // Allows full 0–3.3V range
  analogSetWidth(12);              // 12-bit ADC (0–4095)
}

void loop() {
  // Get averaged analog value
  int adcValue = readAverageADC(CO2_SENSOR_PIN);
  float voltage = adcValue * (3.3 / 4095.0);  // Convert ADC value to voltage

  // Approximate conversion to CO2 ppm (based on MG-811 sensor behavior)
  float co2ppm = -8000.0 * voltage + 20400.0;
  co2ppm = constrain(co2ppm, 400, 10000);  // Limit to realistic CO2 range

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("CO2 Level:");
  display.setCursor(0, 30);
  display.print(co2ppm, 0);  // Display as integer
  display.print(" ppm");
  display.display();

  // Output to Serial Monitor
  Serial.print("ADC: ");
  Serial.print(adcValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | CO2: ");
  Serial.print(co2ppm, 1);
  Serial.println(" ppm");

  delay(1000);  // Update once per second
}

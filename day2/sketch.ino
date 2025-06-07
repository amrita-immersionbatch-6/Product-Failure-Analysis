#define CO2_SENSOR_PIN 33

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Read analog CO2 sensor (potentiometer)
  int adcVal = analogRead(CO2_SENSOR_PIN);
  float voltage = adcVal * (3.3 / 4095.0); // Convert ADC value to voltage (ESP32 ADC is 12-bit)

  // Convert voltage to CO2 ppm based on MG-811 approximation
  float co2ppm = -8000 * voltage + 20400;

  // Clamp ppm values to realistic range
  co2ppm = constrain(co2ppm, 400, 10000);

  Serial.print("Simulated CO2 Level (ppm): ");
  Serial.println(co2ppm);

  delay(1000);  // Print every second
}

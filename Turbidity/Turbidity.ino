#include <SPI.h>
#include <OneWire.h>

void setup() {
  Serial.begin(115200);
  
  // Configure LED pins
  pinMode(LED_CLEAR, OUTPUT);
  pinMode(LED_CLOUDY, OUTPUT);
  pinMode(LED_DIRTY, OUTPUT);

  Serial.println("AquaSentinel: ESP32 Turbidity Monitoring (Serial Mode)");
}

void loop() {
  // 1. Read the raw value (0 - 4095)
  int sensorValue = analogRead(SENSOR_PIN);
  
  // 2. Map the value to a 0-100 percentage based on your sensor's range
  // Note: We use mapFloat to handle your inverse relationship 
  // (Higher ADC = Clearer Water)
  float turbidity = mapFloat(sensorValue, AIR_VAL, WATER_VAL, 100, 0);

  // Constraints to keep it within 0-100%
  if (turbidity < 0) turbidity = 0;
  if (turbidity > 100) turbidity = 100;

  // 3. Print to Serial Monitor
  Serial.print("Raw ADC: ");
  Serial.print(sensorValue);
  Serial.print(" | Turbidity: ");
  Serial.print(turbidity, 1);
  Serial.println("%");

  // 4. Logic for LEDs and Status
  if (turbidity < 20) {
    digitalWrite(LED_CLEAR, HIGH);
    digitalWrite(LED_CLOUDY, LOW);
    digitalWrite(LED_DIRTY, LOW);
    Serial.println("Status: ITS CLEAR");
  } 
  else if (turbidity >= 20 && turbidity <= 50) {
    digitalWrite(LED_CLEAR, LOW);
    digitalWrite(LED_CLOUDY, HIGH);
    digitalWrite(LED_DIRTY, LOW);
    Serial.println("Status: ITS CLOUDY");
  } 
  else {
    digitalWrite(LED_CLEAR, LOW);
    digitalWrite(LED_CLOUDY, LOW);
    digitalWrite(LED_DIRTY, HIGH);
    Serial.println("Status: ITS DIRTY");
  }

  delay(500); // Faster update for real-time monitoring
}

// Custom mapping function for ESP32 precision
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

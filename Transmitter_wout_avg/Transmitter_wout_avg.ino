#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- PIN DEFINITIONS ---
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

#define ONE_WIRE_BUS 4     // Temp Sensor
#define PH_PIN 34          // pH Sensor
#define TDS_PIN 33         // TDS (Moved from 34 to avoid conflict)
#define TURB_PIN 32        // Turbidity
#define TRIG_PIN 27        // Ultrasonic
#define ECHO_PIN 25        // Ultrasonic (Moved from 14 to avoid LoRa conflict)

// --- SENSOR SETUP ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Turbidity Calibration
const int AIR_VAL = 1600;
const int WATER_VAL = 2100;

void setup() {
  Serial.begin(115200);
  
  // Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }
  Serial.println("AquaSentinel Transmitter Ready!");

  // Initialize Sensors
  sensors.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // 1. READ TEMPERATURE
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // 2. READ pH
  int phRaw = analogRead(PH_PIN);
  float phVolt = phRaw * (3.3 / 4095.0);
  float phValue = 7 + ((2.5 - phVolt) / 0.18);

  // 3. READ TDS & EC
  float tdsSum = 0;
  for(int i=0; i<10; i++) { tdsSum += analogRead(TDS_PIN); delay(5); }
  float tdsVolt = (tdsSum/10.0) * 3.3 / 4095.0;
  float tdsValue = (133.42*pow(tdsVolt,3) - 255.86*pow(tdsVolt,2) + 857.39*tdsVolt) * 0.5;
  float ecValue = tdsValue / 0.5;

  // 4. READ TURBIDITY (NTU)
  int turbRaw = analogRead(TURB_PIN);
  float ntu = (turbRaw >= WATER_VAL) ? 0 : mapFloat(turbRaw, AIR_VAL, WATER_VAL, 500, 0);
  if (ntu < 0) ntu = 0;

  // 5. READ ULTRASONIC (DISTANCE)
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 40000);
  float distance = duration * 0.0343 / 2;

  // 6. TRANSMIT VIA LORA
  LoRa.beginPacket();
  LoRa.print("TEMP:"); LoRa.print(tempC);
  LoRa.print("|PH:");   LoRa.print(phValue);
  LoRa.print("|TDS:");  LoRa.print(tdsValue);
  LoRa.print("|NTU:");  LoRa.print(ntu);
  LoRa.print("|DIST:"); LoRa.print(distance);
  LoRa.endPacket();

  // Local Debug Output
  Serial.print("Data Sent: ");
  Serial.println(tempC); 

  delay(3000); // 3-second interval for stability
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// ESP32 Pin Definitions
#define SENSOR_PIN 32    // Using GPIO 32 for Turbidity
#define LED_CLEAR  3     // Green LED
#define LED_CLOUDY 4     // Yellow LED
#define LED_DIRTY  5     // Red LED

// Calibration baselines based on your previous readings
const int AIR_VAL = 1600;    // Represents "Dirty"
const int WATER_VAL = 2100;  // Represents "Clear"


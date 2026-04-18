#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <math.h>

// -------- Pins --------
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 26
#define GPS_RX_PIN 12 
#define GPS_TX_PIN 13
#define TX2_PIN 17
#define RX2_PIN 16
#define ONE_WIRE_BUS 4
#define PH_PIN 34
#define TDS_PIN 35 
#define TURB_PIN 32
#define TRIG_PIN 27
#define ECHO_PIN 25
#define MQ4_PIN 33 
#define MQ135_PIN 39 

// -------- Constants --------
const float MQ4_R0 = 0.33;    // Change this to your calibrated R0
const float MQ135_R0 = 12.5;  // Change this to your calibrated R0
const float RL_BOARD = 1.0;
const int AIR_VAL = 1600;    
const int WATER_VAL = 2100;  

// -------- Global Variables --------
float previousDist = 0;
float lastLat = 0.000000;
float lastLng = 0.000000;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
TinyGPSPlus gps;
HardwareSerial SerialGPS(1); 

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN); 
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); 

  analogSetAttenuation(ADC_11db);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }
  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(7);

  sensors.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("AquaSentinel V4.1: Transmitter Ready");
}

void loop() {
  int samples = 0;
  float tempSum = 0, phSum = 0, tdsSum = 0, ntuSum = 0, distSum = 0;
  float mq4Sum = 0, mq135Sum = 0;
  
  unsigned long startTime = millis();

  while (millis() - startTime < 10000) { // 10s Window
    // 1. Process GPS continuously
    while (SerialGPS.available() > 0) {
      gps.encode(SerialGPS.read());
    }

    // 2. Ultrasonic & Nano Data (Send to Nano every 200ms)
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(3);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(15);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    float currentDist = (duration * 0.0343 / 2.0);
    
    if (duration == 0 || currentDist > 500) currentDist = previousDist;
    else previousDist = currentDist;
    
    Serial2.println(currentDist); // Keeps Nano steering responsive
    distSum += currentDist;

    // 3. Sensor Sampling
    sensors.requestTemperatures();
    tempSum += sensors.getTempCByIndex(0);
    
    float phVolt = analogRead(PH_PIN) * (3.3 / 4095.0);
    phSum += 7 + ((2.5 - phVolt) / 0.18);

    float tdsVolt = analogRead(TDS_PIN) * (3.3 / 4095.0);
    tdsSum += (133.42 * pow(tdsVolt, 3) - 255.86 * pow(tdsVolt, 2) + 857.39 * tdsVolt) * 0.5;

    int turbRaw = analogRead(TURB_PIN);
    ntuSum += constrain(mapFloat((float)turbRaw, (float)AIR_VAL, (float)WATER_VAL, 500.0, 0.0), 0, 500);

    mq4Sum   += calculatePPM(MQ4_PIN, MQ4_R0, 1012.7, -2.786);
    mq135Sum += calculatePPM(MQ135_PIN, MQ135_R0, 110.47, -2.862);

    samples++;
    delay(200); 
  }

  // Final Calculations
  float avgTemp = tempSum / samples;
  float avgPh   = phSum / samples;
  float avgTds  = tdsSum / samples;
  float avgNtu  = ntuSum / samples;
  float avgDist = distSum / samples;
  float avgMQ4  = mq4Sum / samples;
  float avgMQ135 = mq135Sum / samples;

  // GPS Persistence Logic
  if (gps.location.isValid()) {
    lastLat = gps.location.lat();
    lastLng = gps.location.lng();
  }

  // Transmit via LoRa
  String dataPacket = "T:" + String(avgTemp, 1) + "|P:" + String(avgPh, 1) + 
                      "|S:" + String(avgTds, 0) + "|N:" + String(avgNtu, 1) + 
                      "|D:" + String(avgDist, 1) + "|M:" + String(avgMQ4, 1) + 
                      "|A:" + String(avgMQ135, 1) + 
                      "|LAT:" + String(lastLat, 6) + "|LNG:" + String(lastLng, 6);

  Serial.println("Sending: " + dataPacket);
  if (gps.satellites.value() == 0) Serial.println("Warning: No GPS Lock");

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();
}

float calculatePPM(int pin, float r0, float a, float b) {
  float volt = (analogRead(pin) / 4095.0) * 3.3;
  if (volt < 0.1) volt = 0.1;
  float rs = ((3.3 / volt) - 1.0) * RL_BOARD;
  float ratio = rs / r0;
  return a * pow(ratio, b);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
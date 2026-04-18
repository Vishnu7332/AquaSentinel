#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

// -------- LoRa Pins (ESP32 DOIT) --------
#define LORA_SS     5
#define LORA_RST    14
#define LORA_DIO0  26

// -------- Serial Data Out to Nano --------
#define TX2_PIN 17
#define RX2_PIN 16

// -------- Sensor Pins --------
#define ONE_WIRE_BUS 4
#define PH_PIN     34
#define TDS_PIN    33
#define TURB_PIN   32
#define TRIG_PIN   27
#define ECHO_PIN   25

// -------- Global variables for filtered distance --------
float previousDist = 0;

// -------- Sensor Setup --------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int AIR_VAL = 1600;    
const int WATER_VAL = 2100;  

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  Serial.println("\n========================================");
  Serial.println("   AquaSentinel AI Transmitter (V3)     ");
  Serial.println("========================================");

  analogSetAttenuation(ADC_11db);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

  sensors.begin();
  
  if (sensors.getDeviceCount() == 0) {
    Serial.println("WARNING: Temperature Sensor NOT found!");
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  int samples = 0;
  int tempSamples = 0; 
  float tempSum = 0, phSum = 0, tdsSum = 0, ntuSum = 0, distSum = 0;
  unsigned long startTime = millis();

  Serial.println("\nCollecting data for 10 seconds...");

  while (millis() - startTime < 10000) {
    // --- IMPROVED ULTRASONIC MEASUREMENT LOGIC ---
    digitalWrite(TRIG_PIN, LOW); 
    delayMicroseconds(3);
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(15);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 50000);
    float currentDist = (duration * 0.0343 / 2.0);

    // Filter invalid readings (Sample and Hold Filter)
    if (duration == 0 || currentDist <= 2 || currentDist > 500) {
      currentDist = previousDist;
    } else {
      previousDist = currentDist;
    }

    // --- SERIAL DATA TRANSMISSION TO NANO ---
    Serial2.println(currentDist); 

    distSum += currentDist;

    // --- TEMPERATURE ---
    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);
    if (currentTemp != DEVICE_DISCONNECTED_C && currentTemp != 85.00) {
      tempSum += currentTemp;
      tempSamples++;
    }

    // --- pH ---
    int phRaw = analogRead(PH_PIN);
    float phVolt = phRaw * (3.3 / 4095.0);
    phSum += 7 + ((2.5 - phVolt) / 0.18);

    // --- TDS ---
    int tdsRaw = analogRead(TDS_PIN);
    float tdsVolt = tdsRaw * (3.3 / 4095.0);
    tdsSum += (133.42 * pow(tdsVolt, 3) - 255.86 * pow(tdsVolt, 2) + 857.39 * tdsVolt) * 0.5;

    // --- TURBIDITY ---
    int turbRaw = analogRead(TURB_PIN);
    float ntu = mapFloat((float)turbRaw, (float)AIR_VAL, (float)WATER_VAL, 500.0, 0.0);
    if (ntu < 0) ntu = 0;
    if (ntu > 500) ntu = 500;
    ntuSum += ntu;

    samples++;
    delay(200); 
  }

  // Calculate Averages
  float avgTemp = (tempSamples > 0) ? (tempSum / tempSamples) : 0.0;
  float avgPh   = phSum / samples;
  float avgTds  = tdsSum / samples;
  float avgNtu  = ntuSum / samples;
  float avgDist = distSum / samples;

  // Analysis Logic
  String waterStatus;
  char statusCode;
  if ((avgPh >= 6.2 && avgPh <= 7.2) && (avgTds <= 500) && (avgNtu <= 35)) {
    waterStatus = "SAFE (Drinking/Domestic)";
    statusCode = 'S';
  } 
  else if ((avgPh >= 7.3 && avgPh <= 10) && (avgTds <= 1000) && (avgNtu <= 110)) {
    waterStatus = "MODERATELY SAFE (Irrigation)";
    statusCode = 'M';
  } 
  else {
    waterStatus = "UNSAFE / CONTAMINATED";
    statusCode = 'U';
  }

  // -------- REPORT --------
  Serial.println("\n--- AQUASENTINEL REPORT ---");
  if (tempSamples == 0) Serial.println("Temperature : ERROR (Check Wiring/Resistor)");
  else { Serial.print("Temperature : "); Serial.print(avgTemp, 1); Serial.println(" C"); }
  
  Serial.print("pH Value    : "); Serial.println(avgPh, 1);
  Serial.print("TDS Level   : "); Serial.print(avgTds, 0); Serial.println(" ppm");
  Serial.print("Turbidity   : "); Serial.print(avgNtu, 1); Serial.println(" NTU");
  Serial.print("Distance    : "); Serial.print(avgDist, 1); Serial.println(" cm");
  Serial.print("Water Status: "); Serial.println(waterStatus);
  Serial.println("---------------------------");

  // -------- TRANSMIT --------
  String dataPacket = "T:" + String(avgTemp, 1) + "|P:" + String(avgPh, 1) + 
                      "|S:" + String(avgTds, 0) + "|N:" + String(avgNtu, 1) + 
                      "|D:" + String(avgDist, 1) + "|W:" + statusCode;

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  delay(2000);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
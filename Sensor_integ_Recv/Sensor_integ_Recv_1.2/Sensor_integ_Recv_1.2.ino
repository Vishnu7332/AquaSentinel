#include <SPI.h>
#include <LoRa.h>

// --- LoRa Pins (Standard ESP32) ---
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  2 // Ensure this matches your Receiver's wiring

void setup() {
  Serial.begin(115200);

  Serial.println("\n========================================");
  Serial.println("   AquaSentinel Base Station: ONLINE 🛰️");
  Serial.println("   Testing @ 20m Range...               ");
  Serial.println("========================================");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Must match the 433MHz frequency used in your boat
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed! Check connections.");
    while (1);
  }

  // Matching Transmitter SF7 and SyncWord 0x12
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

  Serial.println("Listening for LoRa Packet...");
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    // --- RSSI / Signal Integrity Check ---
    int rssi = LoRa.packetRssi();
    
    // --- PARSING NUMERICAL VALUES ---
    float temp = getValue(receivedData, 'T');
    float ph   = getValue(receivedData, 'P');
    float tds  = getValue(receivedData, 'S');
    float ntu  = getValue(receivedData, 'N');
    float dist = getValue(receivedData, 'D');
    float meth = getValue(receivedData, 'M'); // Methane
    float ammo = getValue(receivedData, 'A'); // Ammonia

    // --- PARSING GPS STRINGS ---
    String lat = getStringValue(receivedData, "LAT");
    String lng = getStringValue(receivedData, "LNG");

    // --- DISPLAY REPORT ---
    Serial.println("\n--- AQUASENTINEL FIELD DATA ---");
    Serial.print("📡 Signal Strength: "); Serial.print(rssi); Serial.println(" dBm");
    
    if (lat == "0.000000") {
      Serial.println("📍 GPS: SEARCHING FOR LOCK...");
    } else {
      Serial.print("📍 Location: "); Serial.print(lat); Serial.print(", "); Serial.println(lng);
    }

    Serial.println("--------------------------------");
    Serial.print("🌡️ Temp: "); Serial.print(temp, 1); Serial.print(" C | ");
    Serial.print("🧪 pH: "); Serial.println(ph, 1);
    Serial.print("💧 TDS: "); Serial.print(tds, 0); Serial.print(" ppm | ");
    Serial.print("🌫️ NTU: "); Serial.println(ntu, 1);
    Serial.print("🛢️ CH4: "); Serial.print(meth, 1); Serial.print(" PPM | ");
    Serial.print("🧼 NH3: "); Serial.println(ammo, 1);
    Serial.print("📏 Dist to Water: "); Serial.print(dist, 1); Serial.println(" cm");
    Serial.println("--------------------------------");

    // --- SIGNAL QUALITY ALERT ---
    if (rssi < -105) {
      Serial.println("⚠️ WARNING: Boat is at maximum range!");
    }

    // --- WATER QUALITY LOGIC ---
    if (meth > 1000 || ammo > 200 || ph < 6.0 || ph > 9.0) {
      Serial.println("🚨 ALERT: POLLUTION HOTSPOT DETECTED!");
    } else {
      Serial.println("✅ Readings within expected range.");
    }
  }
}

// --- PRECISION PARSER FOR NUMERICAL DATA ---
float getValue(String data, char key) {
  String keyStr = String(key) + ":";
  int start = data.indexOf(keyStr);
  if (start == -1) return 0.0;
  
  int colon = data.indexOf(':', start);
  int end = data.indexOf('|', colon);
  if (end == -1) end = data.length();
  
  return data.substring(colon + 1, end).toFloat();
}

// --- PRECISION PARSER FOR GPS STRINGS ---
String getStringValue(String data, String key) {
  String keyStr = key + ":";
  int start = data.indexOf(keyStr);
  if (start == -1) return "0.000000";
  
  int colon = data.indexOf(':', start);
  int end = data.indexOf('|', colon);
  if (end == -1) end = data.length();
  
  return data.substring(colon + 1, end);
}
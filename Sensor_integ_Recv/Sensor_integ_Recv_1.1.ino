#include <SPI.h>
#include <LoRa.h>

// --- LoRa Pins ---
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

void setup() {
  Serial.begin(115200);

  Serial.println("\n========================================");
  Serial.println("   AquaSentinel Receiver: ONLINE 🚀");
  Serial.println("========================================");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }

  // MUST match transmitter
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

  Serial.println("Waiting for AquaSentinel data...\n");
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    // --- TIME STAMP ---
    unsigned long totalSeconds = millis() / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    Serial.print("\n📡 Data Received @ ");
    if(minutes < 10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if(seconds < 10) Serial.print("0");
    Serial.println(seconds);

    Serial.print("Raw Packet: ");
    Serial.println(receivedData);

    Serial.print("Signal Strength (RSSI): ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");

    // --- PARSING VALUES ---
    float temp = getValue(receivedData, 'T');
    float ph   = getValue(receivedData, 'P');
    float tds  = getValue(receivedData, 'S');
    float ntu  = getValue(receivedData, 'N');
    float dist = getValue(receivedData, 'D');

    // --- FORMATTED OUTPUT ---
    Serial.println("\n===== AquaSentinel Report =====");
    Serial.print("🌡 Temperature : "); Serial.print(temp); Serial.println(" °C");
    Serial.print("🧪 pH Level   : "); Serial.println(ph);
    Serial.print("💧 TDS Value  : "); Serial.print(tds); Serial.println(" ppm");
    Serial.print("🌫 Turbidity  : "); Serial.print(ntu); Serial.println(" NTU");
    Serial.print("📏 Water Dist : "); Serial.print(dist); Serial.println(" cm");
    Serial.println("================================");

    // --- QUICK STATUS ---
    if (ph < 6.5 || ph > 8.5 || tds > 500 || ntu > 300) {
      Serial.println("⚠️ ALERT: Water Quality NOT SAFE");
    } else {
      Serial.println("✅ Water Quality NORMAL");
    }

    Serial.println("--------------------------------");
  }

  delay(1000);
}

// --- FUNCTION TO PARSE VALUES ---
float getValue(String data, char key) {
  int start = data.indexOf(key);
  if (start == -1) return 0;

  int colon = data.indexOf(':', start);
  int end = data.indexOf('|', colon);

  if (end == -1) end = data.length();

  return data.substring(colon + 1, end).toFloat();
}
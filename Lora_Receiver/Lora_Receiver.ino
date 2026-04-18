#include <SPI.h>
#include <LoRa.h>

// --- PIN DEFINITIONS ---
#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(115200); 
  while (!Serial); 

  Serial.println("\n--- AquaSentinel LoRa Chat (Receiver) ---");
  Serial.println("Waiting for messages...");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Init Failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0x12);
}

void loop() {
  // 1. Check for incoming packets
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    String incomingData = "";
    
    // 2. Read the packet into a String
    while (LoRa.available()) {
      incomingData += (char)LoRa.read();
    }

    // 3. Print the result
    Serial.print("\n[Message Received]: ");
    Serial.println(incomingData);
    
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
    Serial.println("-------------------------");
  }
}
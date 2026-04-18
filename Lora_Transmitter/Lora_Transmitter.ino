#include <SPI.h>
#include <LoRa.h>

// --- Professional Pin Mapping ---
const int SCK_PIN  = 18;
const int MISO_PIN = 19;
const int MOSI_PIN = 23;
const int SS_PIN   = 5;
const int RST_PIN  = 14;
const int DIO0_PIN = 26;

unsigned int packetCounter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial); 
  
  Serial.println("\n[SYSTEM] Starting LoRa Transmitter Diagnostic...");
  
  // Initialize LoRa Pins
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  // Attempt to begin communication
  Serial.print("[DEBUG] Searching for LoRa module at 433MHz...");
  if (!LoRa.begin(433E6)) {
    Serial.println("\n[ERROR] LoRa Init Failed!");
    Serial.println(" -> Check SPI Wiring (SCK:18, MISO:19, MOSI:23, SS:5)");
    Serial.println(" -> Check Power (Ensure 3.3V is stable)");
    while (1); // Halt
  }

  // Set Professional Radio Parameters
  LoRa.setSyncWord(0x12);           // Private Network ID
  LoRa.setSpreadingFactor(10);      // Higher SF = Better Range/Stability
  LoRa.setSignalBandwidth(125E3);   // Standard Bandwidth
  LoRa.setCodingRate4(5);           // Error correction
  
  Serial.println(" SUCCESS.");
  Serial.println("[SYSTEM] Transmitter Ready. Entering Loop...\n");
}

void loop() {
  Serial.print("[TX] Sending Packet #");
  Serial.print(packetCounter);
  
  // Start Packet
  if (LoRa.beginPacket()) {
    LoRa.print("AquaSentinel_Test_");
    LoRa.print(packetCounter);
    
    // Finalize and Send
    if (LoRa.endPacket()) {
      Serial.println(" | STATUS: Delivered to Buffer.");
      packetCounter++;
    } else {
      Serial.println(" | STATUS: Transmission Timeout!");
    }
  } else {
    Serial.println(" | STATUS: Radio Busy/Error.");
  }

  delay(3000); // 3-second heartbeat
}
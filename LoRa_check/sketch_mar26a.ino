#include <SPI.h>
#include <LoRa.h>

#define SS   5
#define RST  14
#define DIO0 26

void setup() {
  Serial.begin(115200);
  Serial.println("LoRa Transmitter Test");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa Init Failed!");
    while (1);
  }

  Serial.println("✅ LoRa Init Success");
}

void loop() {
  Serial.println("Sending: HELLO");

  LoRa.beginPacket();
  LoRa.print("HELLO");
  LoRa.endPacket();

  delay(2000);
}
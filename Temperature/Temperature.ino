#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into GPIO 4 on the ESP32
#define ONE_WIRE_BUS 4

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup() {
  // Use 115200 for ESP32 to match the bootloader speed
  Serial.begin(115200);
  delay(1000); // Give the Serial monitor time to open
  
  Serial.println("\n-------------------------");
  Serial.println("    AquaSentinel System    ");
  Serial.println("---------------------------");
  
  sensors.begin();
  
  // Check if any sensors are actually connected on the bus
  int deviceCount = sensors.getDeviceCount();
  Serial.print("Sensors found on bus: ");
  Serial.println(deviceCount);
  
  if (deviceCount == 0) {
    Serial.println("WARNING: No DS18B20 sensor detected! Check your 4.7k resistor.");
  }
}

void loop() {
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("AquaSentinel - Temp: ");
  
  // -127.00 is the error code for "Sensor Not Found"
  // 85.00 is the error code for "Power Issue/No Pull-up"
  if(tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Sensor Disconnected!");
  } else if (tempC == 85.00) {
    Serial.println("Error: Power/Pull-up Resistor Issue (Reading 85°C)");
  } else {
    Serial.print(tempC);
    Serial.println(" °C");
  }
  
  delay(500); // 2-second interval is healthy for the DS18B20
}
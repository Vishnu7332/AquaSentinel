#define TdsSensorPin 34
#define VREF 3.3              // ESP32 ADC logic voltage
#define ADC_RES 4096.0        // ESP32 12-bit Resolution

void setup() {
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
  Serial.println("---------------------------------------");
  Serial.println("   AquaSentinel: TDS & Conductivity    ");
  Serial.println("---------------------------------------");
}

void loop() {
  // 1. Read the Analog Value with Averaging
  float rawSum = 0;
  for (int i = 0; i < 10; i++) {
    rawSum += analogRead(TdsSensorPin);
    delay(10);
  }
  float averageADC = rawSum / 10.0;

  // 2. Convert ADC to Voltage
  float voltage = averageADC * VREF / ADC_RES;

  // 3. Convert Voltage to TDS (ppm)
  // This polynomial is the standard curve for these analog sensors
  float tdsValue = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;

  // 4. Convert TDS to Conductivity (EC) in uS/cm
  // Since TDS = EC * 0.5, then EC = TDS / 0.5
  float ecValue = tdsValue / 0.5;

  // 5. Output to Serial Monitor
  Serial.print("Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | ");
  
  Serial.print("TDS: ");
  Serial.print(tdsValue, 0);
  Serial.print(" ppm | ");
  
  Serial.print("EC: ");
  Serial.print(ecValue, 0);
  Serial.println(" uS/cm");

  delay(1000); 
}
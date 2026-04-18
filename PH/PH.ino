#define PH_PIN 34      // Analog pin connected to pH sensor
float voltage = 0.0;
float phValue = 0.0;

void setup()
{
  Serial.begin(115200);
  pinMode(PH_PIN, INPUT);
}

void loop()
{
  int sensorValue = analogRead(PH_PIN);   // Read analog value

  voltage = sensorValue * (3.3 / 4095.0); // Convert to voltage (ESP32 ADC)

  // Convert voltage to pH value
  // Formula depends on sensor calibration
  phValue = 7 + ((2.5 - voltage) / 0.18);

  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V  |  pH Value: ");
  Serial.println(phValue);

  delay(200); // wait 2 seconds
}
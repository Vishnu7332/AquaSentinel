/* * AquaSentinel Sensor Calibration Utility
 * Calibrates MQ-4 (Methane) and MQ-135 (Air Quality)
 * Target: ESP32 (3.3V Logic)
 */

// --- Pin Definitions ---
#define MQ4_PIN 33    // Analog pin for MQ-4
#define MQ135_PIN 39  // Analog pin for MQ-135 (Input Only Pin)

// --- Calibration Constants ---
const float RL_BOARD = 1.0;       // Load Resistor on most modules is 1kOhm (check your board)
const float CLEAN_AIR_RATIO_MQ4 = 4.4;   // RS/R0 ratio for MQ-4 in clean air
const float CLEAN_AIR_RATIO_MQ135 = 3.6; // RS/R0 ratio for MQ-135 in clean air

// --- Variables ---
float mq4_R0 = 0;
float mq135_R0 = 0;

void setup() {
  Serial.begin(115200);
  
  // ESP32 ADC setup
  analogSetAttenuation(ADC_11db); // Range 0 - 3.3V
  
  Serial.println("\n--- MQ SENSOR CALIBRATION STARTED ---");
  Serial.println("Warming up sensors... (Wait 30 seconds)");
  
  // Brief warmup - In a real scenario, wait 2-5 minutes
  delay(30000); 

  Serial.println("Calibrating R0 values...");
  
  mq4_R0 = calibrateSensor(MQ4_PIN, CLEAN_AIR_RATIO_MQ4);
  mq135_R0 = calibrateSensor(MQ135_PIN, CLEAN_AIR_RATIO_MQ135);

  Serial.println("\n--- CALIBRATION COMPLETE ---");
  Serial.print("MQ-4 R0 Value   : "); Serial.println(mq4_R0);
  Serial.print("MQ-135 R0 Value : "); Serial.println(mq135_R0);
  Serial.println("---------------------------------------");
  Serial.println("Copy these R0 values into your main AquaSentinel code.");
}

void loop() {
  // Read raw resistance (Rs) in real-time to check stability
  float mq4_Rs = readRs(MQ4_PIN);
  float mq135_Rs = readRs(MQ135_PIN);

  Serial.print("Current Rs (MQ4): "); Serial.print(mq4_Rs);
  Serial.print(" | Current Rs (MQ135): "); Serial.println(mq135_Rs);
  
  delay(2000);
}

// Function to calculate Rs (Sensor Resistance)
float readRs(int pin) {
  int raw = analogRead(pin);
  float volt = (raw / 4095.0) * 3.3; // Convert to voltage
  
  // Formula: Rs = ((Vcc / Vout) - 1) * RL
  // Since we use a divider, we assume Vout is scaled correctly
  if (volt == 0) return 0;
  float rs = ((3.3 / volt) - 1.0) * RL_BOARD; 
  return rs;
}

// Function to find R0 (Base Resistance in Clean Air)
float calibrateSensor(int pin, float ratio) {
  float sensor_rs_sum = 0;
  int samples = 50;

  for (int i = 0; i < samples; i++) {
    sensor_rs_sum += readRs(pin);
    delay(100);
  }

  float avg_rs = sensor_rs_sum / (float)samples;
  float r0 = avg_rs / ratio; // R0 = Rs_clean_air / Clean_Air_Ratio
  return r0;
}
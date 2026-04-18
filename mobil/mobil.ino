#include <Stepper.h>

// --- BO Motor Pins ---
#define ENA 9   
#define IN1 2   
#define IN2 3   
#define IN3 4   
#define IN4 5   
#define ENB 10  

// --- Stepper Pins ---
const int stepsPerRev = 2048; 
Stepper liftStepper(stepsPerRev, 8, 12, 11, 13); 

// --- Timing Constants (Updated to 2.5 min cycle) ---
const unsigned long TOTAL_CYCLE = 150000; // 2.5 Minutes
const unsigned long ACTIVE_TIME = 60000;  // 1 Minute mobility
const unsigned long DRIVE_STEP = 10000;   // 10s drive bursts

// --- Variables ---
unsigned long cycleStartTime = 0;
unsigned long activeStartTime = 0;
unsigned long lastPrintTime = 0;
float currentDistance = 200.0; // Default safe distance

enum SystemState { SLEEPING, RETRACTING, NAVIGATING, DEPLOY_SENSORS };
SystemState currentState = SLEEPING;

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  liftStepper.setSpeed(12);
  
  // Nano Serial used for BOTH PC debugging and ESP32 data
  Serial.begin(9600); 
  
  Serial.println("--- AquaSentinel: UART Mission Controller Initialized ---");
  stopMotors();
  cycleStartTime = millis(); 
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 1. UPDATE DISTANCE FROM SERIAL (ESP32)
  updateDistance();

  // 2. STATE MACHINE LOGIC
  switch (currentState) {
    case SLEEPING:
      if (currentMillis - cycleStartTime >= (TOTAL_CYCLE - ACTIVE_TIME)) {
        Serial.println("MISSION: Starting Active Cycle...");
        currentState = RETRACTING;
      }
      break;

    case RETRACTING:
      Serial.println("STEPPER: Retracting Sensors (CCW)...");
      runStepper(-1, 10); 
      currentState = NAVIGATING;
      activeStartTime = millis();
      break;

    case NAVIGATING:
      if (currentMillis - activeStartTime >= ACTIVE_TIME) {
        stopMotors();
        currentState = DEPLOY_SENSORS;
      } else {
        runNavigationLogic();
      }
      break;

    case DEPLOY_SENSORS:
      Serial.println("STEPPER: Deploying Sensors (CW)...");
      runStepper(1, 10); 
      stopMotors();
      cycleStartTime = millis(); 
      currentState = SLEEPING;
      Serial.println("MISSION: Cycle Complete. Sleeping for 1.5 minutes.");
      break;
  }
}

// ==========================================
// BLOCK: Navigation & Obstacle Logic
// ==========================================
void runNavigationLogic() {
  if (currentDistance < 50.0) {
    executeMultiStageAvoidance();
  } else {
    Serial.println("BO MOTOR: Path Clear. Moving Forward 10s...");
    moveForward();
    smartDelay(DRIVE_STEP); 
    stopMotors();
  }
}

void executeMultiStageAvoidance() {
  Serial.println("!!! OBSTACLE DETECTED (<20cm) !!!");
  stopMotors();
  
  // Stage 1: Right 3s
  Serial.println("BO MOTOR: Turning Right (3s)...");
  turnRight();
  smartDelay(3000);
  stopMotors();
  updateDistance(); 
  if (currentDistance >= 20.0) return;

  // Stage 2: Left 6s (180 deg)
  Serial.println("!!! Still Blocked. Turning Left (6s)...");
  turnLeft();
  smartDelay(6000);
  stopMotors();
  updateDistance();
  if (currentDistance >= 20.0) return;

  // Stage 3: Left 3s + Reverse
  Serial.println("!!! Trapped. Reversing...");
  turnLeft();
  smartDelay(3000);
  stopMotors();
  moveBackward();
  smartDelay(3000);
  stopMotors();
}

// ==========================================
// BLOCK: Communication & Helpers
// ==========================================

void updateDistance() {
  // Checks if ESP32 sent data via Serial (Pins 0/1)
  if (Serial.available() > 0) {
    float newData = Serial.parseFloat();
    if (newData > 0) {
      currentDistance = newData;
    }
  }
  
  // Print Diagnostic every 500ms
  if (millis() - lastPrintTime >= 500) {
    Serial.print("LIVE DISTANCE: "); Serial.print(currentDistance); Serial.println(" cm");
    lastPrintTime = millis();
  }
}

void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateDistance(); // Keep listening to Serial during delays
    delay(10);
  }
}

void runStepper(int dir, int seconds) {
  unsigned long start = millis();
  while (millis() - start < (seconds * 1000UL)) {
    liftStepper.step(dir * 10);
    updateDistance();
  }
  digitalWrite(8, LOW); digitalWrite(11, LOW);
  digitalWrite(12, LOW); digitalWrite(13, LOW);
}

// ==========================================
// BLOCK: Motor Movements
// ==========================================
void moveForward()  { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void moveBackward() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void turnRight()    { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void turnLeft()     { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  analogWrite(ENA, 80); analogWrite(ENB, 80); }
void stopMotors()   { analogWrite(ENA, 0); analogWrite(ENB, 0); digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); }


/*
#include <Stepper.h>

// --- BO Motor Pins (L298N) ---
#define ENA 9   
#define IN1 2   
#define IN2 3   
#define IN3 4   
#define IN4 5   
#define ENB 10  

// --- Ultrasonic Pins ---
#define TRIG_PIN 6
#define ECHO_PIN 7

// --- Stepper Pins (ULN2003) ---
const int stepsPerRev = 2048; 
Stepper liftStepper(stepsPerRev, 8, 12, 11, 13); 

// --- Timing Constants ---
const unsigned long TOTAL_CYCLE = 150000; // 2.5 Minutes
const unsigned long ACTIVE_TIME = 60000;  // 1 Minute mobility
const unsigned long DRIVE_STEP = 10000;   // 10s drive bursts

// --- Variables ---
unsigned long cycleStartTime = 0;
unsigned long activeStartTime = 0;
unsigned long lastPrintTime = 0;
float currentDistance = 200.0; 

enum SystemState { SLEEPING, RETRACTING, NAVIGATING, DEPLOY_SENSORS };
SystemState currentState = SLEEPING;

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  
  // Ultrasonic Pin Setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  liftStepper.setSpeed(12);
  Serial.begin(115200); 
  
  Serial.println("--- AquaSentinel: Direct Ultrasonic Mission Controller ---");
  stopMotors();
  cycleStartTime = millis(); 
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 1. UPDATE DISTANCE DIRECTLY FROM SENSOR
  updateDistance();

  // 2. STATE MACHINE LOGIC
  switch (currentState) {
    case SLEEPING:
      if (currentMillis - cycleStartTime >= (TOTAL_CYCLE - ACTIVE_TIME)) {
        Serial.println("MISSION: Starting Active Cycle...");
        currentState = RETRACTING;
      }
      break;

    case RETRACTING:
      Serial.println("STEPPER: Retracting Sensors (CCW)...");
      runStepper(-1, 10); 
      currentState = NAVIGATING;
      activeStartTime = millis();
      break;

    case NAVIGATING:
      if (currentMillis - activeStartTime >= ACTIVE_TIME) {
        stopMotors();
        currentState = DEPLOY_SENSORS;
      } else {
        runNavigationLogic();
      }
      break;

    case DEPLOY_SENSORS:
      Serial.println("STEPPER: Deploying Sensors (CW)...");
      runStepper(1, 10); 
      stopMotors();
      cycleStartTime = millis(); 
      currentState = SLEEPING;
      Serial.println("MISSION: Cycle Complete. Sleeping for 1.5 minutes.");
      break;
  }
}

// ==========================================
// BLOCK: Navigation & Obstacle Logic
// ==========================================
void runNavigationLogic() {
  if (currentDistance < 20.0) {
    executeMultiStageAvoidance();
  } else {
    Serial.println("BO MOTOR: Path Clear. Moving Forward 10s...");
    moveForward();
    smartDelay(DRIVE_STEP); 
    stopMotors();
  }
}

void executeMultiStageAvoidance() {
  Serial.println("!!! OBSTACLE DETECTED (<20cm) !!!");
  stopMotors();
  
  // Stage 1: Right 3s
  Serial.println("BO MOTOR: Turning Right (3s)...");
  turnRight();
  smartDelay(3000);
  stopMotors();
  updateDistance(); 
  if (currentDistance >= 20.0) return;

  // Stage 2: Left 6s (180 deg)
  Serial.println("!!! Still Blocked. Turning Left (6s)...");
  turnLeft();
  smartDelay(6000);
  stopMotors();
  updateDistance();
  if (currentDistance >= 20.0) return;

  // Stage 3: Left 3s + Reverse
  Serial.println("!!! Trapped. Reversing...");
  turnLeft();
  smartDelay(3000);
  stopMotors();
  moveBackward();
  smartDelay(3000);
  stopMotors();
}

// ==========================================
// BLOCK: Sensor & Helpers
// ==========================================

void updateDistance() {
    // --- DIRECT ULTRASONIC CODE ---
    digitalWrite(TRIG_PIN, LOW); 
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 40000);
    float dist = (duration * 0.0343 / 2);

    if (dist > 0) {
       currentDistance = dist;
    }
  
  // Print Diagnostic every 500ms
  if (millis() - lastPrintTime >= 500) {
    Serial.print("LIVE DISTANCE: "); Serial.print(currentDistance); Serial.println(" cm");
    lastPrintTime = millis();
  }
}

void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateDistance(); // Keep checking distance during moves
    delay(10);
  }
}

void runStepper(int dir, int seconds) {
  unsigned long start = millis();
  while (millis() - start < (seconds * 1000UL)) {
    liftStepper.step(dir * 10);
    updateDistance();
  }
  digitalWrite(8, LOW); digitalWrite(11, LOW);
  digitalWrite(12, LOW); digitalWrite(13, LOW);
}

// ==========================================
// BLOCK: Motor Movements
// ==========================================
void moveForward()  { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void moveBackward() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void turnRight()    { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); analogWrite(ENA, 80); analogWrite(ENB, 80); }
void turnLeft()     { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  analogWrite(ENA, 80); analogWrite(ENB, 80); }
void stopMotors()   { analogWrite(ENA, 0); analogWrite(ENB, 0); digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); }


*/
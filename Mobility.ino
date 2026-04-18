#include <Stepper.h>

// --- BO Motor Pins ---
#define ENA 9   
#define IN1 2   
#define IN2 3   
#define IN3 4   
#define IN4 5   
#define ENB 10  

// --- Stepper ---
const int stepsPerRev = 2048; 
Stepper liftStepper(stepsPerRev, 8, 12, 11, 13); 

// --- Timing ---
const unsigned long TOTAL_CYCLE = 150000;
const unsigned long ACTIVE_TIME = 60000;
const unsigned long DRIVE_STEP = 10000;

// --- Variables ---
unsigned long cycleStartTime = 0;
unsigned long activeStartTime = 0;
unsigned long lastPrintTime = 0;

float currentDistance = 200.0;
int obstacleStage = 0;

// --- States ---
enum SystemState { SLEEPING, RETRACTING, NAVIGATING, DEPLOY_SENSORS };
SystemState currentState = SLEEPING;

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  liftStepper.setSpeed(10); // reliable speed

  Serial.begin(9600);

  stopMotors();
  cycleStartTime = millis();
}

void loop() {

  updateDistance();

  switch (currentState) {

    case SLEEPING:
      if (millis() - cycleStartTime >= (TOTAL_CYCLE - ACTIVE_TIME)) {
        Serial.println("MISSION START");
        currentState = RETRACTING;
      }
      break;

    case RETRACTING:
      Serial.println("Retracting Sensors");
      runStepper(-1, 2);   // 2 rotations
      currentState = NAVIGATING;
      activeStartTime = millis();
      break;

    case NAVIGATING:
      if (millis() - activeStartTime >= ACTIVE_TIME) {
        stopMotors();
        currentState = DEPLOY_SENSORS;
      } else {
        runNavigationLogic();
      }
      break;

    case DEPLOY_SENSORS:
      Serial.println("Deploying Sensors");
      runStepper(1, 2);
      stopMotors();
      cycleStartTime = millis();
      currentState = SLEEPING;
      Serial.println("Cycle Complete");
      break;
  }
}

// ==========================================
// 🚗 NAVIGATION
// ==========================================

void runNavigationLogic() {

  if (currentDistance < 20.0) {
    handleObstacle();
  } 
  else {
    obstacleStage = 0;

    Serial.println("Moving Forward");

    // LEFT CW, RIGHT ACW (due to mounting)
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // LEFT CW
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // RIGHT ACW

    analogWrite(ENA, 80);
    analogWrite(ENB, 80);

    smartDelay(DRIVE_STEP);
    stopMotors();
  }
}

// ==========================================
// 🚧 OBSTACLE HANDLING
// ==========================================

void handleObstacle() {

  stopMotors();

  if (obstacleStage == 0) {
    Serial.println("Stage 1: Turn Right (90)");

    // LEFT motor HARD STOP
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 0);

    // RIGHT motor ACW
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 80);

    smartDelay(3000);
  }

  else if (obstacleStage == 1) {
    Serial.println("Stage 2: Turn Left (180)");

    // RIGHT motor HARD STOP
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 0);

    // LEFT motor CW
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 80);

    smartDelay(6000);
  }

  else if (obstacleStage == 2) {
    Serial.println("Stage 3: Turn Left (90)");

    // RIGHT motor HARD STOP
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 0);

    // LEFT motor CW
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 80);

    smartDelay(3000);
  }

  else {
    // 🚨 TRAPPED CONDITION
    Serial.println("TRAPPED → Moving Backward");

    // BACKWARD (reverse of forward)
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // LEFT ACW
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // RIGHT CW

    analogWrite(ENA, 80);
    analogWrite(ENB, 80);

    smartDelay(3000);

    obstacleStage = 0;
    return;
  }

  stopMotors();
  obstacleStage++;
}

// ==========================================
// 📡 DISTANCE UPDATE
// ==========================================

void updateDistance() {
  if (Serial.available() > 0) {
    float val = Serial.parseFloat();
    if (val > 0) currentDistance = val;
  }

  if (millis() - lastPrintTime >= 500) {
    Serial.print("Distance: ");
    Serial.println(currentDistance);
    lastPrintTime = millis();
  }
}

// ==========================================
// ⏱ SMART DELAY
// ==========================================

void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateDistance();
    delay(10);
  }
}

// ==========================================
// ⚙️ STEPPER
// ==========================================

void runStepper(int dir, int rotations) {
  int steps = rotations * stepsPerRev;
  liftStepper.step(dir * steps);

  digitalWrite(8, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}

// ==========================================
// 🛑 STOP (HARD BRAKE)
// ==========================================

void stopMotors() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
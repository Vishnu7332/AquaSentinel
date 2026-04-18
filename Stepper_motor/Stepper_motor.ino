#include <Stepper.h>

// 2048 steps for one full 360-degree rotation
const int stepsPerRevolution = 2048;  


// Sequence for ULN2003: IN1, IN3, IN2, IN4
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

void setup() {
  myStepper.setSpeed(12); // Speed in RPM
  Serial.begin(9600);
 
}

void loop() {
  // 1. Move 90 degrees Anti-Clockwise

  myStepper.step(-stepsPerRevolution); 
  delay(10000); // Wait 1 second

  myStepper.step(stepsPerRevolution); 
  delay(10000); // Wait 1 second
}
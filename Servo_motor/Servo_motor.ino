#include <Servo.h>

Servo myRudder;

void setup() {
  myRudder.attach(9); // Signal wire on D9
}

void loop() {
  myRudder.write(0);   // Move to 0 degrees
  delay(1000);
  myRudder.write(90);  // Move to center (90 degrees)
  delay(1000);
  myRudder.write(180); // Move to 180 degrees
  delay(1000);
}
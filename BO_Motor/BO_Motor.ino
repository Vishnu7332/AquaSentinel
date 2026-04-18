// L298N Control Pins
#define ENA 9   
#define IN1 2   
#define IN2 3   
#define IN3 4   
#define IN4 5   
#define ENB 10  

// REDUCED SPEED: Set to 45 (approx 17-18%) for stability
// If the motor only hums and doesn't spin, increase this to 55-60.
int motorSpeed = 45; 
int currentCommand = 0; 
int lastCommand = -1;   

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  
  Serial.begin(115200); 
  Serial.println("AquaSentinel: Low-Speed Stability Mode Active.");
  Serial.println("Commands: 0:Stop, 1:Fwd, 2:Bwd, 3:Right, 4:Left");
  
  stopMotors(); 
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    // Check if input is a valid command digit
    if (input >= '0' && input <= '4') {
      currentCommand = input - '0'; 
      Serial.print("Executing: ");
      printCommandName(currentCommand);
    }
  }

  // Update motors ONLY on state change to keep the signal smooth
  if (currentCommand != lastCommand) {
    executeMovement(currentCommand);
    lastCommand = currentCommand;
  }
}

void executeMovement(int cmd) {
  switch(cmd) {
    case 1: moveForward();  break;
    case 2: moveBackward(); break;
    case 3: turnRight();    break;
    case 4: turnLeft();     break;
    case 0: stopMotors();   break;
    default: stopMotors();  break;
  }
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnRight() {
  // Pivot: Left FRONT, Right BACK
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnLeft() {
  // Pivot: Left BACK, Right FRONT
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  // Physical cut-off for safety
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void printCommandName(int cmd) {
  switch(cmd) {
    case 0: Serial.println("STOP"); break;
    case 1: Serial.println("FORWARD"); break;
    case 2: Serial.println("BACKWARD"); break;
    case 3: Serial.println("RIGHT PIVOT"); break;
    case 4: Serial.println("LEFT PIVOT"); break;
  }
}
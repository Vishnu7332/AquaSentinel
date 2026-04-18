// L298N Control Pins
#define ENA 9   
#define IN1 2   
#define IN2 3   
#define IN3 4   
#define IN4 5   
#define ENB 10  

// SAFETY LIMIT: Change this based on your motor's heat levels
// 100 is roughly 40% of 11.1V (~4.5V average), which is safe for toy motors.
const int MAX_SAFE_SPEED = 100; 

int targetSpeed = 45;   // Initial startup speed
int currentPWM = 0;     // For smooth ramping
int currentCommand = 0; 
int lastCommand = -1;   

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  
  Serial.begin(115200); 
  Serial.println("--- AquaSentinel Safety System ---");
  Serial.print("Hard Speed Limit set to: "); Serial.println(MAX_SAFE_SPEED);
  Serial.println("Commands: 0:Stop, 1:Fwd, 2:Bwd, 3:Right, 4:Left");
  Serial.println("Type 'speed:value' to change (Max 100)");
  
  stopMotors(); 
}

void loop() {
  // 1. Handle Serial Input
  if (Serial.available() > 0) {
    String inputStr = Serial.readStringUntil('\n');
    inputStr.trim();

    if (inputStr.startsWith("speed:")) {
      int newSpeed = inputStr.substring(6).toInt();
      
      // APPLY SAFETY LIMIT HERE
      if (newSpeed > MAX_SAFE_SPEED) {
        Serial.print("! WARNING: Speed limited to ");
        Serial.println(MAX_SAFE_SPEED);
        targetSpeed = MAX_SAFE_SPEED;
      } else {
        targetSpeed = constrain(newSpeed, 0, MAX_SAFE_SPEED);
        Serial.print("Target Speed: "); Serial.println(targetSpeed);
      }
    } 
    else if (inputStr.length() == 1) {
      char cmdChar = inputStr.charAt(0);
      if (cmdChar >= '0' && cmdChar <= '4') {
        currentCommand = cmdChar - '0';
        Serial.print("Action: "); printCommandName(currentCommand);
      }
    }
  }

  // 2. Continuous Ramping Logic
  updateRamping();

  // 3. Update Direction Pins
  if (currentCommand != lastCommand) {
    setDirection(currentCommand);
    lastCommand = currentCommand;
  }
}

void updateRamping() {
  int goal = (currentCommand == 0) ? 0 : targetSpeed;

  // Soft-start / Soft-stop adjustment
  if (currentPWM < goal) {
    currentPWM++; 
    delay(10); // Increase for slower, gentler acceleration
  } else if (currentPWM > goal) {
    currentPWM--; 
    delay(5);  // Speed of deceleration
  }

  analogWrite(ENA, currentPWM);
  analogWrite(ENB, currentPWM);
}

void setDirection(int cmd) {
  switch(cmd) {
    case 1: // Forward
      digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
      break;
    case 2: // Backward
      digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
      break;
    case 3: // Right Pivot
      digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
      break;
    case 4: // Left Pivot
      digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
      break;
    case 0: // Stop
      digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
      break;
  }
}

void stopMotors() {
  currentCommand = 0;
  currentPWM = 0;
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void printCommandName(int cmd) {
  switch(cmd) {
    case 0: Serial.println("STOP"); break;
    case 1: Serial.println("FORWARD"); break;
    case 2: Serial.println("BACKWARD"); break;
    case 3: Serial.println("RIGHT"); break;
    case 4: Serial.println("LEFT"); break;
  }
}
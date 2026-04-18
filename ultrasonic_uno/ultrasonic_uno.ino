// Define the pins
const int trigPin = 9;
const int echoPin = 10;

// Variables for duration and distance
long duration;
int distance;

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);
  
  // Set the pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // Clear the trigPin by setting it LOW for 2 microseconds
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trigger the sensor by setting the trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin; pulseIn returns the travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance (Speed of sound is 0.034 cm/us)
  // We divide by 2 because the sound travels to the object and back
  distance = duration * 0.034 / 2;

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Wait 500ms before the next reading
  delay(500);
}
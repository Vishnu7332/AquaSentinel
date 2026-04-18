int ph;
float volt;

void setup() {
  Serial.begin(9600);
  pinMode(ph,INPUT);

}

void loop() {
  ph = analogRead(34);
  volt = ph*(3.3/4095.0) ;
  Serial.print("Voltage: ");
  Serial.println(volt) ;
  delay(1000);
}

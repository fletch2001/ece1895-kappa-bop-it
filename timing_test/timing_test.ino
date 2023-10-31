void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  long time_start = millis();

  digitalWrite(6, LOW);
  digitalWrite(8, HIGH);

  while(time_start + millis() < 1000);
  digitalWrite(6, HIGH);

  while(1); // keep alive
}

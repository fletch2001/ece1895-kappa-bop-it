void setup() {
  // set up serial output to test code
  Serial.begin(9600);
  bool isGameRunning = false;
}

void loop() {
  // constantly poll for game to be started
  if (!isGameRunning)
  else {
    // seed random number with time
    // pick random command
    int command = random(3);

    // first command (twist it)
    if (command == 0) {
      Serial.println("Twist It!");
    }
  }
  Serial.println(command);
}

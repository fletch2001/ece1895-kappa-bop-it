#include <time.h>

#include 


#define START_BUTTON 4
#define LED1 6
#define LED2 7
#define LED3 8

enum commands {TwistIt, PourIt, RipIt};

int rand_seed_counter;

void setup() {
  // set up serial output to test code
  Serial.begin(9600);

  rand_seed_counter = 0;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // start button
  pinMode(START_BUTTON, INPUT);

}

void wait_for_user_response(uint8_t input_pin, uint8_t signal_to_wait_for, uint8_t output_pin) {
  digitalWrite(output_pin, HIGH);
  while((digitalRead(input_pin) != signal_to_wait_for));
  while(digitalRead(input_pin) == signal_to_wait_for); // wait for signal to reset

  digitalWrite(output_pin, LOW);

}

void loop() {

  // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
  // because we don't have an RTC to keep track of time.
  while(digitalRead(START_BUTTON) == HIGH) {
    rand_seed_counter++;
  }

  // seed random numbers
    srand(rand_seed_counter);

    // initialize input time and score
    float inputTime = 2;
    int score = 0;

    // loop for the game
    bool isRunning = true;
    if (!isRunning) // game has not started (ie. button needs to be pressed)
        printf("game is not running");
    else { // game is running
        while (true) {
            int command = rand() % 3;
            printf("Score: %d\n", score);

            // twist it
            if (command == TwistIt) {
                //printf("Twist It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED1);

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }
            // pour it
            else if (command == PourIt) {
                //printf("Pour It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED2);

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }

            // rip it
            else if (command == RipIt) {
                //printf("Rip It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED3);

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }
        }
    }
}

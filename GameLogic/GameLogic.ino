#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include 


#define START_BUTTON 4
#define LED1 6
#define LED2 7
#define LED3 8

#define WIDTH 128
#define HEIGHT 64

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);

enum commands {TwistIt, PourIt, RipIt};

// initialize input time and score
float inputTime = 2;
int score = 0;

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

  // setup OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while(true);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void wait_for_user_response(uint8_t input_pin, uint8_t signal_to_wait_for, uint8_t output_pin, String command) {  
  // write command to OLED
  display.clearDisplay();
  display.setCursor(10, 10);
  display.println("score = " + String(score));
  display.println(command);
  display.display();

  // wait for input to go to desired and then back to normal state
  int timeStart = millis();
  int timeAction;
  while(digitalRead(input_pin) != signal_to_wait_for) {
    // constantly poll time elapsed
    timeAction = millis();

    // if time elapsed is longer than current input time allowed, game over!
    if (timeAction - timeStart > inputTime*1000) {
      display.clearDisplay();
      display.println("score = " + String(score));
      display.println("GAME OVER!");
      display.display();
      exit(0);
    }
  }
  while(digitalRead(input_pin) == signal_to_wait_for); // wait for signal to reset
}

void loop() {

  // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
  // because we don't have an RTC to keep track of time.
  while(digitalRead(START_BUTTON) == HIGH) {
    display.setCursor(0, 10);
    display.print("waiting...");
    display.display();
    display.clearDisplay();
    rand_seed_counter++;
  }

  display.clearDisplay();

  // seed random numbers
  srand(rand_seed_counter);

  // loop for the game
  bool isRunning = true;
  if (!isRunning) // game has not started (ie. button needs to be pressed)
      printf("game is not running");
  else { // game is running
      while (true) {
        int command = rand() % 3;
    // initialize input time and score
    float inputTime = 2;
    int score = 0;

        // twist it
        if (command == TwistIt) {
            wait_for_user_response(START_BUTTON, LOW, LED1, "Twist It!");

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }
        // pour it
        else if (command == PourIt) {
            //printf("Pour It!\n");
            wait_for_user_response(START_BUTTON, LOW, LED2, "Pour It!");

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }

        // rip it
        else if (command == RipIt) {
            //printf("Rip It!\n");
            wait_for_user_response(START_BUTTON, LOW, LED3, "Rip It!");

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }
      }
  }
}

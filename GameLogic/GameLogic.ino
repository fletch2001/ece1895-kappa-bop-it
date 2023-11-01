#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
}

void wait_for_user_response(uint8_t input_pin, uint8_t signal_to_wait_for, uint8_t output_pin, String command) {
  //digitalWrite(output_pin, HIGH);
  
  // write command to OLED
  display.clearDisplay();
  display.println(score);
  display.print(command);
  display.display();

  while(digitalRead(input_pin) != signal_to_wait_for);
  while(digitalRead(input_pin) == signal_to_wait_for); // wait for signal to reset

  //digitalWrite(output_pin, LOW);

}

void loop() {

  // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
  // because we don't have an RTC to keep track of time.
  while(digitalRead(START_BUTTON) == HIGH) {
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

            // twist it
            if (command == TwistIt) {
                //printf("Twist It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED1, "Twist It!");

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }
            // pour it
            else if (command == PourIt) {
                //printf("Pour It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED2, "Pour It!");

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }

            // rip it
            else if (command == RipIt) {
                //printf("Rip It!\n");
                wait_for_user_response(START_BUTTON, LOW, LED3, "Rip It!");

                // poll for user input
                delay(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime -= 0.02;
                score++;
            }
        }
    }
}

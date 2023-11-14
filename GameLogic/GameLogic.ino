#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MPU6050.h"


#define START_BUTTON 4
#define LED1 6
#define LED2 7
#define LED3 8

#define TEST_BUTTON 15

#define TWIST_IT 1
#define RIP_IT 2
#define POUR_IT 4

#define WIDTH 128
#define HEIGHT 64

#define UPRIGHT_DIRECTION Z

String commands_list[] = {"Twist it!", "Rip it!", "Pour it!"};

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
MPU6050 accelgyro; // IMU object

enum commands {TwistIt, PourIt, RipIt};

// initialize input time and score
float inputTime = 2;
int score = 0;

int rand_seed_counter;


int16_t ax, ay, az;
int16_t prev_ax, prev_ay, prev_az;

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
  pinMode(TEST_BUTTON, INPUT);

  // setup OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while(true);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);

  accelgyro.initialize(); // init accelgyro


}

void display_command_and_score_to_oled(String command) {
  // write command to OLED
  display.clearDisplay();
  display.setCursor(10, 10);
  display.println("score = " + String(score));
  display.println(command);
  display.display();
}

// the 1,2,4 method - no combination of them equals another and then we can distinguish between outputs passed back.
int poll_twist_it() {
  if(digitalRead(TEST_BUTTON) == 0) return 1;
  else return 0;
}

int poll_rip_it() {
  if(digitalRead(TEST_BUTTON) == 0) return 2;
  else return 0;
}

int poll_pour_it() {
  accelgyro.getAcceleration(&ax, &ay, &az);

    // check not upright
    if(az < ax && az < ay) {
      display.setCursor(0, 100);
      display.println("not upright");
      display.display();
      return 4;
    } else {
      return 0;
    }
}

int poll_sensors() {
  return poll_pour_it() + poll_rip_it() + poll_twist_it();
}

void wait_for_user_response(int command) {  
  
  display_command_and_score_to_oled(commands_list[command >> 1]);

  // accelgyro.getAcceleration(&prev_ax, &prev_ay, &prev_az);

  int timeStart = millis();
  int sensor_sum = poll_sensors();

  // wait for input to go to desired and then back to normal state
  
  int timeAction = timeStart;
  while(!sensor_sum) {
    // constantly poll time elapsed
    timeAction = millis();

    // if time elapsed is longer than current input time allowed, game over!
    if ((timeAction - timeStart) > inputTime*1000) {
      display.clearDisplay();
      display.println("score = " + String(score));
      display.println("GAME OVER!");
      display.display();
      exit(0);
    }
    sensor_sum = poll_sensors();
  }
  if(sensor_sum != command) {
    display.clearDisplay();
      display.println("score = " + String(score));
      display.println("GAME OVER!");
      display.display();
      exit(0);
  }
  
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
  if (!isRunning) {// game has not started (ie. button needs to be pressed)
      display.setCursor(0, 0);
      display.print("game is not running");
      display.display();
  } else { // game is running
      while (true) {
        int command = rand() % 3;
    // initialize input time and score
    float inputTime = 2;
    int score = 0;

        // twist it
        if (command == TwistIt) {
            wait_for_user_response(TWIST_IT);

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }
        // pour it
        else if (command == PourIt) {
            //printf("Pour It!\n");



            wait_for_user_response(POUR_IT);

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }

        // rip it
        else if (command == RipIt) {
            //printf("Rip It!\n");
            wait_for_user_response(RIP_IT);

            // poll for user input
            delay(1000);

            // adjust inputTime to be faster for next instruction
            inputTime -= 0.02;
            score++;
        }
      }
  }
}

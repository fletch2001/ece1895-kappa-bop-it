// includes for libraries used
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <avr/io.h>
#include <math.h>


//#define DEBUG 1

#include "MPU6050.h"
#define MPU_ADDR 0x68

// defines for pins for inputs
#define START_BUTTON 5
#define RIP_IT_SLIDE_POT A2
#define TWIST_IT_ROT_POT 17

// define for speaker output
#define SPEAKER 9

// defines for input commands
#define TWIST_IT 0
#define RIP_IT 1
#define POUR_IT 2

#define A_MIN 265
#define A_MAX 401

#define hold()                                \
    while (digitalRead(START_BUTTON) == LOW)  \
        ;                                     \
    while (digitalRead(START_BUTTON) == HIGH) \
        ;

String commands_list[] = {"Twist it!", "Rip it!", "Pour it!"};
enum commands { TwistIt,
                RipIt,
                PourIt };

int command_sum[] = {1, 2, 4};

int current_command;

// defines for OLED display
#define WIDTH 128
#define HEIGHT 64

// defines for accelerometer
#define UPRIGHT_DIRECTION Z
#define POUR_IT_TOLERANCE 1
#define POUR_IT_ANGLE 45

// global vars for previous potentiometer inputs
int PREV_TWIST_IT;
int PREV_RIP_IT;
#define TOLERANCE 75

// initialize OLED display and IMU
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
MPU6050 accelgyro;
int16_t ax, ay, az;

int score;
float inputTime;
int rand_seed_counter;

int oldTwistVal;
int oldRipVal;

double X_ANG;
double Y_ANG;
double Z_ANG;

// initialize file for sd card
// File sdCard;
// TMRpcm tmrpcm;

void setup() {
    rand_seed_counter = 0;

    // start button
    pinMode(START_BUTTON, INPUT);

    // initialize accelgyro
    //accelgyro.initialize();
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    // initialize current potentiometer values as the baseline
    PREV_TWIST_IT = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    PREV_RIP_IT = map(analogRead(RIP_IT_SLIDE_POT), 0, 1023, 0, 179);

    // if OLED setup fails, program will hang
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        while (true)
            ;

    // set default OLED writing settings
    display.setTextSize(1);
    display.setTextColor(WHITE);

    // setup speaker
    pinMode(SPEAKER, OUTPUT);
}

void wait(uint16_t ms) {
    int time_start = millis();
    while (millis() - time_start < ms);
}

// function to write score to OLED display
void display_score_to_oled() {
    // set cursor to first line and write score
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("score: " + String(score));
    display.display();
}

// function to display command to OLED display
void display_command_to_oled() {
    // set cursor to second line and write command
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.print(commands_list[current_command]);
    display.display();
}

// wrapper function to write score and command
void display_command_and_score_to_oled() {
    display.clearDisplay();
    display_score_to_oled();
    display_command_to_oled();
}

void get_angle() {
  int xAng = map(ax, A_MIN, A_MAX, -90, 90);
  int yAng = map(ay, A_MIN, A_MAX, -90, 90);
  int zAng = map(az, A_MIN, A_MAX, -90, 90);

  X_ANG = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  Y_ANG = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  Z_ANG = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
}

// functions to poll the sensors
// the 1,2,4 method - no combination of them equals another and then we can distinguish between outputs passed back.
int poll_twist_it() {
    int twistVal = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);

    // find difference between current val and prev val
    int diff = abs(twistVal - PREV_TWIST_IT);

    // if difference is greater than tolerance, store old val and return correct
    if (diff > 100) {
        PREV_TWIST_IT = twistVal;
        return 1;
    }
    return 0;
}

int poll_rip_it() {
    int ripVal = map(analogRead(RIP_IT_SLIDE_POT), 0, 1023, 0, 179);

    // find difference between current val and prev val
    int diff = abs(ripVal - PREV_RIP_IT);

    // if difference is greater than tolerance, store old val and return correct
    if (diff > TOLERANCE) {
        PREV_RIP_IT = ripVal;
        return 2;
    }
    return 0;
}

int poll_pour_it() {
    //accelgyro.getAcceleration(&ax, &ay, &az);
    // check not upright

    // from how2electronics.com/measure-tilt-angle-mpu6050-arduino/
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);
    ax = Wire.read()<<8|Wire.read();
    ay = Wire.read()<<8|Wire.read();
    az = Wire.read()<<8|Wire.read();
    Wire.endTransmission(true);

    get_angle();

    if(abs(Z_ANG) > POUR_IT_ANGLE) {
      return 4;
    } else {
      return 0;
    }
}

int poll_sensors() {
    return poll_rip_it() + poll_twist_it();
    poll_pour_it();
}

// function to poll sensors and check if the user responded within the time limit
void wait_for_user_response(int command) {
    // set current command display it
    current_command = command;
    display_command_and_score_to_oled();

    // PREV_TWIST_IT = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    PREV_TWIST_IT = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    PREV_RIP_IT = map(analogRead(RIP_IT_SLIDE_POT), 0, 1023, 0, 179);

    // initialize the start time of the command being sent
    int timeStart = millis();
    int sensor_sum = poll_sensors();

    // wait for input to go to desired and then back to normal state
    int timeAction = timeStart;

    // keep looping while time limit hasn't been reached or while no inputs have been selected
    while ((sensor_sum == 0) && ((timeAction - timeStart) < inputTime * 1000)) {
        // constantly poll time elapsed
        timeAction = millis();
        sensor_sum = poll_sensors();
    }

#ifdef DEBUG
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println(String(ax) + ", " + String(ay) + ", " + String(az));
    display.println(String(X_ANG) + " " + String(X_ANG) + " " + String(X_ANG));
    display.println(String(command) + "\n" + String(command_sum[command]) + "\n" + String(sensor_sum));
    display.display();
    hold();
#endif
    
    // if time elapsed is longer than current input time allowed, game over!
    if (sensor_sum != command_sum[command]) {
        // display score and game over message
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.print("score = " + String(score) + "\n");
        display.setCursor(0, 10);
        display.setTextSize(1);
        display.print("GAME OVER!\n\n");

        display.display();

        while(digitalRead(START_BUTTON) == LOW);
        while(digitalRead(START_BUTTON) == HIGH);

        // go back to start state
        loop();
    } else {
        // if input was correct, increase score and decrease allowed input time
        score++;
        inputTime -= 0.02;
        display.clearDisplay();
        display_score_to_oled();
        delay(1000);
    }
}

void loop() {

  score = 0;
  inputTime = 3;

  // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
  // because we don't have an RTC to keep track of time.
  while(digitalRead(START_BUTTON) == LOW) {
    display.clearDisplay();
    display.setCursor((WIDTH / 2) - (String("BONK-IT!").length() / 2), 4);
    display.setTextSize(2);
    display.println("BONK-IT!");
    display.setTextSize(1);
    display.print("press lid to start");
    display.display();
    rand_seed_counter++;
  }

  // wait for button to be let-go of
  while(digitalRead(START_BUTTON) == HIGH);
  display.clearDisplay();

  // seed random numbers
  srand(rand_seed_counter);

  // loop for the game
  bool isRunning = true;
  if (!isRunning) { // game has not started (ie. button needs to be pressed)
    display.setCursor(0, 0);
    display.print("game is not running");
    display.display();
  } 
  
  // game is running - run game loop
  else {
      delay(2000);
      while (score <= 99) {
        // get a random command
        int command = rand() % 2;
        //int command = 1;

        // twist it
        if (command == TwistIt) {
          // output sound
          tone(SPEAKER, 5000, 500);

          // poll for user input
          wait_for_user_response(TWIST_IT);
          wait(1000);
        }
        // pour it
        else if (command == PourIt) {
          tone(SPEAKER, 2500, 500);

          // poll for user input
          wait_for_user_response(POUR_IT);
          wait(1000);
        }
        // rip it
        else if (command == RipIt) {
          tone(SPEAKER, 1000, 500);
          // poll for user input
          wait_for_user_response(RIP_IT);
          delay(1000);
        }
      }
  }
}
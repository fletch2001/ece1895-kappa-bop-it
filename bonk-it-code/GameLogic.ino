// includes for libraries used
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_light.h>
#include <math.h>


//#define DEBUG

#define MPU_ADDR 0x68

// defines for pins for inputs
#define START_BUTTON 5
#define RIP_IT_SLIDE_POT A2
#define TWIST_IT_ROT_POT A3

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
int RIP_IT_VAL;
int TWIST_IT_VAL;
#define TOLERANCE 75

// initialize OLED display and IMU
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
MPU6050 mpu(Wire);
int16_t ax, ay, az;

int score;
float inputTime;
int rand_seed_counter;

int oldTwistVal;
int oldRipVal;

double X_ANG;
double Y_ANG;
double Z_ANG;

int SENSOR_SUM;
int RIP_IT_DIFF;
int TWIST_IT_DIFF;
int TIMESTART;
int TIMEACTION;

// initialize file for sd card
// File sdCard;
// TMRpcm tmrpcm;

void setup() {
    rand_seed_counter = 0;

    // start button
    pinMode(START_BUTTON, INPUT);

    // initialize current potentiometer values as the baseline
    // PREV_TWIST_IT = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    // PREV_RIP_IT = map(analogRead(RIP_IT_SLIDE_POT), 0, 1023, 0, 179);

    // if OLED setup fails, program will hang
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        while (true)
            ;

    // initialize accelgyro
    if(!mpu.begin()) {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.println("MPU6050 not initialized");
      display.display();
    }

    // calibrate accelerometer
    mpu.calcGyroOffsets();


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
    display.setCursor(16, 0);
    display.setTextSize(2);
    display.print("score: " + String(score));
    display.display();
}

// function to display command to OLED display
void display_command_to_oled() {
    // set cursor to second line and write command
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(commands_list[current_command]);
    display.display();
}

// wrapper function to write score and command
void display_command_and_score_to_oled() {
    display.clearDisplay();
    display_score_to_oled();
    display_command_to_oled();
}

// functions to poll the sensors
// the 1,2,4 method - no combination of them equals another and then we can distinguish between outputs passed back.
int poll_twist_it() {
    //TWIST_IT_VAL = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    TWIST_IT_VAL = analogRead(TWIST_IT_ROT_POT);

    // find difference between current val and prev val
    int diff = abs(TWIST_IT_VAL - PREV_TWIST_IT);

    // if difference is greater than tolerance, store old val and return correct
    if (diff > 100) {
#ifdef DEBUG
    display.clearDisplay();
    display.println("Prev val" + String(PREV_TWIST_IT) + "curr: " + String(TWIST_IT_VAL));
    display.display();
    hold();
#endif
        // PREV_TWIST_IT = TWIST_IT_VAL;
        return 1;
    }
    return 0;
}

int poll_rip_it() {
    //RIP_IT_VAL = map(analogRead(RIP_IT_SLIDE_POT), 0, 1023, 0, 179);
    RIP_IT_VAL = analogRead(RIP_IT_SLIDE_POT);

    // find difference between current val and prev val
    int diff = abs(RIP_IT_VAL - PREV_RIP_IT);

    // if difference is greater than tolerance, store old val and return correct
    if (diff > TOLERANCE) {

        PREV_RIP_IT = RIP_IT_VAL;
        return 2;
    }
    return 0;
}

int poll_pour_it() {
    //accelgyro.getAcceleration(&ax, &ay, &az);
    // check not upright

    // poll MPU 
    mpu.update();

    Z_ANG = mpu.getAngleZ();
    Y_ANG = mpu.getAngleY();
    X_ANG = mpu.getAngleX();

    if(abs(X_ANG) > POUR_IT_ANGLE || abs(Y_ANG) > POUR_IT_ANGLE) {
      return 4;
    } else {
      return 0;
    }
}

int poll_sensors() {
    return poll_rip_it() + poll_twist_it() + poll_pour_it();
}

// function to poll sensors and check if the user responded within the time limit
void wait_for_user_response(int command) {
    // set current command display it
    current_command = command;
    display_command_and_score_to_oled();

    // PREV_TWIST_IT = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);
    PREV_TWIST_IT = analogRead(TWIST_IT_ROT_POT);
    PREV_RIP_IT = analogRead(RIP_IT_SLIDE_POT);


    // initialize the start time of the command being sent
    wait(500);
    SENSOR_SUM = poll_sensors();
    TIMESTART = millis();

    // wait for input to go to desired and then back to normal state
    int TIMEACTION = TIMESTART;

    // keep looping while time limit hasn't been reached or while no inputs have been selected
    while ((SENSOR_SUM == 0) && ((TIMEACTION - TIMESTART) < inputTime * 1000)) {
        // constantly poll time elapsed
        TIMEACTION = millis();
        // poll MPU values
        SENSOR_SUM = poll_sensors();
    }

#ifdef DEBUG
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println(String(analogRead(TWIST_IT_ROT_POT)));
    display.println(String(TWIST_IT_VAL) + " " + String(PREV_TWIST_IT));
    display.println(String(ax) + ", " + String(ay) + ", " + String(az));
    display.println(String(X_ANG) + " " + String(Y_ANG) + " " + String(Z_ANG));
    display.println(String(command) + "\n" + String(command_sum[command]) + "\n" + String(SENSOR_SUM));
    display.display();
    hold();
#endif
    
    // if time elapsed is longer than current input time allowed, game over!
    if (SENSOR_SUM != command_sum[command]) {
        // display score and game over message
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.println("score = " + String(score));
        display.setCursor(0, 20);
        display.setTextSize(2);
        display.print("GAME OVER!");

        display.display();
#if DEBUG
        while(digitalRead(START_BUTTON) == LOW);
        while(digitalRead(START_BUTTON) == HIGH);
#endif

        // go back to start state
        wait(2500);
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
    display.flush();
    display.clearDisplay();
    display.setCursor((WIDTH / 4) -18, 4);
    display.setTextSize(2);
    display.println("BONK-IT!");
    display.setTextSize(1);
    display.setCursor(8, 20);
    display.println("press lid to start");
#if DEBUG
    display.println("v0.9.5.3");
#endif
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
        int command = rand() % 3;

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
      // win screen
      display.clearDisplay();
      display.setCursor(4, 0);
      display.setTextSize(2);
      display.println("You win!");
      display.display();
      wait(5000);
      // go back to start
      loop();
  }
}
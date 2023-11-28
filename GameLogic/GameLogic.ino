// includes for libraries used
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <avr/io.h>
#include <math.h>


#define DEBUG 1

#include "MPU6050.h"

#define F_CPU 8000000UL

#define SD_CS 16

// defines for pins for inputs
#define START_BUTTON 5
#define RIP_IT_SLIDE_POT 19
#define TWIST_IT_ROT_POT 17

// define for speaker output
#define SPEAKER 15

// defines for input commands
#define TWIST_IT 0
#define RIP_IT 1
#define POUR_IT 2

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

// global vars for previous potentiometer inputs
int PREV_TWIST_IT;
int PREV_RIP_IT;
#define TOLERANCE 10

// initialize OLED display and IMU
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
MPU6050 accelgyro;
int16_t ax, ay, az;

// initialize input time, score, and random seed
// global vars for input time, score, and random seed
float inputTime = 2;
int score = 0;
int rand_seed_counter;

int oldTwistVal;
int oldRipVal;

// initialize file for sd card
// File sdCard;
// TMRpcm tmrpcm;

void setup() {
    rand_seed_counter = 0;

    // start button
    pinMode(START_BUTTON, INPUT);

    // initialize accelgyro
    accelgyro.initialize();

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
    int twistVal = map(analogRead(TWIST_IT_ROT_POT), 0, 1023, 0, 179);

    // twistVal = map(twistVal, 0, 1023, 0, 179);
    // wait(15);

    // find difference between current val and prev val
    int diff = abs(twistVal - PREV_TWIST_IT);

    // display.clearDisplay();
    // display.setCursor(0, 0);
    // display.setTextSize(1);
    // display.print("twist: ");

    // display.println(String(diff));
    // display.println("waiting for start");
    // display.display();
    // hold();

    // if difference is greater than tolerance, store old val and return correct
    if (diff > TOLERANCE) {
        PREV_TWIST_IT = twistVal;
        return 1;
    }
    return 0;
}

int poll_rip_it() {
    if (analogRead(RIP_IT_SLIDE_POT) > PREV_RIP_IT + 100 || analogRead(RIP_IT_SLIDE_POT) < PREV_RIP_IT - 100) {
        PREV_RIP_IT = analogRead(RIP_IT_SLIDE_POT);
        return 2;
    } else
        return 0;
}

int poll_pour_it() {
    accelgyro.getAcceleration(&ax, &ay, &az);
    // check not upright

#if UPRIGHT_DIRECTION == Z
    if (az < ax && az < ay) {
        // display.setCursor(0, 100);
        // display.println("not upright");
        // display.display();
        return 4;
#elif UPRIGHT_DIRECTION == X
    if (ax < ay && ax < az) {
        // display.setCursor(0, 100);
        // display.println("not upright");
        // display.display();
        return 4;
#elif UPRIGHT_DIRECTION == Y
    if (ay < ax && ay < az) {
        // display.setCursor(0, 100);
        // display.println("not upright");
        // display.display();
        return 4;
#endif
    } else {
        return 0;
    }
}

int poll_sensors() {
    // return poll_rip_it();
    return poll_pour_it() + poll_twist_it();
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

    // busy-wait 0.8 seconds to sample user input
    // while(millis() - timeStart < 800);

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
        display.setTextSize(2);
        display.print("GAME OVER!\n\n");
        display.print(String(sensor_sum) + "\n" + commands_list[sensor_sum >> 1]);
        display.display();

        while(digitalRead(START_BUTTON) == LOW);
        while(digitalRead(START_BUTTON) == HIGH);

        // go back to start state
        loop();
    } else {
        // if input was correct, increase score and decrease allowed input time
        score++;
        inputTime -= 0.02;
        display_score_to_oled();
    }
}

void loop() {
    // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
    // because we don't have an RTC to keep track of time.
    while (digitalRead(START_BUTTON) == LOW) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.print("waiting...");
        display.display();
        display.clearDisplay();
        rand_seed_counter++;
    }

    // wait for button to be let-go of
    while (digitalRead(START_BUTTON) == HIGH)
        ;
    display.clearDisplay();

    // seed random numbers
    srand(rand_seed_counter);

    // loop for the game
    bool isRunning = true;
    if (!isRunning) {  // game has not started (ie. button needs to be pressed)
        display.setCursor(0, 0);
        display.print("game is not running");
        display.display();
    }

    // game is running - run game loop
    else {
        while (true) {
            // get a random command
            // int command = rand() % 3;
            int command = PourIt;
            // twist it
            if (command == TwistIt) {
                // output sound
                tone(SPEAKER, 10000, 500);
                noTone(SPEAKER);

                // poll for user input
                wait_for_user_response(TWIST_IT);
                wait(1000);
            }
            // pour it
            else if (command == PourIt) {
                // poll for user input
                wait_for_user_response(POUR_IT);
                wait(1000);
            }
            // rip it
            // else if (command == RipIt) {
            //     // poll for user input
            //     wait_for_user_response(RIP_IT);
            //     delay(1000);
            // }
        }
    }
    wait(2000);
}
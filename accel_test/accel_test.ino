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

#define WIDTH 128
#define HEIGHT 64

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
MPU6050 accelgyro; // IMU object

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

int16_t ax, ay, az;
int16_t gx, gy, gz;

void loop() {

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // keep increasing the rand seed counter until start is pressed. This will add a randomness effect
  // because we don't have an RTC to keep track of time.
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(ax);
  display.print(ay);
  display.println(az);
  display.print(gx);
  display.print(gy);
  display.println(gz);
  display.display();

  delay(0.5*1000);
}
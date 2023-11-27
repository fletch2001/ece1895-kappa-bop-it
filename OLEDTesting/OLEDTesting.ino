#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64

#define RESET 4
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);

void setup() {

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while(true);
  }

  display.clearDisplay();

  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
}

void loop() {
  for (int i = 0; i < 20; i++) {
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print(String(i));
    display.display();
  }
}

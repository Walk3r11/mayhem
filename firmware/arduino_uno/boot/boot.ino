#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#define GREEN_LED 2
#define RED_LED 3

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  tft.begin();
  tft.fillScreen(ILI9341_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(10, 10);
  tft.println("Booting Devices...");

  delay(500);
  tft.setCursor(10, 40);
  tft.setTextColor(ILI9341_CYAN);
  tft.println("ESP-1: OK");

  delay(500);
  tft.setCursor(10, 70);
  tft.println("ESP-2: OK");

  delay(500);
  tft.setCursor(10, 100);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("Arduino: OK");

  delay(500);
  tft.setCursor(10, 130);
  tft.setTextColor(ILI9341_GREEN);
  tft.println("System Ready!");
}

void loop() {
  
}

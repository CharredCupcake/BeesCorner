#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "epd4in2b_V2.h"
#include "imagedata.h"
#include "epdpaint.h"

unsigned int VISITOR_COUNT = 0;

//Display vars
#define COLORED     0
#define UNCOLORED   1
char itoaStr[10];
unsigned char image[1500];
Paint paint(image, 32, 300); //width should be the multiple of 8
Epd epd;

//Dust vars
#define PM25Pin 6
size_t PM25 = 0;
void queryPM25();

//LEDs vars
#define LED_OUT 3
#define LED_COUNT 12
#define LED_brightness 50
Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(LED_COUNT, LED_OUT, NEO_GRB + NEO_KHZ800);
void changeColor(const size_t r, const size_t g, const size_t b);
void LEDsInit();
void LEDsOn();
void LEDsOff();
void LEDsColorRed();
void LEDsColorGreen();
void LEDsColorYellow();
void LEDsColorPM25(const size_t pm25);
void LEDsStart();

//Detection vars
#define DETECT_PIN 5

void setup() {
  epd.Init();
  pinMode(DETECT_PIN, INPUT);
  pinMode(PM25Pin, INPUT);
  LEDsInit();
  Serial.begin(9600);
}

void loop() {

  Serial.println("looped");
  delay(8U*(60U*1000U));
  epd.ClearFrame();
  paint.SetRotate(3);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, " ", &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  paint.Clear(COLORED);
  paint.DrawStringAt(30, 4, "WWW.PODOBRI.ORG", &Font24, UNCOLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 32, 0, paint.GetWidth(), paint.GetHeight());
  paint.Clear(COLORED);
  //paint.DrawStringAt(90, 4, "SHUMEN", &Font24, UNCOLORED);
  paint.DrawStringAt(80, 4, "KASPICHAN", &Font24, UNCOLORED);
  //paint.DrawStringAt(32, 4, "VELIKI PRESLAV", &Font24, UNCOLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 64, 0, paint.GetWidth(), paint.GetHeight());
  /*
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(15, 0, "Temperature", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 96, 0, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(215, 0, varToStr(33), &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 96, 0, paint.GetWidth(), paint.GetHeight());
  paint.DrawStringAt(245, 0, "o", &Font12, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 96, 0, paint.GetWidth(), paint.GetHeight());
  paint.DrawStringAt(255, 0, "C", &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 96, 0, paint.GetWidth(), paint.GetHeight());
  */
  queryPM25();
  LEDsStart();
  Serial.println("DUST: ");
  Serial.println(PM25);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(15, 0, "Air Quality PM2.5", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 128, 0, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(100, 0, itoa(PM25, itoaStr, 10), &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 160, 0, paint.GetWidth(), paint.GetHeight());
  paint.DrawStringAt(155, 0, "mg/m", &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 160, 0, paint.GetWidth(), paint.GetHeight());
  paint.DrawStringAt(223, 0, "3", &Font12, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 160, 0, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(80, 0, "Visitors", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 228, 0, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(100, 0, itoa(VISITOR_COUNT, itoaStr, 10), &Font24, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 255, 0, paint.GetWidth(), paint.GetHeight());
  /* This displays the data from the SRAM in e-Paper module */
  epd.DisplayFrame();
  /* Deep sleep */
  //epd.Sleep();
  LEDsOff();
  while (digitalRead(DETECT_PIN) == LOW)
  {
    Serial.println("waiting for sensor");
  }
  VISITOR_COUNT++;
}

//Dust
void queryPM25()
{
  PM25 = pulseIn(PM25Pin, HIGH, 1500000) / 1000 - 2;
}

//LEDs
void changeColor(const size_t r, const size_t g, const size_t b ) 
{
  for(size_t i=0; i<LED_COUNT; i++) {
    LEDs.setPixelColor(i, LEDs.Color(r, g, b));
    LEDs.show();
    delay(350);
  }
}
void LEDsInit()
{
  LEDs.begin();
}
void LEDsOn()
{
  LEDs.setBrightness(LED_brightness);
  LEDs.show();
}
void LEDsOff()
{
  LEDs.clear();
  LEDs.show();
}
void LEDsColorRed()
{
  changeColor(255, 0, 0);
  LEDs.show();
}
void LEDsColorGreen()
{
  changeColor(0, 255, 0);
  LEDs.show();
}
void LEDsColorYellow()
{
  changeColor(190, 65, 0);
  LEDs.show();
}
void LEDsColorPM25(const size_t pm25)//changes according to pm2.5 concentration
{
  if(PM25 <= 35)
  {
    LEDsColorGreen();
  }
  else
  {
    LEDsColorRed();
  }
}
void LEDsStart()
{
  LEDsColorPM25(PM25);
  LEDsOn();
}

#include <Arduino.h>
#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <driver/rtc_io.h>

#include "image.cpp"

//Sleep vars
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10*60        /* Time ESP32 will go to sleep (in seconds) */
//RTC_DATA_ATTR int var = 0; //Non-volatile memory
//

//Dust vars
#define PM25Pin 27
size_t PM25_NEW = 0;
RTC_DATA_ATTR int PM25_OLD = 0;
void queryPM25();
//

//Screen vars
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=5*/ SS, /*DC=17*/ 17, /*RST=16*/ 16, /*BUSY=4*/ 4)); // GDEW042T2
//uint16_t x = 125;//((display.width() - tbw) / 2) - tbx;
//uint16_t y = 175;//((display.height() - tbh) / 2) - tby;
void displayBackground();
void displayDate();
void displayDay();
void displayTemperature();
void displayDust();
void displayHumidity();
void displayPressure();
void loadScreen();
void refreshData();
//

//I2C vars
TwoWire I2C_BME = TwoWire(0);
#define I2C_SDA 25
#define I2C_SCL 33
//

//BME vars
Adafruit_BME280 BME;
size_t temp_new = 0;
size_t humi_new = 0;
size_t pres_new = 0;
RTC_DATA_ATTR size_t temp_old = 26;
RTC_DATA_ATTR size_t humi_old = 34;
RTC_DATA_ATTR size_t pres_old = 1047;
void initBME();
void queryBME();
//

//LEDs vars
#define LED_OUT 32
#define LED_COUNT 10
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
// TODO light patterns
//

//Interupt vars
#define INTERUPT_PIN GPIO_NUM_14
void interuptInit();
void sensorTriggered();
//

void setup() {

  delay(500);// CORE BUG QUICK FIX
  Serial.begin(9600);
  switch (esp_sleep_get_wakeup_cause())
  {
  case ESP_SLEEP_WAKEUP_TIMER:
  Serial.println("Timer Woke up!");
  queryPM25();
  initBME();
  queryBME();
  // TODO upload data
  // TODO start sensor
  //interuptInit(); //!!! UNDER DEBUGGING !!!
  //temp funs
  LEDsStart();
  loadScreen();
  delay(5*1000);
  LEDsOff();
  //
  delay(5*1000);
  break;
  case ESP_SLEEP_WAKEUP_EXT0:
  Serial.println("Sensor Woke up!");
  LEDsStart();
  //queryPM25();
  //initBME();
  //queryBME();
  loadScreen();
  delay(10*1000);
  LEDsOff();
  break;
  case ESP_SLEEP_WAKEUP_UNDEFINED:
  Serial.println("Undefined Woke up!");
  queryPM25();
  initBME();
  queryBME();
  loadScreen();
  LEDsStart();
  detachInterrupt(digitalPinToInterrupt(INTERUPT_PIN));
  delay(10*1000);// TODO add timer to switch the system off
  LEDsOff();
  break;
  default:
    break;
  }


  //Sleep
  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 1); //!!! UNDER DEBUGGING !!!
  // delay(500);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(500);
  esp_deep_sleep_start();
  /*  When the chip wakes up from the  sleep , the pins specified will be configured as RTC IO.
      To be able to use them again as normal digital pins, you have first to call the rtc_gpio_deinit(gpio_num)  method.
      The ext0_wakeup method at the moment cannot be used together with touch pad or ULP events.
  */
  //
}

void loop() {
  
}

//Screen fun
void displayBackground()
{
  display.setRotation(0);
  display.drawImage(IMAGE_BACKGROUND, 0, 0, 400, 300);
  display.firstPage();
}
void displayDate()
{
  const size_t x = 5;
  const size_t y = 10;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 112, 16);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 16); // y + offset depending on font size
  display.print("20/04/2022"); // TODO add date var
  display.nextPage();
}
void displayDay()
{
  const size_t x = 190;
  const size_t y = 35;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 100, 16);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 16); // y + offset depending on font size
  display.print("Wednesday"); // TODO add day var
  display.nextPage();
}
void displayTemperature()
{
  const size_t x = 85;
  const size_t y = 90;
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 56, 32);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 32); // y + offset depending on font size
  display.print(String(temp_old)); //°C
  display.nextPage();
}
void displayDust()
{
  const size_t x = 126;
  const size_t y = 160;
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 56, 32);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 32); // y + offset depending on font size
  display.print(String(PM25_OLD));
  display.nextPage();
}
void displayHumidity()
{
  const size_t x = 85;
  const size_t y = 260;
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 56, 16);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 16); // y + offset depending on font size
  display.print(String(humi_old) + "%");
  display.nextPage();
}
void displayPressure()
{
  const size_t x = 75;
  const size_t y = 305;
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, 64, 16);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y + 16); // y + offset depending on font size
  display.print(String(pres_old));
  display.nextPage();
}
void loadScreen()
{
  display.init();
  displayBackground();
  displayDate();
  displayDay();
  displayTemperature();
  displayDust();
  displayHumidity();
  displayPressure();
}
//

//Dust fun
void queryPM25()
{
  pinMode(PM25Pin, INPUT);
  PM25_NEW = pulseIn(PM25Pin, HIGH, 1500000) / 1000 - 2;
  (PM25_NEW)? PM25_OLD = PM25_NEW : PM25_NEW = PM25_OLD;
}
//

//BME fun
void initBME()
{
  I2C_BME.begin(I2C_SDA, I2C_SCL, 100000); //I2C init
  BME.begin(0x76, &I2C_BME);
}
void queryBME()
{
  temp_new = BME.readTemperature();
  (temp_new)? temp_old = temp_new: temp_new = temp_old;
  humi_new = BME.readHumidity();
  (humi_new)? humi_old = humi_new: humi_new = humi_old;
  pres_new = BME.readPressure();
  (pres_new)? pres_old = pres_new: pres_new = pres_old;
}
//

//NeoPixel fun
void changeColor(const size_t r, const size_t g, const size_t b ) 
{
  for(size_t i=0; i<LED_COUNT; i++) {
    LEDs.setPixelColor(i, LEDs.Color(r, g, b));
    LEDs.show();
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
  if(PM25_OLD <= 12)
  {
    LEDsColorGreen();
  }
  else
  {
    if(PM25_OLD <= 55)
    {
      LEDsColorYellow();
    }
    else
    {
      LEDsColorRed();
    }
  }
}
void LEDsStart()
{
  LEDsInit();
  LEDsColorPM25(PM25_OLD);
  LEDsOn();
}
//

//Interrupt fun
void interuptInit()
{
  rtc_gpio_deinit(INTERUPT_PIN);
  pinMode(INTERUPT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INTERUPT_PIN), sensorTriggered, RISING);
}
void sensorTriggered()
{
  detachInterrupt(digitalPinToInterrupt(INTERUPT_PIN));
  loadScreen();
  LEDsStart();
}
//
/*External libraries you need:
 * Adafruit RTCLib:   https://github.com/adafruit/RTClib
 * FastLED:           https://github.com/FastLED/FastLED
 */

// Libraries Required:
#include <FastLED.h>
#include <Wire.h>
#include "RTClib.h"

// FastLED Constants:
#define NUM_LEDS 25
#define DATA_PIN 7
#define BRIGHTNESS 255

#define SEG_0_STARTING_INDEX 0
#define SEG_1_STARTING_INDEX 7
#define SEPERATOR_INDEX 14
#define SEPERATOR_LENGTH 2
#define SEG_2_STARTING_INDEX 16
#define SEG_3_STARTING_INDEX 23

// INPUT/OUTPUT PINS:
#define SET_BTN 2
#define UP_BTN 4
#define DOWN_BTN 3
#define CHANGE_COLOR_BTN 5

// Creating rtc & DateTime Object:
RTC_DS3231 rtc;
DateTime now;

// Creating LEDs object for FastLED lib.:
CRGB LEDs[NUM_LEDS];

// Creating Colors:
CRGB color = CRGB(255, 20, 147); // Default Color;
CRGB colorOFF = CRGB::Black;
CHSV colorCHSV;

// Variables:
volatile int hours = 0;   // For Storing Hour Value
volatile int minutes = 0; // For Storing Minute Value

long changeColorBtnLastDebounceTime = 0;
long setBtnLastDebounceTime = 0;
long upBtnLastDebounceTime = 0;
long downBtnLastDebounceTime = 0;
long debounceDelay = 100;

enum modes
{
  SHOW_CLOCK,
  SET_HOUR,
  SET_MINUTES,
  MODES_LENGTH
};
enum modes currentClockMode;

enum colorModes
{
  YELLOW,
  GREEN,
  BLUE,
  RED,
  CYAN,
  MAGENTA,
  RAINBOW,
  COLORMODES_LENGTH
};
enum colorModes currentColorMode;

void setup()
{

  // Setting Up Deafult Modes
  currentClockMode = SHOW_CLOCK;
  currentColorMode = RAINBOW;

  // Seting Up Pin Modes:
  pinMode(SET_BTN, INPUT);
  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(CHANGE_COLOR_BTN, INPUT);

  // Initialize LED Strip:
  FastLED.delay(1000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Initialize Serial Connection:
  Serial.begin(9600);
  while (!Serial)
    ;

  // Initaialize RTC:
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  // Setting Up Time If RTC losts Power/ New Module:
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Display Welcome Message
  now = rtc.now();
  displayWelcomeMessage();
}

void displayWelcomeMessage()
{
  Serial.println("Welcome To Mark I :-)");
  Serial.println("RTC Started Succesfully");
  Serial.print("\nRTC Set To:- ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print("\t");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print("\nSystem Date:");
  Serial.print(__DATE__);
  Serial.print("\tSystem Time:");
  Serial.print(__TIME__);
  Serial.print("\nRTC Temperature:- ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");
}

void displaySegments(int startindex, int number)
{
  // Based On 7 Seg LED Layout
  byte numbers[] = {
      0b00111111, // 0
      0b00000110, // 1
      0b01011011, // 2
      0b01001111, // 3
      0b01100110, // 4
      0b01101101, // 5
      0b01111101, // 6
      0b00000111, // 7
      0b01111111, // 8
      0b01101111, // 9
      0b00000000  // 10 (OFF)
  };
  for (int i = 0; i < 7; i++)
  {
    LEDs[i + startindex] = ((numbers[number] & 1 << i) == 1 << i) ? (currentClockMode == RAINBOW ? colorCHSV : color) : colorOFF;
  }
}

void show_RTC_time()
{
  Serial.print("RTC set to: ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print("\t");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.println();
}

void loop()
{
  switch (currentClockMode)
  {
  case 0:
    color = CRGB::Yellow;
    break;
  case 1:
    color = CRGB::Green;
    break;
  case 2:
    color = CRGB::Blue;
    break;
  case 3:
    color = CRGB::Red;
    ;
    break;
  case 4:
    color = CRGB(0, 255, 255); // CYAN
    break;
  case 5:
    color = CRGB(255, 0, 255); // Magenta
    break;
  case 6: // COLOR CHANGING MODE / RAINBOW MODE
  default:
    colorCHSV.sat = 255;
    colorCHSV.val = 255;
    if (colorCHSV.hue >= 255)
      colorCHSV.hue = 0;
    else
      colorCHSV.hue++;
    FastLED.show();
    break;
  }

  // Checks if ColorChange Button is Pressed
  if ((millis() - changeColorBtnLastDebounceTime) > debounceDelay)
  {
    if (digitalRead(CHANGE_COLOR_BTN) == HIGH)
    {
      currentClockMode = (currentClockMode + 1) % COLORMODES_LENGTH;
    }
    changeColorBtnLastDebounceTime = millis();
  }

  // Checks if SET Button is Pressed
  if ((millis() - setBtnLastDebounceTime) > debounceDelay)
  {
    if (digitalRead(SET_BTN) == HIGH)
    {
      currentClockMode = (currentClockMode + 1) % MODES_LENGTH;
      if (currentClockMode == SHOW_CLOCK)
      {
        int year = now.year();
        int month = now.month();
        int day = now.day();
        int seconds = 0;
        rtc.adjust(DateTime(year, month, day, hours, minutes, seconds));
        show_RTC_time();
      }
      else
      {
        hours = currentClockMode == SET_HOUR ? now.hour() : hours;
        minutes = currentClockMode == SET_MINUTES ? now.minute() : minutes;
        // Checks if UP Button is Pressed
        if ((millis() - upBtnLastDebounceTime) > debounceDelay)
        {
          if (digitalRead(UP_BTN) == HIGH)
          {
            if (currentClockMode == SET_HOUR)
              hours = (hours + 1) % 24;
            else if (currentClockMode == SET_MINUTES)
              minutes = (minutes + 1) % 60;
          }
          upBtnLastDebounceTime = millis();
        }
        // Checks if DOWN Button is Pressed
        if ((millis() - downBtnLastDebounceTime) > debounceDelay)
        {
          if (digitalRead(DOWN_BTN) == HIGH)
          {
            if (currentClockMode == SET_HOUR)
              hours = hours > 0 ? hours - 1 : 23;
            else if (currentClockMode == SET_MINUTES)
              minutes = minutes > 0 ? minutes - 1 : 59;
          }
          downBtnLastDebounceTime = millis();
        }
      }
    }
    setBtnLastDebounceTime = millis();
  }
  int formattedHours = hours > 12 ? hours - 12 : hours;
  int hoursLeft = formattedHours / 10;
  int hoursRight = formattedHours % 10;
  int minutesLeft = minutes / 10;
  int minutesRight = minutes % 10;

  if (currentClockMode == SHOW_CLOCK)
  {
    displaySegments(SEG_0_STARTING_INDEX, minutesRight);
    displaySegments(SEG_1_STARTING_INDEX, minutesLeft);
    // Blinking Seperators
    for (int i = 0; i < SEPERATOR_LENGTH; i++)
    {
      LEDs[SEPERATOR_INDEX + i] = (now.second() % 2 == 0) ? (currentClockMode == RAINBOW ? colorCHSV : color) : colorOFF;
    }
    displaySegments(SEG_2_STARTING_INDEX, hoursRight);
    displaySegments(SEG_3_STARTING_INDEX, hoursLeft);
  }
  else if (currentClockMode == SET_HOUR)
  {
    displaySegments(SEG_0_STARTING_INDEX, minutesRight);
    displaySegments(SEG_1_STARTING_INDEX, minutesLeft);
    // To Make Blinking Effect On the SET Modes
    if (now.second() % 2 == 0)
    {
      displaySegments(SEG_2_STARTING_INDEX, hoursRight);
      displaySegments(SEG_3_STARTING_INDEX, hoursLeft);
    }
    else
    {
      displaySegments(SEG_2_STARTING_INDEX, 10);
      displaySegments(SEG_3_STARTING_INDEX, 10);
    }
  }
  else if (currentClockMode == SET_MINUTES)
  {
    displaySegments(SEG_2_STARTING_INDEX, hoursRight);
    displaySegments(SEG_3_STARTING_INDEX, hoursLeft);
    // To Make Blinking Effect On the SET Modes
    if (now.second() % 2 == 0)
    {
      displaySegments(SEG_0_STARTING_INDEX, minutesRight);
      displaySegments(SEG_1_STARTING_INDEX, minutesLeft);
    }
    else
    {
      displaySegments(SEG_0_STARTING_INDEX, 10);
      displaySegments(SEG_1_STARTING_INDEX, 10);
    }
  }
  FastLED.show();
}

// LED ARRANGEMENT:
//+---------+---------+-----+---------+---------+
//|    23   |   16    | 14  |    7    |    0    |-> Starting Index
//+---------+---------+-----+---------+---------+
//|    3    |    6    |     |    7    |    0    |
//| 8     4 | 1     7 |  4  | 2     8 | 5     1 |
//|         |    2    |     |    3    |    6    |
//| 7     5 | 0     8 |  5  | 1     9 | 4     2 |
//|    6    |    9    |     |    0    |    3    |
//+----+----+---------+-----+---------+---------+
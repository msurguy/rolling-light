#include "Arduino.h"

// include the AnalogMultiButton library
#include <AnalogMultiButton.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WS2812FX.h>


// Data wire is plugged into pin 10 on the Trinket
#define ONE_WIRE_BUS 10

#define PIXEL_PIN    6 // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 20

const int BALL_HOLDER_BUTTON = 11;

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


// arrays to hold device address
DeviceAddress insideThermometer;

// define the pin you want to use
const int BUTTONS_PIN = A0;

// set how many buttons you have connected
const int BUTTONS_TOTAL = 10;

float temperature;

// find out what the value of analogRead is when you press each of your buttons and put them in this
// array
// you can find this out by putting Serial.println(analogRead(BUTTONS_PIN)); in your loop() and
// opening the serial monitor to see the values
// make sure they are in order of smallest to largest
const int BUTTONS_VALUES[BUTTONS_TOTAL] = { 330, 504, 611, 683, 735, 775, 805, 830, 851, 868 };

// you can also define constants for each of your buttons, which makes your code easier to read
// define these in the same order as the numbers in your BUTTONS_VALUES array, so whichever button
// has the smallest analogRead() number should come first
const int BUTTON_1  = 0;
const int BUTTON_2  = 1;
const int BUTTON_3  = 2;
const int BUTTON_4  = 3;
const int BUTTON_5  = 4;
const int BUTTON_6  = 5;
const int BUTTON_7  = 6;
const int BUTTON_8  = 7;
const int BUTTON_9  = 8;
const int BUTTON_10 = 9;

// make an AnalogMultiButton object, pass in the pin, total and values array
AnalogMultiButton buttons(BUTTONS_PIN, BUTTONS_TOTAL, BUTTONS_VALUES);

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

unsigned long last_change = 0;
unsigned long now         = 0;
unsigned long prevTime    = 0;

WS2812FX ws2812fx = WS2812FX(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint32_t colors[] = {  BLUE, RED, GREEN }; // create an array of colors


#define D_MILLIS 1000                      // wait time after start conversion
unsigned long Dtime;                       // last start time, millis
byte Dflag;
byte dstate;                               // state variable


uint32_t tempToColor;

#define ACTIVE_COLOR 0xBDF609

// pass a fourth parameter to set the debounce time in milliseconds
// this defaults to 20 and can be increased if you're working with particularly bouncy buttons

void setup() {
  // begin serial so we can see which buttons are being pressed through the serial monitor
  Serial.begin(115200);

  pinMode(BALL_HOLDER_BUTTON, INPUT_PULLUP);

  ws2812fx.init();
  ws2812fx.setBrightness(255);

  ws2812fx.setSpeed(128);

  // ws2812fx.setSegment(0, 0, 9, FX_MODE_COMET, colors, 2000, FADE_XSLOW);
  ws2812fx.start();


  // Start up the sensor library
  sensors.begin();
  sensors.setWaitForConversion(false); // we'll do our own timing
  Dflag = dstate = 0;

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println(
      "Unable to find address for Device 0");
  sensors.setResolution(insideThermometer, 11);
}

void Dstate()
{
  switch (dstate)
  {
  case 0:
    sensors.requestTemperatures();                // Send the command to get temperatures
    Dtime  = millis();                            // keep track of time of start
    dstate = 1;                                   // go to waiting state
    break;

  case 1:                                         // waiting for results to be ready

    if (millis() - Dtime > D_MILLIS) dstate = 2;  // done waiting if true
    break;

  case 2:                                         // read results into our array, and restart

    if (sensors.getTempFByIndex(0) < 100) {
      temperature = sensors.getTempFByIndex(0);
    }

    // }
    Dflag  = 1;
    dstate = 0; // get another
    break;
  }
}

uint16_t effect1(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 0, ACTIVE_COLOR);
  ws2812fx.setPixelColor(19, ACTIVE_COLOR);

  return seg.speed; // return the segment's speed
}

uint16_t effect2(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 1, ACTIVE_COLOR);
  ws2812fx.setPixelColor(18, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect3(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 2, ACTIVE_COLOR);
  ws2812fx.setPixelColor(17, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect4(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 3, ACTIVE_COLOR);
  ws2812fx.setPixelColor(16, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect5(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 4, ACTIVE_COLOR);
  ws2812fx.setPixelColor(15, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect6(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 5, ACTIVE_COLOR);
  ws2812fx.setPixelColor(14, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect7(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 6, ACTIVE_COLOR);
  ws2812fx.setPixelColor(13, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect8(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 7, ACTIVE_COLOR);
  ws2812fx.setPixelColor(12, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect9(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 8, ACTIVE_COLOR);
  ws2812fx.setPixelColor(11, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint16_t effect10(void) {
  WS2812FX::Segment seg = ws2812fx.getSegment();

  for (uint16_t i = seg.start; i <= seg.stop; i++) {
    ws2812fx.setPixelColor(i, 0);
  }
  ws2812fx.setPixelColor( 9, ACTIVE_COLOR);
  ws2812fx.setPixelColor(10, ACTIVE_COLOR);
  return seg.speed; // return the segment's speed
}

uint32_t color_wheel(uint8_t pos) {
  pos = 255 - pos;

  if (pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}

void loop() {
  now = millis();

  Dstate(); // do sensor reading

  if (Dflag)
  {
    // Serial.println(temperature);
    Dflag = 0;
  }

  tempToColor = map(temperature, 70, 85, 200, 255);

  ws2812fx.service();

  if (digitalRead(BALL_HOLDER_BUTTON) == HIGH)
  {
    // Ball is in the socket
    ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_RAINBOW_CYCLE, colors, 1, false);

    // Serial.println("BALL IS IN!");
  } else {
    // When Ball is not in the socket


    // update the AnalogMultiButton object every loop
    buttons.update();

    if (buttons.isPressed(BUTTON_1))
    {
      Serial.println("Button 1 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect1);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_2)) {
      Serial.println("Button 2 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect2);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_3)) {
      Serial.println("Button 3 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect3);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_4)) {
      Serial.println("Button 4 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect4);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_5)) {
      Serial.println("Button 5 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect5);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_6)) {
      Serial.println("Button 6 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect6);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_7)) {
      Serial.println("Button 7 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect7);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_8)) {
      Serial.println("Button 8 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect8);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_9)) {
      Serial.println("Button 9 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect9);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    }
    else if (buttons.isPressed(BUTTON_10)) {
      Serial.println("Button 10 is pressed");
      prevTime = now;

      ws2812fx.setCustomMode(effect10);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    }
    else {
      if (now - prevTime > 100) {
        ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_BREATH, color_wheel(tempToColor), 1,
                            true);
      }
    }

    // Serial.println("BALL IS NOT IN!");
  }


  //
  // if (buttons.isPressed(BUTTON_2))
  // {
  //   Serial.println("Button 2 is pressed");
  //
  //   strip.setPixelColor( 1, strip.Color(0, 0, 255));
  //   strip.setPixelColor(18, strip.Color(0, 0, 255));
  //   strip.show();
  // } else {
  //   strip.setPixelColor( 1, 0);
  //   strip.setPixelColor(18, 0);
  //   strip.show();
  // }


  // // check if BUTTON_RED is pressed
  // if (buttons.isPressed(BUTTON_RED))
  // {
  //   Serial.println("Button red is pressed");
  // } else {
  //   // Serial.println("Button red is not pressed");
  // }

  // check if BUTTON_GREEN has just been pressed this update
  // if (buttons.onPress(BUTTON_GREEN))
  // {
  //   Serial.println("Green has been pressed");
  // }

  // check if BUTTON_GREEN has just been released this update
  // if (buttons.onRelease(BUTTON_GREEN))
  // {
  //   Serial.println("Green has been released");
  // }

  // do this if BUTTON_BLUE has been released
  // if (buttons.onRelease(BUTTON_BLUE))
  // {
  //   Serial.println("Blue has been released");
  // }

  // do this once if BUTTON_BLUE has been held for 1 second
  // if (buttons.onPressAfter(BUTTON_BLUE, 1000))
  // {
  //   Serial.println("Blue has been down for 1 second");
  // }

  // do this contantly if BUTTON_GREEN has been held down for less than a second
  // if (buttons.isPressedBefore(BUTTON_GREEN, 1000))
  // {
  //   Serial.print("Green is held for ");
  //   Serial.print(             buttons.getPressDuration());
  //   Serial.println(" ms");
  // }

  // do this contantly if BUTTON_RED has been held down for more than a second
  // if (buttons.isPressedAfter(BUTTON_RED, 1000))
  // {
  //   Serial.print("Red is held for ");
  //   Serial.print(           buttons.getPressDuration());
  //   Serial.println(" ms");
  // }

  // do this if BUTTON_BLUE was released, and it was held for 1 second or less
  // if (buttons.onReleaseBefore(BUTTON_BLUE, 1000))
  // {
  //   Serial.println("Blue has been released after less than 1 second of pressing");
  //   Serial.print("Blue was held for ");
  //   Serial.print(             buttons.getLastReleasePressDuration());
  //   Serial.println(" ms");
  // }

  // do this if BUTTON_BLUE was released, and it was held for 2 seconds or more
  // if (buttons.onReleaseAfter(BUTTON_BLUE, 2000))
  // {
  //   Serial.println("Blue has been released after at least 2 seconds of pressing");
  //   Serial.print("Blue was held for ");
  //   Serial.print(             buttons.getLastReleasePressDuration());
  //   Serial.println(" ms");
  // }

  // do this if BUTTON_BLUE has been released
  // if (buttons.onRelease(BUTTON_PURPLE))
  // {
  //   Serial.println("Purple has been released");
  // }

  //
  // More examples:
  //
  // do this once when BUTTON_BLUE is pressed, and again after 1 second
  // if(buttons.onPressAndAfter(BUTTON_BLUE, 1000)) {}
  //
  // do this once if BUTTON_BLUE is held for 1 second, and again every 0.5 seconds after that
  // if(buttons.onPressAfter(BUTTON_BLUE, 1000, 500)) {}
  //
  // do this once when BUTTON_BLUE is pressed, and again after 1 second, and again every 0.5 seconds
  // after that
  // useful for cursors or scrolling through menu items
  // if(buttons.onPressAndAfter(BUTTON_BLUE, 1000, 500)) {}
  //

  // delay(10);
}

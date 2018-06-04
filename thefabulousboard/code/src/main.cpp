// Ryan Wu and Maksim Surguy, Spring 2018

/*

   This code runs on Trinket Pro and does the following:
   - Reads temperature from a OneWire sensor and changes color of the lights accordingly
   - Reads state of 10 buttons via one analog pin (!)
   - Reads state of another button to enable / disable certain animations

   This code uses a highly modified WS2812FX library for the lighting effects

 */

#include "Arduino.h"

// include the libraries
#include <AnalogMultiButton.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Had to trim this library down to fit on the Trinket Pro
#include <WS2812FX.h>

// Data wire is plugged into pin 10 on the Trinket Pro
#define ONE_WIRE_BUS 10
#define PIXEL_PIN    6        // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 20        // How many LEDs
#define BALL_HOLDER_BUTTON 11 // Which pin is the ball holder button connected to
#define ACTIVE_COLOR 0xBDF609 // Which color to turn the LEDs with, when the ball is pressing on the
                              // springs

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// define the pin you want to use for the 10 buttons (that work like a piano)
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

unsigned long last_change = 0;
unsigned long now         = 0;
unsigned long prevTime    = 0;

// Initialize LED effects
WS2812FX ws2812fx = WS2812FX(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint32_t colors[] = {  BLUE, RED, GREEN }; // create an array of colors

// The following values are for state machine to read the temp sensor data
#define D_MILLIS 1000                      // wait time after start conversion
unsigned long Dtime;                       // last start time, millis
byte Dflag;
byte dstate;                               // state variable

uint32_t tempToColor;

// pass a fourth parameter to set the debounce time in milliseconds
// this defaults to 20 and can be increased if you're working with particularly bouncy buttons

void setup() {
  // begin serial so we can see which buttons are being pressed through the serial monitor
  Serial.begin(115200);

  pinMode(BALL_HOLDER_BUTTON, INPUT_PULLUP);

  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(128);
  ws2812fx.start();

  // Start up the sensor library
  sensors.begin();
  sensors.setWaitForConversion(false); // we'll do our own timing
  Dflag = dstate = 0;

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println(
      "Unable to find address for Device 0");
  sensors.setResolution(insideThermometer, 11);
}

// Function to manage state of the temperature sensor
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

    Dflag  = 1;
    dstate = 0; // get another
    break;
  }
}

// The following functions define what happens when each of the 10 buttons are pressed.
// Doing it this way allows for many possibilities of visual effects being implemented
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

  // Run the LED effects service
  ws2812fx.service();

  Dstate(); // get the sensor reading

  if (Dflag)
  {
    // Uncomment this line to see the actual temperature reading
    // Serial.println(temperature);
    Dflag = 0;
  }

  // Map temperature values to some range on the color wheel
  tempToColor = map(temperature, 70, 85, 200, 255);

  // We need to do different things depending on the ball position in the socket
  if (digitalRead(BALL_HOLDER_BUTTON) == HIGH)
  {
    // Ball is in the socket, shoot the rainbows!
    ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_RAINBOW_CYCLE, colors, 1, false);

    // Serial.println("BALL IS IN!");
  } else {
    // When Ball is not in the socket
    // Serial.println("BALL IS NOT IN!");

    // Read the state of the 10 buttons (keys)
    buttons.update();

    if (buttons.isPressed(BUTTON_1))
    {
      // Serial.println("Button 1 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect1);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_2)) {
      // Serial.println("Button 2 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect2);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_3)) {
      // Serial.println("Button 3 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect3);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_4)) {
      // Serial.println("Button 4 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect4);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_5)) {
      // Serial.println("Button 5 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect5);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_6)) {
      // Serial.println("Button 6 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect6);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_7)) {
      // Serial.println("Button 7 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect7);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_8)) {
      // Serial.println("Button 8 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect8);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    } else if (buttons.isPressed(BUTTON_9)) {
      // Serial.println("Button 9 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect9);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    }
    else if (buttons.isPressed(BUTTON_10)) {
      // Serial.println("Button 10 is pressed");
      prevTime = now;
      ws2812fx.setCustomMode(effect10);
      ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_CUSTOM, RED, 1, NO_OPTIONS);
    }
    else {
      // If no buttons are pressed for more than 100 milliseconds, activate "breathing" pattern in
      // the color that corresponds to the temperature reading
      if (now - prevTime > 100) {
        ws2812fx.setSegment(0, 0, PIXEL_COUNT - 1, FX_MODE_BREATH, color_wheel(tempToColor), 1,
                            true);
      }
    }
  }
}

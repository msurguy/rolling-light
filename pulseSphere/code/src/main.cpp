// Created by Maksim Surguy, Spring 2018

/*

   This code runs on WeMos D1 mini board with a pulse sensor connected to pin A0, 9 Neopixel LEDs
      connected to pin D2, a button connected to pin D3.

   The objective of this code is to read human heart rate and to pulsate lights according to the
      heart rate
   When the ball is not held, it simply stays blue

 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>

Ticker flipper;

//  VARIABLES
#define LEDCOUNT 9

int inputPin = D3; // pushbutton connected to digital pin D3
// int input2Pin = D10;               // pushbutton connected to digital pin D3
int buttonVal = 1; // variable to store the read value

int fadeRate = 0;  // used to fade LED on with PWM on fadePin

// these variables are volatile because they are used during the interrupt
// service routine!
volatile int BPM;                  // used to hold the pulse rate
volatile int Signal;               // holds the incoming raw data
volatile int IBI = 600;            // holds the time between beats, must be
                                   // seeded!
volatile boolean Pulse = false;    // true when pulse wave is high, false when
                                   // it's low
volatile boolean QS = false;       // becomes true when Arduoino finds a beat.

int rate[10];                      // array to hold last ten IBI values
unsigned long sampleCounter = 0;   // used to determine pulse timing
unsigned long lastBeatTime  = 0;   // used to find IBI
int P                       = 512; // used to find peak in pulse wave, seeded
int T                       = 512; // used to find trough in pulse wave,
                                   // seeded
int thresh = 512;                  // used to find instant moment of heart
                                   // beat, seeded
int amp = 100;                     // used to hold amplitude of pulse
                                   // waveform, seeded
boolean firstBeat = true;          // used to seed rate array so we startup
                                   // with reasonable BPM
boolean secondBeat = false;        // used to seed rate array so we startup
                                   // with reasonable BPM

// Set up the LED strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCOUNT, D2, NEO_GRB + NEO_KHZ800);

int i;

// THIS IS THE TICKER INTERRUPT SERVICE ROUTINE.
// Ticker makes sure that we take a reading every 2 miliseconds
void ISRTr() {                                    // triggered when flipper
                                                  // fires....
  cli();                                          // disable interrupts while we
                                                  // do this
  Signal         = analogRead(A0);                // read the Pulse Sensor
  sampleCounter += 2;                             // keep track of the time in
                                                  // mS with this variable
  int N = sampleCounter - lastBeatTime;           // monitor the time since the
                                                  // last beat to avoid noise

  //  find the peak and trough of the pulse wave
  if ((Signal < thresh) && (N > (IBI / 5) * 3)) { // avoid dichrotic noise by
                                                  // waiting 3/5 of last IBI
    if (Signal < T) {                             // T is the trough
      T = Signal;                                 // keep track of lowest point
                                                  // in pulse wave
    }
  }

  if ((Signal > thresh) && (Signal > P)) { // thresh condition helps avoid noise
    P = Signal;                            // P is the peak
  }                                        // keep track of highest point in
                                           // pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250) {                          // avoid high frequency noise
    if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3)) {
      Pulse = true;                       // set the Pulse flag when we
                                          // think there is a pulse
      // digitalWrite(blinkPin, HIGH);       // turn on pin 13 LED
      IBI = sampleCounter - lastBeatTime; // measure time between beats
                                          // in mS
      lastBeatTime = sampleCounter;       // keep track of time for
                                          // next pulse

      if (secondBeat) {                   // if this is the second
                                          // beat, if secondBeat ==
                                          // TRUE
        secondBeat = false;               // clear secondBeat flag

        for (int i = 0; i <= 9; i++) {    // seed the running total to
                                          // get a realisitic BPM at
                                          // startup
          rate[i] = IBI;
        }
      }

      if (firstBeat) {      // if it's the first time we found a beat, if
                            // firstBeat == TRUE
        firstBeat  = false; // clear firstBeat flag
        secondBeat = true;  // set the second beat flag
        sei();              // enable interrupts again
        return;             // IBI value is unreliable so discard it
      }


      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                // clear the runningTotal variable

      for (int i = 0; i <= 8; i++) {        // shift data in the rate array
        rate[i]       = rate[i + 1];        // and drop the oldest IBI value
        runningTotal += rate[i];            // add up the 9 oldest IBI values
      }

      rate[9] = IBI;                        // add the latest IBI to the rate
                                            // array
      runningTotal += rate[9];              // add the latest IBI to
                                            // runningTotal
      runningTotal /= 10;                   // average the last 10 IBI values
      BPM           = 60000 / runningTotal; // how many beats can fit into a
                                            // minute? that's BPM!
      QS = true;                            // set Quantified Self flag
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }
  }

  if ((Signal < thresh) && (Pulse == true)) { // when the values are going down,
                                              // the beat is over
    // digitalWrite(blinkPin, LOW);              // turn off pin 13 LED
    Pulse = false;                            // reset the Pulse flag so we can
                                              // do it again
    amp    = P - T;                           // get amplitude of the pulse wave
    thresh = amp / 2 + T;                     // set thresh at 50% of the
                                              // amplitude
    P = thresh;                               // reset these for next time
    T = thresh;
  }

  if (N > 2500) {                 // if 2.5 seconds go by without a beat
    thresh       = 512;           // set thresh default
    P            = 512;           // set P default
    T            = 512;           // set T default
    lastBeatTime = sampleCounter; // bring the lastBeatTime up to date
    firstBeat    = true;          // set these to avoid noise
    secondBeat   = false;         // when we get the heartbeat back
  }

  sei();                          // enable interrupts when youre done!
}// end isr

void interruptSetup() {
  // Initializes Ticker to have flipper run the ISR to sample every 2mS as per
  // original Sketch.
  flipper.attach_ms(2, ISRTr);
}

void setup() {
  Serial.begin(115200);
  pinMode(inputPin, INPUT); // set pin as input

  strip.setBrightness(255);
  strip.begin();

  // Play initialization animation
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 9; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      strip.setPixelColor(i, strip.Color(0, 0, 255));
      strip.show();
      delay(100);
    }
  }

  interruptSetup(); // sets up to read Pulse Sensor signal every 2mS
}

/* Returns a hexadecimal value of color wheel, based on input value ranging from
   0 to 255 */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;

  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }

  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Slightly different, this makes the rainbow equally distributed throughout the strip
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// After the heart beat is detected, dim the lights
void ledFadeToBeat() {
  fadeRate -= 0.25; //  set LED fade value

  for (int i = 0; i < 9; i++) {
    // strip.Color takes RGB values, from 0,0,0 up to 255,255,255
    // Let's map the fadeRate to some shade of red
    strip.setPixelColor(i, strip.Color(map(fadeRate, 0, 1024, 60, 255), 0, 0));
  }
  strip.show();
}

void loop() {
  unsigned long currentMillis = millis();

  buttonVal = digitalRead(inputPin); // read the button input pin

  // Sensor data
  if (QS == true) {                  // Quantified Self flag is true when arduino finds a heartbeat
    fadeRate = 1024;

    // Uncomment this line if want to see heart rate in beats per minute
    // Serial.println(BPM);
    QS = false; // reset the Quantified Self flag
    // for next time
  }

  // When button is not pressed, we are showing the heart rate or default animation
  if (buttonVal == 1) {
    if ((BPM > 50) && (BPM < 140)) {
      ledFadeToBeat();
    } else {
      for (int i = 0; i < 9; i++) {
        // for now set the whole strip as blue
        // TODO: animate into slow breathing pattern
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }
      strip.show();
    }
  }

  // When button on the ball is pressed, make a rainbow. Wohoo!
  if (buttonVal == 0) {
    rainbowCycle(10);
  }
}

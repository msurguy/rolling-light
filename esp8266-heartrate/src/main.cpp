#include <Arduino.h>

/*
   >> Pulse Sensor Amped 1.2 <<
   This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
    www.pulsesensor.com
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
   Pulse Sensor sample aquisition and processing happens in the background via
      Timer 2 interrupt. 2mS sample rate.
   PWM on pins 3 and 11 will not work when using this code, because we are using
      Timer 2!
   The following variables are automatically updated:
   Signal :    int that holds the analog signal data straight from the sensor.
      updated every 2mS.
   IBI  :      int that holds the time interval between beats. 2mS resolution.
   BPM  :      int that holds the heart rate value, derived every beat, from
      averaging previous 10 IBI values.
   QS  :       boolean that is made true whenever Pulse is found and BPM is
      updated. User must reset.
   Pulse :     boolean that is true when a heartbeat is sensed then false in
      time with pin13 LED going out.

   This code is designed with output serial data to Processing sketch
      "PulseSensorAmped_Processing-xx"
   The Processing sketch is a simple data visualizer.
   All the work to find the heartbeat and determine the heartrate happens in the
      code below.
   Pin 13 LED will blink with heartbeat.
   If you want to use pin 13 for something else, adjust the interrupt handler
   It will also fade an LED on pin fadePin with every beat. Put an LED and
      series resistor from fadePin to GND.
   Check here for detailed code walkthrough:
   http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

   Code Version 1.2 by Joel Murphy & Yury Gitman  Spring 2013
   This update fixes the firstBeat and secondBeat flag usage so that realistic
      BPM is reported.

   Environment Monitor modified for ESP8266

 */
#include <Ticker.h>
#include <Neosegment.h>


Ticker flipper;

//  VARIABLES
// int blinkPin = D4; // pin to blink led at each beat
int fadePin = D6; // pin to do fancy classy fading blink at
                  // each beat
int fadeRate = 0; // used to fade LED on with PWM on fadePin

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

#define nDigits 2
#define LEDPIN D4
#define BRIGHTNESS 100

int digitBuffer;
int digitIndex;

Neosegment neosegment(nDigits, LEDPIN, BRIGHTNESS);

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
  // pinMode(blinkPin, OUTPUT); // pin that will blink to your heartbeat!
  pinMode(fadePin, OUTPUT); // pin that will fade to your heartbeat!
  Serial.begin(115200);     // we agree to talk fast!

  neosegment.begin();
  neosegment.clearAll();

  interruptSetup(); // sets up to read Pulse Sensor signal every 2mS
  // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW
  // VOLTAGE,
  // AND APPLY THAT VOLTAGE TO THE A-REF PIN
  // analogReference(EXTERNAL);
}

/* Returns a hexadecimal value of color wheel, based on input value ranging from
   0 to 255 */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;

  if (WheelPos < 85) {
    return neosegment.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }

  if (WheelPos < 170) {
    WheelPos -= 85;
    return neosegment.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return neosegment.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void ledFadeToBeat() {
  fadeRate -= 1;                           //  set LED fade value
  fadeRate  = constrain(fadeRate, 0, 255); //  keep LED fade value from going
                                           // into negative numbers!
  // analogWrite(fadePin, fadeRate);          //  fade LED
  // neosegment.clearDigit(0);
  // neosegment.clearDigit(1);

  digitIndex  = nDigits - 1;
  digitBuffer = BPM; // Start with the whole string of numbers

  // Display every digit from the sensor reading on appropriate Neosegment Digit
  while (digitBuffer > 0)
  {
    int digit = digitBuffer % 10;

    // Write digit to Neosegment display in color that corresponds to the sensor
    // reading
    neosegment.setDigit(digitIndex, digit, neosegment.Color(fadeRate, 0, 0));

    digitBuffer /= 10;
    digitIndex--;
  }

  // Serial.println(fadeRate);
}

void sendDataToProcessing(char symbol, int data) {
  Serial.print(symbol); // symbol prefix tells Processing what type of data is
                        // coming
  Serial.println(data); // the data to send culminating in a carriage return
}

void loop() {
  // sendDataToProcessing('S', Signal);     // send Processing the raw Pulse
  // Sensor data
  if (QS == true) { // Quantified Self flag is true when
    // arduino finds a heartbeat
    fadeRate = 255; // Set 'fadeRate' Variable to 255 to
    // fade LED with pulse
    //        sendDataToProcessing('B',BPM);   // send heart rate with a 'B'
    // prefix
    //        sendDataToProcessing('Q',IBI);   // send time between beats with a
    // 'Q' prefix
    QS = false; // reset the Quantified Self flag
    // for next time
  }

  ledFadeToBeat();
}

//
// /*  PulseSensor™ Starter Project and Signal Tester
//  *  The Best Way to Get Started  With, or See the Raw Signal of, your
// PulseSensor™ & Arduino.
//  *
//  *  Here is a link to the tutorial
//  *  https://pulsesensor.com/pages/code-and-guide
//  *
//  *  WATCH ME (Tutorial Video):
//  *  https://www.youtube.com/watch?v=82T_zBZQkOE
//  *
//  *
// -------------------------------------------------------------
// 1) This shows a live human Heartbeat Pulse.
// 2) Live visualization in Arduino's Cool "Serial Plotter".
// 3) Blink an LED on each Heartbeat.
// 4) This is the direct Pulse Sensor's Signal.
// 5) A great first-step in troubleshooting your circuit and connections.
// 6) "Human-readable" code that is newbie friendly."
//
// */
// #include <Adafruit_NeoPixel.h>
//
// // On a Trinket or Gemma we suggest changing this to 1
// #define PIN            D4
//
// // How many NeoPixels are attached to the Arduino?
// #define NUMPIXELS      16
//
// // When we setup the NeoPixel library, we tell it how many pixels, and which
// pin to use to send signals.
// // Note that for older NeoPixel strips you might need to change the third
// parameter--see the strandtest
// // example for more information on possible values.
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB +
// NEO_KHZ800);
//
// //  Variables
// int PulseSensorPurplePin = A0;        // Pulse Sensor PURPLE WIRE connected
// to ANALOG PIN 0
// int LED13 = 13;   //  The on-board Arduion LED
//
//
// int Signal;                // holds the incoming raw data. Signal value can
// range from 0-1024
// int Threshold = 400;            // Determine which Signal to "count as a
// beat", and which to ingore.
//
//
// // The SetUp Function:
// void setup() {
//   pinMode(LED13,OUTPUT);         // pin that will blink to your heartbeat!
//    Serial.begin(9600);         // Set's up Serial Communication at certain
// speed.
//    pixels.begin(); // This initializes the NeoPixel library.
//
// }
//
// // The Main Loop Function
// void loop() {
//
//   Signal = analogRead(PulseSensorPurplePin);  // Read the PulseSensor's
// value.
//                                               // Assign this value to the
// "Signal" variable.
//
//    Serial.println(Signal);                    // Send the Signal value to
// Serial Plotter.
//
//
//    if(Signal > Threshold){                          // If the signal is above
// "550", then "turn-on" Arduino's on-Board LED.
//      for(int i=0;i<NUMPIXELS;i++){
//
//       // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
//       pixels.setPixelColor(i, pixels.Color(map(Signal, 500,700, 50,
// 255),0,0)); // Moderately bright green color.
//       pixels.show(); // This sends the updated pixel color to the hardware.
//
//     }
//    } else {
//      for(int i=0;i<NUMPIXELS;i++){
//
//       // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
//       pixels.setPixelColor(i, pixels.Color(map(400, 500,700, 50, 255),0,0));
// // Moderately bright green color.
//       pixels.show(); // This sends the updated pixel color to the hardware.
//
//      }
//   }
//
//
//
// delay(10);
//
//
// }


// /*
//    Sketch to handle each sample read from a PulseSensor.
//    Typically used when you don't want to use interrupts
//    to read PulseSensor voltages.
//
//    Here is a link to the tutorial that discusses this code
//    https://pulsesensor.com/pages/getting-advanced
//
//    Copyright World Famous Electronics LLC - see LICENSE
//    Contributors:
//      Joel Murphy, https://pulsesensor.com
//      Yury Gitman, https://pulsesensor.com
//      Bradford Needham, @bneedhamia, https://bluepapertech.com
//
//    Licensed under the MIT License, a copy of which
//    should have been included with this software.
//
//    This software is not intended for medical use.
// */
//
// /*
//    Every Sketch that uses the PulseSensor Playground must
//    define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
//    Here, #define USE_ARDUINO_INTERRUPTS false tells the library to
//    not use interrupts to read data from the PulseSensor.
//
//    If you want to use interrupts, simply change the line below
//    to read:
//      #define USE_ARDUINO_INTERRUPTS true
//
//    Set US_PS_INTERRUPTS to false if either
//    1) Your Arduino platform's interrupts aren't yet supported
//    by PulseSensor Playground, or
//    2) You don't wish to use interrupts because of the side effects.
//
//    NOTE: if US_PS_INTERRUPTS is false, your Sketch must
//    call pulse.sawNewSample() at least once every 2 milliseconds
//    to accurately read the PulseSensor signal.
// */
// #define USE_ARDUINO_INTERRUPTS false
// #define US_PS_INTERRUPTS false
// #include <PulseSensorPlayground.h>
//
// /*
//    The format of our output.
//
//    Set this to PROCESSING_VISUALIZER if you're going to run
//     the Processing Visualizer Sketch.
//     See
// https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer
//
//    Set this to SERIAL_PLOTTER if you're going to run
//     the Arduino IDE's Serial Plotter.
// */
// const int OUTPUT_TYPE = PROCESSING_VISUALIZER;
//
// /*
//    Pinout:
//      PIN_INPUT = Analog Input. Connected to the pulse sensor
//       purple (signal) wire.
//      PIN_BLINK = digital Output. Connected to an LED (and 220 ohm resistor)
//       that will flash on each detected pulse.
//      PIN_FADE = digital Output. PWM pin onnected to an LED (and resistor)
//       that will smoothly fade with each pulse.
//       NOTE: PIN_FADE must be a pin that supports PWM.
//        If USE_INTERRUPTS is true, Do not use pin 9 or 10 for PIN_FADE,
//        because those pins' PWM interferes with the sample timer.
// */
// const int PIN_INPUT = A0;
// const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
// const int PIN_FADE = 5;
// const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
//
// /*
//    samplesUntilReport = the number of samples remaining to read
//    until we want to report a sample over the serial connection.
//
//    We want to report a sample value over the serial port
//    only once every 20 milliseconds (10 samples) to avoid
//    doing Serial output faster than the Arduino can send.
// */
// byte samplesUntilReport;
// const byte SAMPLES_PER_SERIAL_SAMPLE = 10;
//
// /*
//    All the PulseSensor Playground functions.
// */
// PulseSensorPlayground pulseSensor;
//
// void setup() {
//   /*
//      Use 115200 baud because that's what the Processing Sketch expects to
// read,
//      and because that speed provides about 11 bytes per millisecond.
//
//      If we used a slower baud rate, we'd likely write bytes faster than
//      they can be transmitted, which would mess up the timing
//      of readSensor() calls, which would make the pulse measurement
//      not work properly.
//   */
//   Serial.begin(115200);
//
//   // Configure the PulseSensor manager.
//   pulseSensor.analogInput(PIN_INPUT);
//   pulseSensor.blinkOnPulse(PIN_BLINK);
//   pulseSensor.fadeOnPulse(PIN_FADE);
//
//   pulseSensor.setSerial(Serial);
//   pulseSensor.setOutputType(OUTPUT_TYPE);
//   pulseSensor.setThreshold(THRESHOLD);
//
//   // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
//   samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
//
//   // Now that everything is ready, start reading the PulseSensor signal.
//   if (!pulseSensor.begin()) {
//     /*
//        PulseSensor initialization failed,
//        likely because our Arduino platform interrupts
//        aren't supported yet.
//
//        If your Sketch hangs here, try changing USE_PS_INTERRUPT to false.
//     */
//     for(;;) {
//       // Flash the led to show things didn't work.
//       digitalWrite(PIN_BLINK, LOW);
//       delay(50);
//       digitalWrite(PIN_BLINK, HIGH);
//       delay(50);
//     }
//   }
// }
//
// void loop() {
//
//   /*
//      See if a sample is ready from the PulseSensor.
//
//      If USE_INTERRUPTS is true, the PulseSensor Playground
//      will automatically read and process samples from
//      the PulseSensor.
//
//      If USE_INTERRUPTS is false, this call to sawNewSample()
//      will, if enough time has passed, read and process a
//      sample (analog voltage) from the PulseSensor.
//   */
//   if (pulseSensor.sawNewSample()) {
//     /*
//        Every so often, send the latest Sample.
//        We don't print every sample, because our baud rate
//        won't support that much I/O.
//     */
//     if (--samplesUntilReport == (byte) 0) {
//       samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
//
//       pulseSensor.outputSample();
//
//       /*
//          At about the beginning of every heartbeat,
//          report the heart rate and inter-beat-interval.
//       */
//       if (pulseSensor.sawStartOfBeat()) {
//         pulseSensor.outputBeat();
//       }
//     }
//
//     /*******
//       Here is a good place to add code that could take up
//       to a millisecond or so to run.
//     *******/
//   }
//
//   /******
//      Don't add code here, because it could slow the sampling
//      from the PulseSensor.
//   ******/
// }

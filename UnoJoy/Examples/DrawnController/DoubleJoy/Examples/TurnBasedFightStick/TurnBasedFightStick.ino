/* Delayed Fight Stick
    Alan Chatham - 2012
    
    This code uses the UnoJoy library to implement a fight stick
     that has a 1 second delay between when it recieves the input
     and when it delivers that data to the console.
     
    It utilizes a ring buffer, where the input from the buttons
     is stored just behind where the output is drawn from, so
     the output will always be the least-fresh data available.
     In this way, every command the controller reads is preserved
     and sent out, just with a lag depending on the controller's
     rate of button polling and the size of the buffer.  
 */
#include "UnoJoy.h"

// This is the lag amount, in milliseconds
const int InputDuration = 1000;

// These are the pin definitions for the 
//  input buttons and sticks.
byte SquarePin = 4;
byte TrianglePin = 5;
byte CirclePin = 6;
byte CrossPin = 7;
byte StartPin = 8;
byte LEDPin = 2;
byte SpeakerPin = 3;

byte LeftStickXPin = A0;
byte LeftStickYPin = A1;

const int ControllerUpdateRate = 20;
const int RingBufferSize = InputDuration / ControllerUpdateRate;
dataForController_t ControllerDataRingBuffer[RingBufferSize];
int InputDataIndex = 0;
int OutputDataIndex = 0;

void toneSetup(){
  pinMode(SpeakerPin, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM20);
}

void toneLow(){
  OCR2A = 255;
  OCR2B = 128;
  TCCR2B = _BV(WGM22) | 5;
}

void toneHigh(){
  OCR2A = 255;
  OCR2B = 128;
  TCCR2B = _BV(WGM22) | 4;
}
void toneOff(){
  OCR2B = 255;
}

void setup(){
  // Set up button pins as input, internal pull-up on
  pinMode(SquarePin, INPUT);
  digitalWrite(SquarePin, HIGH);
  pinMode(TrianglePin, INPUT);
  digitalWrite(TrianglePin, HIGH);
  pinMode(CirclePin, INPUT);
  digitalWrite(CirclePin, HIGH);
  pinMode(CrossPin, INPUT);
  digitalWrite(CrossPin, HIGH);
  pinMode(StartPin, INPUT);
  digitalWrite(StartPin, HIGH);
  // Set up our LED pin as an output
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
  // Set up our ring buffer with zeroed controller data packets
  for (int i = 0; i < RingBufferSize; i++){
    ControllerDataRingBuffer[i] = getBlankDataForController();
  }
  // Set up our piezo buzzer
  toneSetup();
  // And set up UnoJoy communication
  setupUnoJoy();
}

void LEDOneSecondSlowBlink(void){
  digitalWrite(LEDPin, HIGH);
  delay(100);
  digitalWrite(LEDPin, LOW);
  delay(900);
}

void LEDOneSecondFastBlink(void){
  for (byte i = 0; i < 4; i++){
    digitalWrite(LEDPin, HIGH);
    delay(100);
    digitalWrite(LEDPin, LOW);
    delay(150);
  }
}

void loop(){
  // Start each loop by outputting a neutral controller signal
  setControllerData(getBlankDataForController());
  // Give 3 seconds of slow blink for thinking time
  //LEDOneSecondSlowBlink();
  //LEDOneSecondSlowBlink();
  //LEDOneSecondSlowBlink();
  // Give 1 second warning that input time is almost here
  LEDOneSecondFastBlink();
  
 // Spend an amount of time gathering input //
 //  (determined by InputDuration, assigned at the top of the file) //
  // LED goes on to show input mode
  digitalWrite(LEDPin, HIGH);
  toneLow();
  delay(75);
  toneOff();
  // Start at the beginning of our buffer
  InputDataIndex = 0;
  // Each update tick, take in and store the controller input
  while (InputDataIndex < RingBufferSize){
    delay(ControllerUpdateRate);
    ControllerDataRingBuffer[InputDataIndex] = readController();
    InputDataIndex++;
  }
  toneHigh();
  delay(75);
  toneOff();
  delay(425);
 // Spend an equal amount of time outputting that data //
  // LED goes on to show input mode
  digitalWrite(LEDPin, LOW);
  // Start at the beginning of our buffer
  OutputDataIndex = 0;
  // Each update tick, output the appropriate data from our buffer
  while (OutputDataIndex < RingBufferSize){
    delay(ControllerUpdateRate);
    setControllerData(ControllerDataRingBuffer[OutputDataIndex]);
    OutputDataIndex++;
  }
}


dataForController_t readController(){
  dataForController_t controllerData = getBlankDataForController();
  if (digitalRead(SquarePin) == LOW)
    controllerData.squareOn = 1;
  if (digitalRead(TrianglePin) == LOW)
    controllerData.triangleOn = 1;
  if (digitalRead(CirclePin) == LOW)
    controllerData.circleOn = 1;
  if (digitalRead(CrossPin) == LOW)
    controllerData.crossOn = 1;
  if (digitalRead(StartPin) == LOW)
    controllerData.startOn = 1;
    
  controllerData.leftStickX = analogRead(LeftStickXPin) >> 2;
  controllerData.leftStickY = analogRead(LeftStickYPin) >> 2;
  return controllerData;
}

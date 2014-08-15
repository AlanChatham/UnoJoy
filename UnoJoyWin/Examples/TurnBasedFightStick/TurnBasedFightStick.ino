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
#include "DoubleJoy.h"

// This is the lag amount, in milliseconds
const int InputDuration = 1000;

// These are the pin definitions for the 
//  input buttons and sticks.
byte Square1Pin = 4;
byte Triangle1Pin = 5;
byte Circle1Pin = 6;
byte Cross1Pin = 7;

byte Left1Pin = 8;
byte Up1Pin = 9;
byte Right1Pin = 10;
byte Down1Pin = 11;

byte Square2Pin = A0;
byte Triangle2Pin = A1;
byte Circle2Pin = A2;
byte Cross2Pin = A3;

byte Left2Pin = A4;
byte Up2Pin = A5;
byte Right2Pin = 12;
byte Down2Pin = 2;

byte LEDPin = 13;

byte SpeakerPin = 3;

byte inputPinArray[] = {Square1Pin, Triangle1Pin, Circle1Pin, Cross1Pin,
                        Left1Pin, Up1Pin, Right1Pin, Down1Pin,
                        Square2Pin, Triangle2Pin, Circle2Pin, Cross2Pin,
                        Left2Pin, Up2Pin, Right2Pin, Down2Pin };

const int ControllerUpdateRate = 20;
const int RingBufferSize = InputDuration / ControllerUpdateRate;
dataForController_t Controller1DataRingBuffer[RingBufferSize];
dataForController_t Controller2DataRingBuffer[RingBufferSize];

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
  for (int i = 0; i < sizeof(inputPinArray); i++) {
    pinMode(inputPinArray[i], INPUT);
    digitalWrite(inputPinArray[i], HIGH);
  }

  // Set up our LED pin as an output
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
  // Set up our ring buffer with zeroed controller data packets
  for (int i = 0; i < RingBufferSize; i++){
    Controller1DataRingBuffer[i] = getBlankDataForController();
    Controller2DataRingBuffer[i] = getBlankDataForController();
  }
  // Set up our piezo buzzer
  toneSetup();
  // And set up UnoJoy communication
  setupDoubleJoy();
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
  setControllerData(1, getBlankDataForController());
  setControllerData(2, getBlankDataForController());
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
    Controller1DataRingBuffer[InputDataIndex] = readController1();
    Controller2DataRingBuffer[InputDataIndex] = readController2();
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
    setControllerData(1, Controller1DataRingBuffer[OutputDataIndex]);
    setControllerData(2, Controller2DataRingBuffer[OutputDataIndex]);
    OutputDataIndex++;
  }
}


dataForController_t readController1(){
  dataForController_t controllerData = getBlankDataForController();
  if (digitalRead(Square1Pin) == LOW)
    controllerData.squareOn = 1;
  if (digitalRead(Triangle1Pin) == LOW)
    controllerData.triangleOn = 1;
  if (digitalRead(Circle1Pin) == LOW)
    controllerData.circleOn = 1;
  if (digitalRead(Cross1Pin) == LOW)
    controllerData.crossOn = 1;
    
  if (digitalRead(Up1Pin) == LOW)
    controllerData.dpadUpOn = 1;
  if (digitalRead(Down1Pin) == LOW)
    controllerData.dpadDownOn = 1;
  if (digitalRead(Left1Pin) == LOW)
    controllerData.dpadLeftOn = 1;
  if (digitalRead(Right1Pin) == LOW)
    controllerData.dpadRightOn = 1;
    
  return controllerData;
}
dataForController_t readController2(){
  dataForController_t controllerData = getBlankDataForController();
  if (digitalRead(Square2Pin) == LOW)
    controllerData.squareOn = 1;
  if (digitalRead(Triangle2Pin) == LOW)
    controllerData.triangleOn = 1;
  if (digitalRead(Circle2Pin) == LOW)
    controllerData.circleOn = 1;
  if (digitalRead(Cross2Pin) == LOW)
    controllerData.crossOn = 1;
    
  if (digitalRead(Up2Pin) == LOW)
    controllerData.dpadUpOn = 1;
  if (digitalRead(Down2Pin) == LOW)
    controllerData.dpadDownOn = 1;
  if (digitalRead(Left2Pin) == LOW)
    controllerData.dpadLeftOn = 1;
  if (digitalRead(Right2Pin) == LOW)
    controllerData.dpadRightOn = 1;
    
  return controllerData;
}

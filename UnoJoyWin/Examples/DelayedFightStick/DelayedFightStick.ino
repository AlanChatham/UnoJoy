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
const int ControllerLag = 1000;

// These are the pin definitions for the 
//  input buttons and sticks.
byte SquarePin = 2;
byte TrianglePin = 3;
byte CirclePin = 4;
byte CrossPin = 5;

byte LeftStickXPin = A0;
byte LeftStickYPin = A1;

const int ControllerUpdateRate = 50;
const int RingBufferSize = ControllerLag / ControllerUpdateRate;

dataForController_t ControllerDataRingBuffer[RingBufferSize];
int InputDataIndex = 0;
int OutputDataIndex = 1;

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
  // Set up our ring buffer with zeroed controller data packets
  for (int i = 0; i < RingBufferSize; i++){
    ControllerDataRingBuffer[i] = getBlankDataForController();
  }
  // And set up UnoJoy communication
  setupUnoJoy();
}

void loop(){
  // Update our data at a certain interval
  delay(ControllerUpdateRate);
  ControllerDataRingBuffer[InputDataIndex] = readController();
  setControllerData(ControllerDataRingBuffer[OutputDataIndex]);
  
  InputDataIndex++;
  if (InputDataIndex >= RingBufferSize)
    InputDataIndex = 0;
    
  OutputDataIndex++;
  if (OutputDataIndex >= RingBufferSize)
    OutputDataIndex = 0;
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
  controllerData.leftStickX = analogRead(LeftStickXPin);
  controllerData.leftStickY = analogRead(LeftStickYPin);
  return controllerData;
}

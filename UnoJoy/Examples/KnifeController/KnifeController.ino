/* Knife-based controller for Assassin's Creed
 *
 *   Button in knife handle activates stabbing,
 *   Analog stick handles movement
 *   Button in analog stick handles dismount/blending
 *
 */

#include "UnoJoy.h" 

int StabPin = 2;
int JoyButtonPin = 3;
int JoyXPin = A0;
int JoyYPin = A1;
 
void setup(void){
  setupPins();
  setupUnoJoy();
}

void setupPins(void){
  pinMode(StabPin, INPUT);
  digitalWrite(StabPin, HIGH);
  pinMode(JoyButtonPin, INPUT);
  digitalWrite(JoyButtonPin, HIGH);
}

unsigned long JoyButtonPinTime = 0;

dataForController_t readInput(void){
  // Start with a new controller packet
  dataForController_t controllerData = getBlankDataForController();
  
  // Select your hidden knife every 20 seconds
  if ((millis() % 20000) < 100)
    controllerData.dpadUpOn = 1;
  
  // Check the buttons
  if (digitalRead(StabPin) == LOW)
    controllerData.squareOn = 1;
  if (digitalRead(JoyButtonPin) == LOW){
    // Turn the X button on
    controllerData.crossOn = 1;
    // and if we've held it for a bit, press circle too
    if (millis() - JoyButtonPinTime > 100)
      controllerData.circleOn = 1;
  }
  else{ // Joystick button is unpressed
    // Reset the joystick button timer
    JoyButtonPinTime = millis();
    // Turn on running
    controllerData.r1On = 1;
  }
  // Read the analog sticks
  controllerData.leftStickX = 255 - (analogRead(JoyXPin) >> 2);
  controllerData.leftStickY = 255 - (analogRead(JoyYPin) >> 2);
  
  return controllerData;
}

void loop(void){
  setControllerData(readInput());
}

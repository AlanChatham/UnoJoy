/* This is a controller for VVVVVV that you hold in your
 *  hands, tilting to move and flipping over to flip the screen.
 */

#include "UnoJoy.h"

#define FlipPin 3
#define LeftPin 2
#define RightPin 4


void setup(void){
  setupPins();
  setupUnoJoy();
}

void loop(void){
  // We update 10 times a second, and so our
  //  flip signal lasts that long
  delay(100);
  setControllerData(readInput());
}

void setupPins(void){
  pinMode(FlipPin, INPUT);
  digitalWrite(FlipPin, HIGH);
  pinMode(LeftPin, INPUT);
  digitalWrite(LeftPin, HIGH);
  pinMode(RightPin, INPUT);
  digitalWrite(RightPin, HIGH);
}


int UP   = true;
int DOWN = false;

boolean currentOrientation = DOWN; 



dataForController_t readInput(void){
  
  // create empty controller data object
  dataForController_t controllerData = getBlankDataForController();
  
  
  // check whether the orientation has changed
  if (digitalRead(FlipPin) != currentOrientation){
    // Debounce
    delay(20);
    // if the orientation is still different, lets flip!
    if (digitalRead(FlipPin) != currentOrientation) {
      currentOrientation = digitalRead(FlipPin);
      controllerData.triangleOn = 1;
    }
  }
  
  // Check for left & right
  
  // if our orientation is DOWN...
  if( currentOrientation == DOWN ) {
    
    // left tilt triggers left arrow
    if (digitalRead(LeftPin) == LOW)
      controllerData.squareOn = 1;
    // right tilt triggers right arrow
    if (digitalRead(RightPin) == LOW)
      controllerData.circleOn = 1;
  } 
  // if it's UP...
  else {
    // left tilt triggers RIGHT arrow
    if (digitalRead(LeftPin) == HIGH)
      controllerData.circleOn = 1;
    // right tilt triggers LEFT arrow
    if (digitalRead(RightPin) == HIGH)
      controllerData.squareOn = 1;
  }
  
  return controllerData;
}


/* Official Sample Code for DoubleJoy
     2012 - Alan Chatham
  
    This code, in conjunction with the DoubleJoy firmware,
     implements two PS3/PC/OSX compatible arcade joysticks.
     Well suited to projects like MAME cabinets and such.
     
    The buttons are mapped to pins, with the only tricky part
    being that, due to a lack of pins, the activating the
    Start button requires the player to hold down two buttons -
    the one shared start button and their own Cross button.
*/
#include "DoubleJoy.h"

void setup(){
  setupPins();
  setupDoubleJoy();
}

void loop(){
  // Always be getting fresh data
  dataForController_t controller1Data = readController1();
  dataForController_t controller2Data = readController2();
  setControllerData(1, controller1Data);
  setControllerData(2, controller2Data);
}

void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (int i = 2; i <= 12; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  pinMode(A0, INPUT);
  digitalWrite(A0, HIGH);
  pinMode(A1, INPUT);
  digitalWrite(A1, HIGH);
  pinMode(A2, INPUT);
  digitalWrite(A2, HIGH);
  pinMode(A3, INPUT);
  digitalWrite(A3, HIGH);
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
}

dataForController_t readController1(void){
  
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.triangleOn = !digitalRead(2);
  controllerData.circleOn = !digitalRead(3);
  controllerData.squareOn = !digitalRead(4);
  // Block Cross from working if start is being held
  if (digitalRead(10))
    controllerData.crossOn = !digitalRead(5);
  
  controllerData.dpadUpOn = !digitalRead(6);
  controllerData.dpadDownOn = !digitalRead(7);
  controllerData.dpadLeftOn = !digitalRead(8);
  controllerData.dpadRightOn = !digitalRead(9);
  
   if (!digitalRead(10) && !digitalRead(5) )
    controllerData.startOn = 1;
  
  // And return the data!
  return controllerData;
}

// Just like readController1, but with different pins
//  and 'Start' is activated by Cross and p1 start
dataForController_t readController2(void){
  
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.triangleOn = !digitalRead(A0);
  controllerData.circleOn = !digitalRead(A1);
  controllerData.squareOn = !digitalRead(A2);
  // Block Cross from working if start is being held
  if (digitalRead(10))
    controllerData.crossOn = !digitalRead(A3);
  
  controllerData.dpadUpOn = !digitalRead(A4);
  controllerData.dpadDownOn = !digitalRead(A5);
  controllerData.dpadLeftOn = !digitalRead(11);
  controllerData.dpadRightOn = !digitalRead(12);
  
  if (!digitalRead(10) && !digitalRead(A3) )
    controllerData.startOn = 1;
    
  // And return the data!
  return controllerData;
}


#include "UnoJoy.h"

void setup(){
  setupPins();
  setupUnoJoy();
}

#define LEFT 0
#define RIGHT 1

int LeftPadPin = 2;
int RightPadPin = 3;
int LastPadHit = LEFT;
// These hold when the pad started to be
//  held down, for counting debounce delay.
unsigned long LeftPadHitTimer = 0;
unsigned long RightPadHitTimer = 0;

// How long a button needs to be held down to register
int DebounceDelay = 10;
// LeftStickX holds steering data that persists across loops
uint8_t LeftStickX = 128;
// RightStickY holds accelerator data that persists across loops
uint8_t RightStickY = 128;
//  How often (in ms) to lower the acceleration by 1
int SlowDownPeriod = 100;
int SpeedUpAmount = 20;
// The first time we step to turn, we move the stick further than usual
//  so we get a faster response
int FirstTurnAmount = 50;
// This is how much more extreme we make the turn with every iteration.
int TurnIncrement = 10;

void loop(){
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  
  // First off, slow down the controller every so often
  if ((millis() % SlowDownPeriod) == 0){
    if (RightStickY > 85){
      RightStickY -= 1;
    }
  }
  
  // Then check the buttons to see if their state has 
  //  changed
  if (digitalRead(LeftPadPin) == LOW){
    // Only record a button hit if it's been at least
    //  a certain amount of time since it was held down
    if ((millis() - LeftPadHitTimer) > DebounceDelay){
      // We set LeftPadHitTimer to zero after registering
      //  a hit, so we'll only do this bit once per press
      if (LeftPadHitTimer > 0){
        // Speed up
        if (RightStickY < (255 - SpeedUpAmount))
          RightStickY += SpeedUpAmount;
        // And set the steering properly
        if ((LastPadHit == LEFT) && (LeftStickX > TurnIncrement)){
          // If we were neutral, let's start with a pretty decent turn
          if ( LeftStickX > 125)
            LeftStickX = 128 - FirstTurnAmount;
          else
            LeftStickX -= TurnIncrement;
        }
        if (LastPadHit == RIGHT)
          LeftStickX = 128;
        LastPadHit = LEFT;
        // Finally, set the timer to zero so we
        //  don't hit this code again until a new press
        LeftPadHitTimer = 0;
      }
    }
  }
  // If we didn't press the pad, then reset the timer
  if (digitalRead(LeftPadPin) == HIGH)
    LeftPadHitTimer = millis();
  // End of Left Pad
    
  if (digitalRead(RightPadPin) == LOW){
    // Only record a button hit if it's been at least
    //  a certain amount of time since it was held down
    if ((millis() - RightPadHitTimer) > DebounceDelay){
      // We set LeftPadHitTimer to zero after registering
      //  a hit, so we'll only do this bit once per press
      if (RightPadHitTimer > 0){
        // Speed up
        if (RightStickY < (255 - SpeedUpAmount))
          RightStickY += SpeedUpAmount;
        // And set the steering properly
        if ((LastPadHit == RIGHT) && (LeftStickX < (255 - TurnIncrement))){
          if ( LeftStickX < 125)
            LeftStickX = 128 + FirstTurnAmount;
          else
            LeftStickX += TurnIncrement;
        }
        if (LastPadHit == LEFT)
          LeftStickX = 128;
        LastPadHit = RIGHT;
        // Finally, set the timer to zero so we
        //  don't hit this code again until a new press
        RightPadHitTimer = 0;
      }
    }
  }
  // If we didn't press the pad, then reset the timer
  if (digitalRead(RightPadPin) == HIGH)
    RightPadHitTimer = millis();
  // End of Right Pad
  
  /*if ((millis() % 500) == 0){
    Serial.print("Left Stick X: ");
    Serial.println(LeftStickX);
    Serial.print("Right Stick Y: ");
    Serial.println(RightStickY);
  }*/
  controllerData.leftStickX = LeftStickX;
  controllerData.rightStickY = 255 - RightStickY;
  setControllerData(controllerData);
 
}

void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (int i = 2; i <= 12; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
}

dataForController_t getControllerData(void){
  // Set up a place for our controller data
  dataForController_t controllerData;
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.triangleOn = 0;//!digitalRead(2);
  controllerData.circleOn = 0;//!digitalRead(3);
  controllerData.squareOn = 0;//!digitalRead(4);
  controllerData.crossOn = 0;//!digitalRead(5);
  controllerData.dpadUpOn = !digitalRead(6);
  controllerData.dpadDownOn = !digitalRead(7);
  controllerData.dpadLeftOn = !digitalRead(8);
  controllerData.dpadRightOn = !digitalRead(9);
  controllerData.r2On = 0;//!digitalRead(10);
  controllerData.r3On = 0;//!digitalRead(11);
  controllerData.selectOn = 0;//!digitalRead(12);
  controllerData.startOn = 0;//!digitalRead(A4);
  
  // Fill up the other buttons with zeroes - otherwise they
  //  tend to be filled with random memory junk.
  // And we take the analog stick data from
  //  the analog pins.
  controllerData.l1On = 0;
  controllerData.l2On = 0;
  controllerData.l3On = 0;
  controllerData.r1On = 0;
  controllerData.homeOn = 0;//!digitalRead(A5);
 // controllerData.dpadLeftOn = 0;
  //controllerData.dpadUpOn = 0;
  //controllerData.dpadRightOn = 0;
  //controllerData.dpadDownOn = 0;
  
  controllerData.leftStickX = 128;
  controllerData.leftStickY = 128;
  controllerData.rightStickX = 128;
  controllerData.rightStickY = 128;
  // And return the data!
  return controllerData;
}

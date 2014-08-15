/* Example code for a controller made with capacitive inputs.
    Each input should have a 1 Megaohm resistor pulling it up
    to +5v for the sensing to work well
    
*/
#include "UnoJoy.h"

// We define our pins here
#define UP_PIN 2
#define RIGHT_PIN 11
#define LEFT_PIN 3
#define DOWN_PIN 10
#define SQUARE_PIN 4
#define TRIANGLE_PIN 5
#define CIRCLE_PIN 9
#define CROSS_PIN 8


#define START_PIN 6
#define HOME_PIN 7

// This array just helps us set our pins easily
#define NUMBER_OF_INPUTS 10
int pinArray[NUMBER_OF_INPUTS] = {UP_PIN, RIGHT_PIN, LEFT_PIN, DOWN_PIN,
                   TRIANGLE_PIN, CIRCLE_PIN, SQUARE_PIN, CROSS_PIN,
                   START_PIN, HOME_PIN};
                   
// This stores the baseline cutoff value for the
//  capacitive sensors on each pin
int baselineArray[20];

void setup(){
  setupUnoJoy();
  // Give things a chance to settle after power-up
  delay(20);
  // This function establishes the baseline reading
  //  for each capacitive sensor, effectively calibrating it
  setupBaselines();
}

void loop(){
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);
 
}

// This function reads an average of each capacitive pin,
//  calculates an appropriate cutoff value for the 'touched'
//  condition, and stores that value.
void setupBaselines(void){
  for (byte i = 0; i < NUMBER_OF_INPUTS; i++){
    int baseReading = calibrateCapacitivePin(pinArray[i]);
    baselineArray[pinArray[i]] = baseReading + (baseReading / 4);
  }
}

// This function takes an average of 8 readings on a
//  capacitive sensor pin and returns that.  It's used
//  to establish a baseline value for each sensor.
int calibrateCapacitivePin(int pinNumber){
  readCapacitivePinModified(pinNumber);
  delay(1);
  int averagingTotal = 0;
  for (int i = 0; i < 8; i++){
    int reading = readCapacitivePinModified(pinNumber);
    averagingTotal += reading;
    delay(1);
  }
  return averagingTotal / 8;
  delay(10);
}

// This function reads each of the capacitive pins, and if
//  the value is over the cutoff for a touch, assigns the
//  appropriate button as pressed.
dataForController_t getControllerData(void){
  // Set up a place for our controller data
  dataForController_t controllerData = getBlankDataForController();
 
  // Compare the reading of each capacitive pin to the cutoff we
  // established at the start
  if (readCapacitivePinModified(DOWN_PIN) > baselineArray[DOWN_PIN])
    controllerData.dpadDownOn = 1;

  if (readCapacitivePinModified(UP_PIN) > baselineArray[UP_PIN])
    controllerData.dpadUpOn = 1;
    
  if (readCapacitivePinModified(LEFT_PIN) > baselineArray[LEFT_PIN])
    controllerData.dpadLeftOn = 1;
    
  if (readCapacitivePinModified(RIGHT_PIN) > baselineArray[RIGHT_PIN])
    controllerData.dpadRightOn = 1;
    
  if (readCapacitivePinModified(SQUARE_PIN) > baselineArray[SQUARE_PIN])
    controllerData.squareOn = 1;
    
  if (readCapacitivePinModified(TRIANGLE_PIN) > baselineArray[TRIANGLE_PIN])
    controllerData.triangleOn = 1;
    
  if (readCapacitivePinModified(CIRCLE_PIN) > baselineArray[CIRCLE_PIN])
    controllerData.circleOn = 1;
  
  if (readCapacitivePinModified(CROSS_PIN) > baselineArray[CROSS_PIN])
    controllerData.crossOn = 1;
    
  // And return the data!
  return controllerData;
}


// readCapacitivePin
//  Input: Arduino pin number
//  Output: A number, from 0 to 17 expressing
//          how much capacitance is on the pin
//  When you touch the pin, or whatever you have
//  attached to it, the number will get higher
//  In order for this to work now,
//  THE PIN MUST HAVE A BEEFY PULL-UP RESISTOR
uint8_t readCapacitivePinModified(int pinToMeasure){
  // This is how you declare a variable which
  //  will hold the PORT, PIN, and DDR registers
  //  on an AVR
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  // Here we translate the input pin number from
  //  Arduino pin number to the AVR PORT, PIN, DDR,
  //  and which bit of those registers we care about.
  byte bitmask;
  if ((pinToMeasure >= 0) && (pinToMeasure <= 7)){
    port = &PORTD;
    ddr = &DDRD;
    bitmask = 1 << pinToMeasure;
    pin = &PIND;
  }
  if ((pinToMeasure > 7) && (pinToMeasure <= 13)){
    port = &PORTB;
    ddr = &DDRB;
    bitmask = 1 << (pinToMeasure - 8);
    pin = &PINB;
  }
  if ((pinToMeasure > 13) && (pinToMeasure <= 19)){
    port = &PORTC;
    ddr = &DDRC;
    bitmask = 1 << (pinToMeasure - 13);
    pin = &PINC;
  }
  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  // Make the pin an input WITHOUT the internal pull-up on
  *ddr &= ~(bitmask);
  // Now see how long the pin to get pulled up
  int cycles = 16000;
  for(int i = 0; i < cycles; i++){
    if (*pin & bitmask){
      cycles = i;
      break;
    }
  }
  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  
  return cycles;
}


// This is a debugging function to easily see what
//  values the pins are returning.
void printPins(){
  Serial.print("2: ");
  Serial.println(readCapacitivePinModified(2));
  Serial.print("3: ");
  Serial.println(readCapacitivePinModified(3));
  Serial.print("4: ");
  Serial.println(readCapacitivePinModified(4));
  Serial.print("5: ");
  Serial.println(readCapacitivePinModified(5));
  Serial.print("9: ");
  Serial.println(readCapacitivePinModified(9));
  Serial.print("10: ");
  Serial.println(readCapacitivePinModified(10));
  Serial.print("11: ");
  Serial.println(readCapacitivePinModified(11));
  Serial.print("12: ");
  Serial.println(readCapacitivePinModified(12));
  Serial.println();
}


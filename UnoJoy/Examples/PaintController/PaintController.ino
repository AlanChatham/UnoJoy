/* Example code for a controller made with capacitive inputs.
    Each input should have a 1 Megaohm resistor pulling it up
    to +5v for the sensing to work well
    
*/
#include "UnoJoy.h"

int PressCutoff = 10;
int ReleaseCutoff = 1;

// We define our pins here
#define UP_PIN 11
#define RIGHT_PIN 8
#define LEFT_PIN 7
#define DOWN_PIN 4
#define SQUARE_PIN 9
#define TRIANGLE_PIN 12
#define CIRCLE_PIN 2
#define CROSS_PIN 10
#define L1_PIN 6
#define L2_PIN 5
#define R1_PIN 3
#define R2_PIN 2


#define START_PIN 6
#define HOME_PIN 7

// This array just helps us set our pins easily
#define NUMBER_OF_INPUTS 12
int pinArray[NUMBER_OF_INPUTS] = {UP_PIN, RIGHT_PIN, LEFT_PIN, DOWN_PIN,
                   TRIANGLE_PIN, CIRCLE_PIN, SQUARE_PIN, CROSS_PIN,
                   START_PIN, HOME_PIN, L1_PIN, R1_PIN};
                   
// This stores the baseline cutoff value for the
//  capacitive sensors on each pin
int baselineArray[20];

int lastPinValues[20];
dataForController_t ControllerData;

void setup(){
  ControllerData = getBlankDataForController();
  setupUnoJoy();
  // Give things a chance to settle after power-up
  delay(200);
  // This function establishes the baseline reading
  //  for each capacitive sensor, effectively calibrating it
  setupBaselines();
  for (int i = 0; i < 20; i++){
    lastPinValues[i] = baselineArray[i];
  }
}

void loop(){
  // Always be getting fresh data
  getControllerData();
  setControllerData(ControllerData);
  delay(100);
  //if ((millis() % 250) < 5){
  //  refreshBaselines(40);
  //  delay(5);
  //printPins();
  //}
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

void refreshBaselines(int cutoff){
  for (byte i = 0; i < NUMBER_OF_INPUTS; i++){
    int baseReading = readCapacitivePinModified(pinArray[i]);
    int newValue = baseReading + (baseReading / 4);
    if ((newValue - baselineArray[pinArray[i]]) < cutoff)
      baselineArray[pinArray[i]] = newValue;
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

int readPin(int pin, int previousValue){
  int returnValue = previousValue;
  int reading = 0;
  int reading2 = 0;
  int reading3 = 0;
  int reading4 = 0;
  // Compare the reading of each capacitive pin to the cutoff we
  // established at the start
  reading = readCapacitivePinModified(pin) / 4;
  reading2 = readCapacitivePinModified(pin) / 4;
  reading3 = readCapacitivePinModified(pin) / 4;
  reading4 = readCapacitivePinModified(pin) / 4;
  reading = reading + reading2 + reading3 + reading4;
  if (reading > (lastPinValues[pin] + PressCutoff))
    returnValue = 1;
  else if (reading < (lastPinValues[pin] - ReleaseCutoff))
    returnValue = 0;
  lastPinValues[pin] = reading;
  return returnValue;
}

// This function reads each of the capacitive pins, and if
//  the value is over the cutoff for a touch, assigns the
//  appropriate button as pressed.
void getControllerData(void){
  
  ControllerData.dpadDownOn = readPin(DOWN_PIN, ControllerData.dpadDownOn);
  ControllerData.dpadUpOn = readPin(UP_PIN, ControllerData.dpadUpOn);
  ControllerData.dpadLeftOn = readPin(LEFT_PIN, ControllerData.dpadLeftOn);
  ControllerData.dpadRightOn = readPin(RIGHT_PIN, ControllerData.dpadRightOn);
  
  ControllerData.leftStickY = 128;
  ControllerData.leftStickX = 128;
  
  if (ControllerData.dpadDownOn == 1)
    ControllerData.leftStickY = 245;
  if (ControllerData.dpadUpOn == 1)
    ControllerData.leftStickY = 10;
  if (ControllerData.dpadLeftOn == 1)
    ControllerData.leftStickX = 10;
  if (ControllerData.dpadRightOn == 1)
    ControllerData.leftStickX = 245;
    
  ControllerData.circleOn = readPin(CIRCLE_PIN, ControllerData.circleOn);
  ControllerData.crossOn = readPin(CROSS_PIN, ControllerData.crossOn);
  ControllerData.triangleOn = readPin(TRIANGLE_PIN, ControllerData.triangleOn);
  ControllerData.squareOn = readPin(SQUARE_PIN, ControllerData.squareOn);
  ControllerData.l1On = readPin(L1_PIN, ControllerData.l1On);
  ControllerData.r1On = readPin(R1_PIN, ControllerData.r1On);
  
  
  return;
}


// readCapacitivePin
//  Input: Arduino pin number
//  Output: A number, from 0 to 17 expressing
//          how much capacitance is on the pin
//  When you touch the pin, or whatever you have
//  attached to it, the number will get higher
//  In order for this to work now,
//  THE PIN MUST HAVE A BEEFY PULL-UP RESISTOR
int readCapacitivePinModified(int pinToMeasure){
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
  Serial.print("Baseline: ");
  Serial.print(baselineArray[2]);
  Serial.print("   2: ");
  Serial.println(readCapacitivePinModified(2));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[3]);
  Serial.print("   3: ");
  Serial.println(readCapacitivePinModified(3));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[4]);
  Serial.print("   4: ");
  Serial.println(readCapacitivePinModified(4));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[5]);
  Serial.print("   5: ");
  Serial.println(readCapacitivePinModified(5));
  
  Serial.print("Baseline: ");
  Serial.print(baselineArray[6]);
  Serial.print("   6: ");
  Serial.println(readCapacitivePinModified(6));
  
  Serial.print("Baseline: ");
  Serial.print(baselineArray[7]);
  Serial.print("   7: ");
  Serial.println(readCapacitivePinModified(7));
  
  Serial.print("Baseline: ");
  Serial.print(baselineArray[8]);
  Serial.print("   8: ");
  Serial.println(readCapacitivePinModified(8));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[9]);
  Serial.print("   9: ");
  Serial.println(readCapacitivePinModified(9));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[10]);
  Serial.print("   10: ");
  Serial.println(readCapacitivePinModified(10));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[11]);
  Serial.print("   11: ");
  Serial.println(readCapacitivePinModified(11));
  Serial.print("Baseline: ");
  Serial.print(baselineArray[12]);
  Serial.print("   12: ");
  Serial.println(readCapacitivePinModified(12));
  Serial.println();
}


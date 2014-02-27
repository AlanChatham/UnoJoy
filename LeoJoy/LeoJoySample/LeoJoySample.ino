/*
  LeoJoy Sample Code
  Alan Chatham - 2013
  unojoy.com
  
  Hardware - Attach buttons to any of the pins 2-12, A4 and A5
             to ground, and see different controller buttons
             be pressed.
             Analog stick controls can be enabled by removing
             some comments at the bottom, since it's way
             easier to test just with some wire and not have
             to go drag out some pots.
      
 
 This code is in the public domain
 */

/* Usage of LeoJoy
  The dataForController_t struct is the core of the library.
  You'll create an instance of that and manipulate it,
   then use the Joystick.setControllerData function to send that data out.
   
  The two main functions that LeoJoy! adds are
   - Joystick.setControllerData(dataForController_t)
      This actually updates the USB controller. Edit the dataForController_t struct
       as much as you like before pushing it out the USB interface with this function.
   
   - getBlankDataForController()
      This utility function returns a dataForController_t object, with no buttons pressed
       and both of the analog sticks centered. Handy!
  
  So, basically, you'll do something like this:
  
  void loop(void){
    // Set up an empty controller data struct
    dataForController_t joystickData = getBlankDataForController();
    // Set some of the buttons
    joystickData.squareOn = someBooleanInput();
    // Send the data over USB to the PC
    Joystick.setControllerData(joystickData);
  }
  
  The fields of the dataForController_t stuct are below.
  The struct is hard-coded into the USB communications library of LeoJoy,
   so it's not designed to be changed.

typedef struct dataForController_t
{
  uint8_t triangleOn : 1;  // Each of these member variables
  uint8_t circleOn : 1;    //  control if a button is off or on
  uint8_t squareOn : 1;    // For the buttons, 
  uint8_t crossOn : 1;     //  0 is off
  uint8_t l1On : 1;        //  1 is on
  uint8_t l2On : 1;        
  uint8_t l3On : 1;        // The : 1 here just tells the compiler
  uint8_t r1On : 1;        //  to only have 1 bit for each variable.
                   //  This saves a lot of space for our type!
  uint8_t r2On : 1;
  uint8_t r3On : 1;
  uint8_t selectOn : 1;
  uint8_t startOn : 1;
  uint8_t homeOn : 1;
  uint8_t dpadLeftOn : 1;
  uint8_t dpadUpOn : 1;
  uint8_t dpadRightOn : 1;

  uint8_t dpadDownOn : 1;
  uint8_t padding : 7;     // We end with 7 bytes of padding to make sure we get our data aligned in bytes
                   
  uint8_t leftStickX : 8;  // Each of the analog stick values can range from 0 to 255
  uint8_t leftStickY : 8;  //  0 is fully left or up
  uint8_t rightStickX : 8; //  255 is fully right or down 
  uint8_t rightStickY : 8; //  128 is centered.
                   // Important - analogRead(pin) returns a 10 bit value, so if you're getting strange
                   //  results from analogRead, you may need to do (analogRead(pin) >> 2) to get good data
} dataForController_t;
    

*/
void setup(){
  setupPins();
}

void loop(){
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  // Then send out the data over the USB connection
  // Joystick.set(controllerData) also works.
  Joystick.setControllerData(controllerData);
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
  controllerData.crossOn = !digitalRead(5);
  controllerData.dpadUpOn = !digitalRead(6);
  controllerData.dpadDownOn = !digitalRead(7);
  controllerData.dpadLeftOn = !digitalRead(8);
  controllerData.dpadRightOn = !digitalRead(9);
  controllerData.l1On = !digitalRead(10);
  controllerData.r1On = !digitalRead(11);
  controllerData.selectOn = !digitalRead(12);
  controllerData.startOn = !digitalRead(A4);
  controllerData.homeOn = !digitalRead(A5);
  
  // Set the analog sticks
  //  Since analogRead(pin) returns a 10 bit value,
  //  we need to perform a bit shift operation to
  //  lose the 2 least significant bits and get an
  //  8 bit number that we can use  
  controllerData.leftStickX = 128;// analogRead(A0) >> 2;
  controllerData.leftStickY = 128;//analogRead(A1) >> 2;
  controllerData.rightStickX = 128;//analogRead(A2) >> 2;
  controllerData.rightStickY = 128;//analogRead(A3) >> 2;
  // And return the data!
  return controllerData;
}

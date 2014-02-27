/*  UnoJoy.h
 *   Alan Chatham - 2012
 *    RMIT Exertion Games Lab
 *
 *  This library gives you a standard way to create Arduino code that talks
 *   to the UnoJoy firmware in order to make native USB game controllers.
 *  Functions:
 *   setupMegaJoy()
 *   getBlankDataForController()
 *   setControllerData(megaJoyControllerData_t dataToSet)
 *
 *   NOTE: You cannot use pins 0 or 1 if you use this code - they are used by the serial communication.
 *         Also, the setupMegaJoy() function starts the serial port at 38400, so if you're using
 *         the serial port to debug and it's not working, this may be your problem.
 *   
 *   === How to use this library ===
 *   If you want, you can move this file into your Arduino/Libraries folder, then use it like a normal library.
 *   However, since you'll need to refer to the details of the megaJoyControllerData_t struct in this file, I would suggest you use
 *    it by adding it to your Arduino sketch manually (in Arduino, go to Sketch->Add file...)
 *
 *  To use this library to make a controller, you'll need to do 3 things:
 *   Call setupMegaJoy(); in the setup() block
 *   Create and populate a megaJoyControllerData_t type variable and fill it with your data
 *         The getBlankDataForController() function is good for that.
 *   Call setControllerData(yourData); where yourData is the variable from above,
 *         somewhere in your loop(), once you're ready to push your controller data to the system.
 *         If you forget to call sendControllerData in your loop, your controller won't ever do anything
 *
 *  You can then debug the controller with the included Processing sketch, UnoJoyProcessingVisualizer
 *  
 *  To turn it into an actual USB video game controller, you'll reflash the
 *   Arduino's communication's chip using the instructions found in the 'Firmware' folder,
 *   then unplug and re-plug in the Arduino. 
 * 
 *  Details about the megaJoyControllerData_t type are below, but in order to create and use it,
 *   you'll declare it like:
 *
 *      megaJoyControllerData_t sexyControllerData;
 *
 *   and then control button presses and analog stick movement with statements like:
 *      
 *      sexyControllerData.triangleOn = 1;   // Marks the triangle button as pressed
 *      sexyControllerData.squareOn = 0;     // Marks the square button as unpressed
 *      sexyControllerData.leftStickX = 90;  // Analog stick values can range from 0 - 255
 */

#ifndef UNOJOY_H
#define UNOJOY_H
    #include <stdint.h>
    #include <util/atomic.h>
    #include <Arduino.h>

    // This struct is the core of the library.
    //  You'll create an instance of this and manipulate it,
    //  then use the setControllerData function to send that data out.
    //  Don't change this - the order of the fields is important for
    //  the communication between the Arduino and it's communications chip.
#define BUTTON_ARRAY_SIZE 8
#define ANALOG_AXIS_ARRAY_SIZE 12
    
	typedef struct megaJoyControllerData_t
	{
    uint8_t buttonArray[BUTTON_ARRAY_SIZE];
		
		uint8_t dpad0LeftOn : 1;
		uint8_t dpad0UpOn : 1;
		uint8_t dpad0RightOn : 1;
		uint8_t dpad0DownOn : 1;
    
    uint8_t dpad1LeftOn : 1;
		uint8_t dpad1UpOn : 1;
		uint8_t dpad1RightOn : 1;
		uint8_t dpad1DownOn : 1;
    
    int16_t analogAxisArray[ANALOG_AXIS_ARRAY_SIZE];
	} megaJoyControllerData_t;
    
    // Call setupMegaJoy in the setup block of your program.
    //  It sets up the hardware UnoJoy needs to work properly
    void setupMegaJoy(void);
    
    // This sets the controller to reflect the button and
    // joystick positions you input (as a megaJoyControllerData_t).
    // The controller will just send a zeroed (joysticks centered)
    // signal until you tell it otherwise with this function.
    void setControllerData(megaJoyControllerData_t);
    
    // This function gives you a quick way to get a fresh
    //  megaJoyControllerData_t with:
    //    No buttons pressed
    //    Joysticks centered
    // Very useful for starting each loop with a blank controller, for instance.
    // It returns a megaJoyControllerData_t, so you want to call it like:
    //    myControllerData = getBlankDataForController();
    megaJoyControllerData_t getBlankDataForMegaController(void);
    
    // You can also call the setup function with an integer argument
    //  declaring how often, in  milliseconds, the buffer should send its data 
    //  via the serial port.  Use it if you need to do a lot of processing and
    //  the serial stuff is messing you up, but it'll make your controller
    //  more laggy.
    // IMPORTANT - you can't make this value greater than 20 or so - the code
    //  on the communications chip times out on each serial read after 25ms.
    //  If you need more time than 20ms, you'll have to alter the code for the
    //  ATmega8u2 as well
    void setupMegaJoy(int);
    
    
//----- End of the interface code you should be using -----//
//----- Below here is the actual implementation of
    
  // This megaJoyControllerData_t is used to store
  //  the controller data that you want to send
  //  out to the controller.  You shouldn't mess
  //  with this directly - call setControllerData instead
  megaJoyControllerData_t controllerDataBuffer;

  // This updates the data that the controller is sending out.
  //  The system actually works as following:
  //  The UnoJoy firmware on the ATmega8u2 regularly polls the
  //  Arduino chip for individual bytes of a megaJoyControllerData_t.
  //  
  void setControllerData(megaJoyControllerData_t controllerData){
    // Probably unecessary, but this guarantees that the data
    //  gets copied to our buffer all at once.
    ATOMIC_BLOCK(ATOMIC_FORCEON){
      controllerDataBuffer = controllerData;
    }
  }
  
  // serialCheckInterval governs how many ms between
  //  checks to the serial port for data.
  //  It shouldn't go above 20 or so, otherwise you might
  //  get unreliable data transmission to the UnoJoy firmware,
  //  since after it sends a request, it waits 25 ms for a response.
  //  If you really need to make it bigger than that, you'll have to
  //  adjust that timeout in the UnoJoy ATmega8u2 firmware code as well.
  volatile int serialCheckInterval = 1;
  // This is an internal counter variable to count ms between
  //  serial check times
  int serialCheckCounter = 0;
  
  // This is the setup function - it sets up the serial communication
  //  and the timer interrupt for actually sending the data back and forth.
  void setupMegaJoy(void){
    // First, let's zero out our controller data buffer (center the sticks)
    controllerDataBuffer = getBlankDataForMegaController();
  
    // Start the serial port at the specific, low-error rate UnoJoy uses.
    //  If you want to change the rate, you'll have to change it in the
    //  firmware for the ATmega8u2 as well.  250,000 is actually the best rate,
    //  but it's not supported on Macs, breaking the processing debugger.
    Serial.begin(38400);
    
    // Now set up the Timer 0 compare register A
    //  so that Timer0 (used for millis() and such)
    //  also fires an interrupt when it's equal to
    //  128, not just on overflow.
    // This will fire our timer interrupt almost
    //  every 1 ms (1024 us to be exact).
    OCR0A = 128;
    TIMSK0 |= (1 << OCIE0A);
  }
  
  // If you really need to change the serial polling
  //  interval, use this function to initialize UnoJoy.
  //  interval is the polling frequency, in ms.
  void setupMegaJoy(int interval){
    serialCheckInterval = interval;
    setupMegaJoy();
  }
  
  // This interrupt gets called approximately once per ms.
  //  It counts how many ms between serial port polls,
  //  and if it's been long enough, polls the serial
  //  port to see if the UnoJoy firmware requested data.
  //  If it did, it transmits the appropriate data back.
  ISR(TIMER0_COMPA_vect){
    serialCheckCounter++;
    if (serialCheckCounter >= serialCheckInterval){
      serialCheckCounter = 0;
      // If there is incoming data stored in the Arduino serial buffer
      while (Serial.available() > 0) {
        //pinMode(13, OUTPUT);
        //digitalWrite(13, HIGH);
        // Get incoming byte from the ATmega8u2
        byte inByte = Serial.read();
        // That number tells us which byte of the megaJoyControllerData_t struct
        //  to send out.
        Serial.write(((uint8_t*)&controllerDataBuffer)[inByte]);
        //digitalWrite(13, LOW);
      }
    }
  }
  
  // Returns a zeroed out (joysticks centered) 
  //  megaJoyControllerData_t variable
  megaJoyControllerData_t getBlankDataForMegaController(void){
    // Create a megaJoyControllerData_t
    megaJoyControllerData_t controllerData;
    // Make the buttons zero
    for (int i = 0; i < 8; i++){
      controllerData.buttonArray[i] = 0;
    }
    controllerData.dpad0LeftOn = 0;
    controllerData.dpad0UpOn = 0;
    controllerData.dpad0RightOn = 0;
    controllerData.dpad0DownOn = 0;  
    
    controllerData.dpad1LeftOn = 0;
    controllerData.dpad1UpOn = 0;
    controllerData.dpad1RightOn = 0;
    controllerData.dpad1DownOn = 0;  
    
    //Set the sticks to 512 - centered
    for (int i = 0; i < ANALOG_AXIS_ARRAY_SIZE; i++){
      controllerData.analogAxisArray[i] = 512;
    }    
    // And return the data! 
    return controllerData;
  }

#endif

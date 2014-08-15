/*
 * typedefs.h
 *
 * Created: 12/13/2011 2:26:51 PM
 *  Author: Alan Chatham
 *
 *	This file contains all the typedef'ed structs we're using to store button data
 */ 


#ifndef PHYSICAL_BUTTON_LIST_T
#define PHYSICAL_BUTTON_LIST_T

// How many total inputs do we have?
#define NUMBER_OF_INPUTS 36
#define NUMBER_OF_DIGITAL_BUTTONS 27

// This one is an abstract representation of what physical buttons we have
//  on the controller itself
typedef struct physicalButtonList_t{
	uint8_t r3On;
	uint8_t startOn;
	uint8_t r2On;
	uint8_t r1On;
	uint8_t triangleOn;
	uint8_t circleOn;
	uint8_t squareOn;
	uint8_t crossOn;
	uint8_t l1On;
	uint8_t l2On;
	uint8_t selectOn;
	uint8_t dpadDownOn;
	uint8_t dpadRightOn;
	uint8_t dpadLeftOn;
	uint8_t dpadUpOn;
	uint8_t menuOn;	
	
	uint8_t leftStickX;
	uint8_t leftStickY;
	uint8_t rightStickX;
	uint8_t rightStickY;
	
	uint8_t joy3X;
	uint8_t joy3Y;
	uint8_t joy4X;
	uint8_t joy4Y;
	
	uint8_t l3On;
	uint8_t joy3ButtonOn;
	uint8_t joy4ButtonOn;
	uint8_t handsFree1;
	uint8_t handsFree2;
	uint8_t handsFree3;
	uint8_t handsFree4;
	uint8_t buttonBelowLStick;
	uint8_t buttonBelowRStick;
	uint8_t sipSwitch;
	uint8_t puffSwitch;
	uint8_t programButtonOn;
	
} physicalButtonList_t;
#endif // PHYSICAL_BUTTON_LIST_T

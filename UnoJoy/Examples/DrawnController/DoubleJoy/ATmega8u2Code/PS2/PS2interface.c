/*********************************
Interface for PlayStation 2
By:     Gene Chatham and Alan Chatham
Copyright 2009-2012
Description:
This AVR is designed to act like
a PlayStation controller.  It has 
input pins that correspond to the
buttons found on PlayStation
controllers.  It communicates with
a PlayStation console via SPI.

This file is part of the AVR PS2 library.
This is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this.  If not, see <http://www.gnu.org/licenses/>.
**********************************/




/********HEADERS********/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "PS2interface.h"

/***GLOBAL CONSTANTS & VARIABLES***/

unsigned char dataIn;			// data from PSX
int8_t frameCounter;			// keeps track of bytes tranceived per packet
unsigned char packetCode;       // keeps track of which type of packet we're sending
unsigned char packetLength;		// keeps track of number of bytes to be exchanged in each packet
unsigned char controllerDataPacketLength; // keeps track of the bytes to send in a controller data packet
unsigned char controllerMode;	// keeps track of configured mode of controller
char x46Switch;		   			// Used for the 0x46 code thing - there's a 2 packet constant 
								//  response that gets sent in response to an incoming
								//  0x46 packet code, so this tracks which response to send
char x4CSwitch;					// Similar to above
int8_t config;					// 0 = not in configuration mode,   1 = configuration mode
char packetData[21];			// Stores the packet's data - not all will be used 
char vibrationData[21];         // This isn't used right now, since we don't support vibration motor
                                //  feedback yet

// I don't think any of this buffering stuff is used right now....
// Set of buffers for reading in button data
//  since this code is interrupt driven
char packetBuffer0[21] = {0xFF, 0x79, 0x5A, 0x11, 0xA5, 0x7F, 0x7F, 0x7F, 0x7F, 
															 0,0,0,0,0,0,0,0,0,0,0,0};		
char packetBuffer1[21] = {0xFF, 0x79, 0x5A, 0x11, 0xA5, 0x7F, 0x7F, 0x7F, 0x7F, 
															 0,0,0,0,0,0,0,0,0,0,0,0};		
char * currentInputBuffer;		// Pointer to hold the location of the current buffer
								//  that we're writing to.  We will read the opposite buffer

// Constants that are used to identify certain packet types
#define DIGITAL_MODE 				0x41
#define DIGITAL_PACKET_LENGTH 		5
#define BASIC_ANALOG_MODE  			0x73
#define BASIC_ANALOG_PACKET_LENGTH	9
#define FULL_ANALOG_MODE			0x79
#define FULL_ANALOG_PACKET_LENGTH 	21
#define CONFIG_MODE					0xF3
#define CONFIG_PACKET_LENGTH		9

/*******FUNCTIONS*******/

void setPacketData(char source[])
{
	packetData[0] = 0xFF;
	packetData[1] = source[1];
	packetData[2] = 0x5A;
	packetData[3] = source[3];
	packetData[4] = source[4];
	packetData[5] = source[5];
	packetData[6] = source[6];
	packetData[7] = source[7];
	packetData[8] = source[8];
}

void setVibrationData(char source[])
{
	vibrationData[0] = 0xFF;
	vibrationData[1] = source[1];
	vibrationData[2] = 0x5A;
	vibrationData[3] = source[3];
	vibrationData[4] = source[4];
	vibrationData[5] = source[5];
	vibrationData[6] = source[6];
	vibrationData[7] = source[7];
	vibrationData[8] = source[8];
}

// Testing packet
char packetDataTest[21] = {0xFF, 0x79, 0x5A, 0x11, 0xA5, 0x7F, 0x7F, 0x7F, 0x7F, 
															 0,0,0,0,0,0,0,0,0,0,0,0};
// Initialize all ports, pins, and variables.
void startPS2Communication(void)
{
	//PORTB setup
	SPI_ACK_DDR |= (1<<ACK);	//output
	SPI_ACK_PORT |= (1<<ACK);		//set HIGH
	SPI_DDR &= ~(1<<ATT);	//input
	SPI_PORT |= (1<<ATT);		//pull-up enabled	
	SPI_DDR &= ~(1<<CMD);  //input
	SPI_PORT |= (1<<CMD);		//pull-up enabled
	SPI_DDR |= (1<<DATA);	//output
	SPI_PORT |= (1<<DATA);		//set HIGH
	SPI_DDR &= ~(1<<CLK);	//input
	SPI_PORT |= (1<<CLK);		//pull-up enabled

	//SPI initialization

	//SPCR = SPI Control Register
	SPCR |= (1<<CPOL);			// SCK HIGH when idle
	SPCR |= (1<<CPHA);			// setup data on falling edge of CLK, read on rising edge
	SPCR &= ~(1<<MSTR);			// select SLAVE mode
	SPCR |= (1<<DORD);			// data order to LSB first
	SPCR |= (1<<SPE);			// SPI enable

	SPDR = 0xFF;				// initialize SPI Data Register to all bits HIGH

	dataIn= 0x00;
	frameCounter = 0;
	x46Switch = 0;
	x4CSwitch = 0;
	packetCode = 0x00;         // keeps track of which type of packet we're sending
	controllerDataPacketLength = FULL_ANALOG_PACKET_LENGTH;//18;
	packetLength = controllerDataPacketLength;
	controllerMode = FULL_ANALOG_MODE;
	config = 0;
	//Set up a default digital controller packet 
	char packetDataInit[21] = {0xFF, controllerMode, 0x5A, PIND, PINC, 0x7F, 0x7F, 0x7F, 0x7F, 
															 0,0,0,0,0,0,0,0,0,0,0,0};
	setPacketData(packetDataInit);
	char vibrationDataInit[21] = {0xFF, 0xF3, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
						 	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	setVibrationData(vibrationDataInit);

	// Finally, enable the interrupts to start PS2 communication
	PCMSK0 = 1; // Enable pin change 0 interrupt
	PCICR |= 1 << PCIE0;
	sei();	// Enable global interrupts - we're using the Pin Change 0 interrupt
}



char SPI_SlaveReceive(void)
{
	while(!(SPSR & (1<<SPIF)))
	{
		if (SPI_PIN & (1 << ATT)){  // If ATT (SS) is held high, just return
			SPDR = 0x00;
			frameCounter = 0;
			return 0;
		}
	}		// wait for reception complete
	return SPDR;					// return value in SPI data register
}

// This waits for the SPI unit to receive data, and returns 1 when the SPI
//  unit receives data.
// If, while waiting for data, SS goes high, then this function returns 0;
// After using this function, read SPDR for the actual data
char spiSlaveHasData(void)
{
	while(!(SPSR & (1<<SPIF)))
	{
		if (PINB & (1 << ATT)){  // If ATT (SS) is held high, just return
			SPDR = 0x00;
			frameCounter = 0;
			return 0;
		}
	}		// wait for reception complete
	return 1;					// return value in SPI data register
}

// This sends one byte, followed by an ACK
void sendOneByteSPI(void){
	SPI_ACK_PORT &= ~(1<<ACK);					// ACK LOW
	frameCounter++;						
	SPDR = packetData[frameCounter];	// set SPDR to return the next byte in the frame
										//  the next time the master communicates
	SPI_ACK_PORT |= (1<<ACK);					// ACK back to HIGH
}
// Sends one byte, but no acknowledge, signifiying the end of a packet
void sendOneByteSPINoACK(void){
	frameCounter++;						
	SPDR = packetData[frameCounter];	// set SPDR to return the next byte in the frame
										//  the next time the master communicates
}


// This function reads in a byte of data and sets up the appropriate byte to be sent
//  in return.  It's kinda hard to follow, but remember - it only actually sends one
//  byte each time it's called. 
void communicate(void)
{

	dataIn = SPI_SlaveReceive();	// read incoming byte from PS

	// Check to see if we're even being talked to
	if (SPI_PIN & (1 << ATT))  // If ATT (SS) is held high, just return
	{
		SPDR = 0x00;
		frameCounter = 0;
		return;
	}
	
	// Now that we're being talked to...

	if(frameCounter == 0)		// start command, always 0x01
	{
		// Read the buttons every start
		// Figure out what sort of packet we'll be sending
		if (dataIn == 0x01){
			if (config == 1){
				packetData[1] = CONFIG_MODE;	// Send the config mode code if appropriate
				packetLength = CONFIG_PACKET_LENGTH;
			}
			else{
				packetData[1] = controllerMode;		// set SPDR to return controller mode configuration
				packetLength = controllerDataPacketLength;
			}
			sendOneByteSPI();
		}
		else // if we didn't get a 0x01, reset our logic
			frameCounter = 0;
	}
	//  The second command will be the main command. We'll be returning
	//   the controller type in this command, in sendOneByteSPI()
	else if(frameCounter == 1)	// find out what buttons are included in poll responses				
	{
		packetCode = dataIn;
		sendOneByteSPI();
	}
	// 0x41 finds out what buttons are included in the polling response
	else if( (frameCounter == 2) && (packetCode == 0x41)){
		if (controllerMode == DIGITAL_MODE){
			packetData[3] = 0x00;
			packetData[4] = 0x00;
			packetData[5] = 0x00;
			packetData[6] = 0x00;
			packetData[7] = 0x00;
			packetData[8] = 0x00;
		}
		else{
			packetData[3] = 0xFF;
			packetData[4] = 0xFF;
			packetData[5] = 0x03;
			packetData[6] = 0x00;
			packetData[7] = 0x00;
			packetData[8] = 0x5A;
		}
		sendOneByteSPI();
	}
	// 0x43 This handles entering and exiting config mode
	else if( (frameCounter == 3) && (packetCode == 0x43) ){
		if (dataIn == 0x01){
			config = 1;
		}
		else{
			config = 0;
		}
		sendOneByteSPI();
	}
	// 0x44  This changes modes between digital and analog mode.
	//  Depending on which mode, we change the length of the controller data packets
	//  and set things up to tell the system what sort of data to expect
	else if( (frameCounter == 3) && (packetCode == 0x44)){// && (config == 1) ){
		sendOneByteSPI();
		if (dataIn == 0x00 && (1 == 2)){
			controllerMode = DIGITAL_MODE;
			controllerDataPacketLength = DIGITAL_PACKET_LENGTH;
		}
		else{
			controllerMode = FULL_ANALOG_MODE;
			controllerDataPacketLength = FULL_ANALOG_PACKET_LENGTH;
		}
	}
	// 0x45 is asking the controller for information about itself
	else if( (frameCounter == 2) && (packetCode == 0x45)){// && (config == 1) ){
		packetData[3] = 0x03;
		packetData[4] = 0x02;
		packetData[5] = 0x01;
		packetData[6] = 0x02;
		packetData[7] = 0x01;
		packetData[8] = 0x00;
		sendOneByteSPI();
	}
	// 0x46  Random checksum return thingy...
	else if( (frameCounter == 2) && (packetCode == 0x46)){// && (config == 1) ){
		packetData[3] = 0x00;
		sendOneByteSPI();
	}
	// Continued 0x46 stuff
	else if( (frameCounter == 3) && (packetCode == 0x46)){// && (config == 1) ){
		if (dataIn == 0x00){
			packetData[4] = 0x00;
			packetData[5] = 0x00;
			packetData[6] = 0x02;
			packetData[7] = 0x00;
			packetData[8] = 0x0A;
		}
		if (dataIn == 0x01){
			packetData[4] = 0x00;
			packetData[5] = 0x00;
			packetData[6] = 0x00;
			packetData[7] = 0x00;
			packetData[8] = 0x14;
		}
		sendOneByteSPI();
	}
	// Another constant packet
	else if( (frameCounter == 2) && (packetCode == 0x47)){// && (config == 1) ){
		packetData[3] = 0x00;
		packetData[4] = 0x00;
		packetData[5] = 0x02;
		packetData[6] = 0x00;
		packetData[7] = 0x00;
		packetData[8] = 0x00;
		sendOneByteSPI();
	}
	// 0x4C  Random checksum return thingy...
	else if( (frameCounter == 2) && (packetCode == 0x4C)){// && (config == 1) ){
		packetData[3] = 0x00;
		sendOneByteSPI();
	}
	// Continued 0x4C stuff
	else if( (frameCounter == 3) && (packetCode == 0x4C)){// && (config == 1) ){
		if (dataIn == 0x00){
			packetData[4] = 0x00;
			packetData[5] = 0x00;
			packetData[6] = 0x04;
			packetData[7] = 0x00;
			packetData[8] = 0x00;
		}
		if (dataIn == 0x01){
			packetData[4] = 0x00;
			packetData[5] = 0x00;
			packetData[6] = 0x06;
			packetData[7] = 0x00;
			packetData[8] = 0x00;
		}
		sendOneByteSPI();
	}
	// 0x4D  Maps bytes in the 0x42 command to activate vibration motors
	//   TODO: MAKE THIS STORE THE VALUE WRITTEN TO IT TO REPEAT LATER
	else if( (frameCounter == 2) && (packetCode == 0x4D)){// && (config == 1) ){
		packetData[3] = 0x00;
		packetData[4] = 0x01;
		packetData[5] = 0xFF;
		packetData[6] = 0xFF;
		packetData[7] = 0xFF;
		packetData[8] = 0xFF;
		sendOneByteSPI();
	}
	// 0x4F  Adds and removes
	//   TODO: MAKE THIS STORE THE VALUE WRITTEN TO IT TO REPEAT LATER
	//    in conjunction with 0x41
	else if( (frameCounter == 2) && (packetCode == 0x4F)){// && (config == 1) ){
		packetData[3] = 0x00;
		packetData[4] = 0x00;
		packetData[5] = 0x00;
		packetData[6] = 0x00;
		packetData[7] = 0x00;
		packetData[8] = 0x5A;
		sendOneByteSPI();
	}
	// Finally, just send out all the rest of the data if told to do so
	else if(frameCounter > 1)
	{
		// We send out an ACK signal for each byte but the last one
		if(frameCounter < packetLength - 1)
		{
				frameCounter++;
				unsigned char byteToSend = packetData[frameCounter]; // Read this here, otherwise it takes too long to send
				SPI_ACK_PORT &= ~(1<<ACK);					// ACK LOW
	
				SPDR = byteToSend;//packetDataTest[frameCounter];	// set SPDR to return the next byte in the frame
													//  the next time the master communicates
				SPI_ACK_PORT |= (1<<ACK);					// ACK back to HIGH
		}
		else if (frameCounter == packetLength - 1)// this is for the last byte
		{
				frameCounter++;	
				unsigned char byteToSend = packetData[frameCounter];
				SPDR = byteToSend;	// set SPDR to return the next byte in the frame
													//  the next time the master communicates
				SPI_ACK_PORT |= (1<<ACK);					// ACK back to HIGH
		}
		// Finally, a little check to make sure we don't go out of bounds
		else if(frameCounter >= packetLength){
			frameCounter = 0;
		}
	}
}

// This interrupt gets called every time Attention changes, and it sends a packet
ISR(PCINT0_vect){
	frameCounter = 0;
	SPDR = 0xFF;
	// While ATT is held low, we complete a full packet exhange.
	while (!(SPI_PIN & (1 << ATT))){
		// communicate() only handles one byte at a time, so we loop this
		communicate();
	}
}

// This is our external hook - someone else writes data to us,
//  and we just throw it into the buffer to send since double-buffering
//  is too slow (running at 2MHz).
void sendPS2Data(dataForController_t controllerData){
	// set the buttons - for the packet, 0 means pressed, 1 means off
	packetData[3] = ( (!controllerData.selectOn) << 0 |
						 (!controllerData.l3On) << 1 |
						 (!controllerData.r3On) << 2 |
						 (!controllerData.startOn) << 3 |
						 (!controllerData.dpadUpOn) << 4 |
						 (!controllerData.dpadRightOn) << 5 |
						 (!controllerData.dpadDownOn) << 6 |
						 (!controllerData.dpadLeftOn) << 7    );

	packetData[4] = ( (!controllerData.l2On) << 0 |
						 (!controllerData.r2On) << 1 |
						 (!controllerData.l1On) << 2 |
						 (!controllerData.r1On) << 3 |
						 (!controllerData.triangleOn) << 4 |
						 (!controllerData.circleOn) << 5 |
						 (!controllerData.crossOn) << 6 |
						 (!controllerData.squareOn) << 7    );

	packetData[5] = controllerData.rightStickX;
	packetData[6] = controllerData.rightStickY;
	packetData[7] = controllerData.leftStickX;
	packetData[8] = controllerData.leftStickY;
}

// This code is only compiled if the LIBRARY_TEST_MAIN flag is
//  set to 1 in PS2interface.h - it provides a default test program
//  for testing to make sure the library is working
#ifdef LIBRARY_TEST_MAIN

void sendPS2DataTest(void){
	dataForController_t data;
	data.dpadLeftOn = 0;
	data.dpadRightOn = 0;
	data.dpadUpOn = 0;
	data.dpadDownOn = 0;
	data.l1On = 0;
	data.l2On = 0;
	data.l3On = 0;
	data.r1On = 0;
	data.r2On = 0;
	data.r3On = 0;
	data.squareOn = !(TEST_PIN & (1 << 7));
	data.triangleOn = !(TEST_PIN & (1 << 4));
	data.crossOn = !(TEST_PIN & (1 << 6));
	data.circleOn = !(TEST_PIN & (1 << 5));
	data.selectOn = 0;
	data.startOn = 0;
	data.leftStickX = 0x7F;
	data.leftStickY = 0x7F;
	data.rightStickX = 0x7F;
	data.rightStickY = 0x7F;
	sendPS2Data(data);
}


void setupButtonPins(void)
{
	// Test port setup - all inputs, all pull-ups enabled
	TEST_DDR = 0;
	TEST_PORT = 0xFF;
}

int main(void)
{
	setupButtonPins();
	startPS2Communication();			// run function to set up I/O pins and global variables

	while(1)				// run indefinitely
	{
		sendPS2DataTest();
	}
	
	return 0;
}
#endif

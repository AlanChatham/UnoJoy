/*********************************
Interface for PlayStation 2
By:     Gene Chatham and Alan Chatham
Copyright 2009-2012

This AVR is designed to act like
a PlayStation controller.  It has 
input pins that correspond to the
buttons found on PlayStation
controllers.  It communicates with
a PlayStation console via SPI.

It uses all the SPI pins, plus one more 
 pin for the PS2's non-standard ACK signal
 
THIS CODE USES INTERRUPTS - specifically, it uses
 a pin change interrupt on the non-SPI 'Attention' line.
 Be aware of this.

This code should work on a variety of AVR devices.
Currently tested working on the AT90USB64/128 series,
running at 2MHz with an external 16MHz crystal
Adjustments are likely needed to the SPI timing prescaler
for proper operation,  and running at speeds below 2MHz
may not be fast enough for the logic to operate correctly.

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

#ifndef PS2INTERFACE
#define PS2INTERFACE

// This holds the external data structure that will be used
//  to send controller data to the PS2
#include "dataForController_t.h"

// Adjust these pins to match your AVR's particular 
//  pins for dedicated SPI
#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_PIN  PINB
#define ATT		0		//SS
#define CMD		2		//MOSI
#define DATA	3		//MISO
#define CLK		1		//CLK

// The PS2 also uses a non-standard 'acknowledge' signal -
//  choose any pin you like for it 
#define SPI_ACK_PORT PORTB
#define SPI_ACK_DDR  DDRB
#define SPI_ACK_PIN  PINB
#define ACK		4		//Choose any pin you like


// This function sets up the PS2 communication.
//  Run it before your main loop, or at least before
//  you try and send data.
void startPS2Communication(void);

// This function takes in a dataForController_t struct
//  that tells the controller what signals to send.
void sendPS2Data(dataForController_t);


// Uncomment this, and a main() function will be included in
//  this code, so you can compile the library as a stand-alone test
//  application.
//#define LIBRARY_TEST_MAIN

// These are used by the test main() function 
//  - the triangle, circle, cross, and square buttons
//  will be controlled by grounding pins 4 - 7, respectively,
//  on the selected port.
#define TEST_PIN PINC
#define TEST_PORT PORTC
#define TEST_DDR DDRC

#endif

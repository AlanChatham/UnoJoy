/* UnoJoy.c
 * Copyright (C) 2012 Alan Chatham <alan@openchord.org>
 * Made in conjunction with the RMIT Exertion Games Lab
 *
 * Based on works by:
 *   Josh Kropf <http://git.slashdev.ca/ps3-teensy-hid>
 *   grunskis <http://github.com/grunskis/gamepad>
 *   Toodles <http://forums.shoryuken.com/showthread.php?t=131230>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Keyboard example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "dataForMegaController_t.h"
#include "usb_gamepad.h"

#define RXLED 4
#define TXLED 5

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

// This sets up an empty controller data packet and sends it out
//  to all the controllers attached.
void setControllersToZero(void){
	dataForMegaController_t emptyData;
	for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++)
		emptyData.buttonArray[i] = 0;
	emptyData.leftStickX = 512;
	emptyData.leftStickY = 512;
//	emptyData.rightStickX = 512;
//	emptyData.rightStickY = 512;
//	emptyData.stick3X = 512;
//	emptyData.stick3Y = 512;
	//sendControllerDataViaUSB(emptyData, 0);
	//sendControllerDataViaUSB(emptyData, 1);
}

// Initializes the USART to receive and transmit,
//  takes in a value you can find in the datasheet
//  based on desired communication and clock speeds
void USART_Init(uint16_t baudSetting){
	// Set baud rate
	UBRR1 = baudSetting;
	// Enable receiver and transmitter
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);
	// Set frame format: 8data, 1stop bit
	UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);	
}

// This reads the USART serial port, returning any data that's in the
//  buffer, or a guaranteed zero if it took longer than timeout ms
//  Input: uint_16 timeout - milliseconds to wait for data before timing out
unsigned char serialRead( uint16_t timeout ){
	// Wait for data to be received 
	while ( !(UCSR1A & (1<<RXC1)) ){
		_delay_ms(1);
		timeout--;
		if (timeout == 0){
			return 0b0;
		}			
	}	
	// Get and return received data from buffer 
	return UDR1;
}

// This sends out a byte of data via the USART.
void serialWrite( unsigned char data )
{
	// Wait for empty transmit buffer
	while ( !( UCSR1A & (1<<UDRE1)) ){
	}	
	// Put data into buffer, sends the data
	UDR1 = data;
}

void flushSerialRead()
{
	unsigned char dummy;
	while ( UCSR1A & (1<<RXC1) )
		dummy = UDR1;
}

// This turns on one of the LEDs hooked up to the chip
void LEDon(char ledNumber){
	DDRD |= 1 << ledNumber;
	PORTD &= ~(1 << ledNumber);
}

// And this turns it off
void LEDoff(char ledNumber){
	DDRD &= ~(1 << ledNumber);
	PORTD |= 1 << ledNumber;
}

// Get a 16 bit value off the serial port by doing two successive reads
//  Assumes that data is being transmitted high byte first
int16_t get16bitValue(int serialIndex){
	int16_t returnValue = 0;
	serialWrite(serialIndex);
	serialIndex++;
	returnValue = serialRead(25);
	
	serialWrite(serialIndex);
	serialIndex++;
	returnValue += serialRead(25) << 8;	
	return returnValue;
}

int main(void) {
	// Make sure our watchdog timer is disabled!
	wdt_reset(); 
	MCUSR &= ~(1 << WDRF); 
	wdt_disable();

	// Start up the USART for serial communications
	// 25 corresponds to 38400 baud - see datasheet for more values
	USART_Init(25);// 103 corresponds to 9600, 8 corresponds to 115200 baud, 3 for 250000
	
	// set the prescale for the USB for our 16 MHz clock
	CPU_PRESCALE(0);

	// Initialize our USB connection
	usb_init();
	while (!usb_configured()){
		LEDon(TXLED);
		_delay_ms(50);
		LEDoff(TXLED);
		_delay_ms(50);
	} // wait

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	// This wait also gives the Arduino bootloader time to timeout,
	//  so the serial data you'll be properly aligned.
	_delay_ms(500);
	dataForMegaController_t controllerData1;
	dataForMegaController_t controllerData2;

	while (1) {
		// Delay so we're not going too fast
		_delay_ms(10);
		
        // We get our data from the ATmega328p by writing which byte we
        //  want from the dataForController_t, and then wait for the
        //  ATmega328p to send that back to us.
        // The serialRead(number) function reads the serial port, and the
        //  number is a timeout (in ms) so if there's a transmission error,
        //  we don't stall forever.
		LEDon(TXLED);
		flushSerialRead();
		
		int serialIndex = 0;
		// The buttons are held in an array, so we need to break it between the two controllers
		for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++){
			serialWrite(serialIndex);
			serialIndex++;
			controllerData1.buttonArray[i] = serialRead(25);	
		}
		
		for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++){
			serialWrite(serialIndex);
			serialIndex++;
			controllerData2.buttonArray[i] = serialRead(25);
		}
		
		serialWrite(serialIndex);
		serialIndex++;
		uint8_t directionButtons = serialRead(25);
		controllerData1.dpadLeftOn = 1 & (directionButtons >> 0);
		controllerData1.dpadUpOn = 1 & (directionButtons >> 1);
		controllerData1.dpadRightOn = 1 & (directionButtons >> 2);
		controllerData1.dpadDownOn = 1 & (directionButtons >> 3);
		
		controllerData2.dpadLeftOn = 1 & (directionButtons >> 4);
		controllerData2.dpadUpOn = 1 & (directionButtons >> 5);
		controllerData2.dpadRightOn = 1 & (directionButtons >> 6);
		controllerData2.dpadDownOn = 1 & (directionButtons >> 7);
		
		// Assuming that 16 bit data gets sent high byte first
		controllerData1.leftStickX = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData1.leftStickY = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData1.rightStickX = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData1.rightStickY = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData1.stick3X = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData1.stick3Y = get16bitValue(serialIndex);
		serialIndex += 2;
		
		controllerData2.leftStickX = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData2.leftStickY = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData2.rightStickX = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData2.rightStickY = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData2.stick3X = get16bitValue(serialIndex);
		serialIndex += 2;
		controllerData2.stick3Y = get16bitValue(serialIndex);
		
		// Communication with the Arduino chip is over here
		LEDoff(TXLED);	
        // Finally, we send the data out via the USB port
		sendControllerDataViaUSB(controllerData1, 1);	
		_delay_ms(10);
		sendControllerDataViaUSB(controllerData2, 2);
	}
}
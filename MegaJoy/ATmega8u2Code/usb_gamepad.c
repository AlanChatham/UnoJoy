/* PS3 Teensy HID Gamepad
 * Copyright (C) 2010 Josh Kropf <josh@slashdev.ca>
 *
 * Based on works by:
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

/* USB Keyboard Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2009 PJRC.COM, LLC
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

#define USB_GAMEPAD_PRIVATE_INCLUDE

#include "usb_gamepad.h"

/**************************************************************************
 *
 *  Configurable Options
 *
 **************************************************************************/

// You can change these to give your code its own name.
#define STR_MANUFACTURER	L"OpenChord X RMIT Exertion Games Lab"
#define STR_PRODUCT			L"UnoJoy Joystick"
//#define STR_MANUFACTURER	L"SEGA"
//#define STR_PRODUCT		L"VIRTUA STICK High Grade"


// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define VENDOR_ID		0x10C4
#define PRODUCT_ID		0x82C1


// USB devices are supposed to implement a halt feature, which is
// rarely (if ever) used.  If you comment this line out, the halt
// code will be removed, saving 102 bytes of space (gcc 4.3.0).
// This is not strictly USB compliant, but works with all major
// operating systems.
//#define SUPPORT_ENDPOINT_HALT

#define GAMEPAD_0_REPORT_ID 1
#define GAMEPAD_1_REPORT_ID 2

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define ENDPOINT0_SIZE	64

#define GAMEPAD_INTERFACE	0
#define GAMEPAD_ENDPOINT	1
#define GAMEPAD_SIZE		64
#define GAMEPAD_BUFFER	EP_SINGLE_BUFFER //EP_DOUBLE_BUFFER

static const uint8_t PROGMEM endpoint_config_table[] = {
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(GAMEPAD_SIZE) | GAMEPAD_BUFFER,
	0,
	0,
	0
};


/**************************************************************************
 *
 *  Descriptor Data
 *
 **************************************************************************/

// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo).  The most commonly
// changed items are editable at the top of this file.  Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of any USB class specifications!


static const uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x10, 0x01,				// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0x00, 0x01,				// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	0,					// iSerialNumber
	1					// bNumConfigurations
};

static const uint8_t PROGMEM gamepad_hid_report_desc[] = {
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,        // USAGE (Gamepad)
	0xa1, 0x01,        // COLLECTION (Application)
	0x85, GAMEPAD_0_REPORT_ID,   //  REPORT ID (Gamepad 0)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
	0x45, 0x01,        //   PHYSICAL_MAXIMUM (1)
	0x75, 0x01,        //   REPORT_SIZE (1)
	0x95, 8 * BUTTON_ARRAY_LENGTH,        //   REPORT_COUNT (32)
	0x05, 0x09,        //   USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 8 * BUTTON_ARRAY_LENGTH,        //   USAGE_MAXIMUM (Button 32)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
	0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
	0x46, 0x3b, 0x01,  //   PHYSICAL_MAXIMUM (315)
	0x75, 0x04,        //   REPORT_SIZE (4)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x65, 0x14,        //   UNIT (Eng Rot:Angular Pos)
	0x09, 0x39,        //   USAGE (Hat switch)
	0x81, 0x42,        //   INPUT (Data,Var,Abs,Null)
	0x65, 0x00,        //   UNIT (None)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
	0x16, 0x00, 0x00,  //   LOGICAL_MINIMUM 0
	0x26, 0xff, 0x03,  //   LOGICAL_MAXIMUM (1024)
	0x36, 0x00, 0x00,  //   PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x03,  //   PHYSICAL_MAXIMUM (1024)
	0x09, 0x30,        //   USAGE (X)
	0x09, 0x31,        //   USAGE (Y)
	0x09, 0x32,        //   USAGE (Z)
	0x09, 0x33,		   //   USAGE (Rx)
	0x09, 0x34,		   //   USAGE (Ry)
	0x09, 0x35,        //   USAGE (Rz)
	0x75, 0x10,        //   REPORT_SIZE (16)
	0x95, 0x06,        //   REPORT_COUNT (6)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x06, 0x00, 0xff,  //   USAGE_PAGE (Vendor Specific)
	0x09, 0x20,        //   Unknown
	0x09, 0x21,        //   Unknown
	0x09, 0x22,        //   Unknown
	0x09, 0x23,        //   Unknown
	0x09, 0x24,        //   Unknown
	0x09, 0x25,        //   Unknown
	0x09, 0x26,        //   Unknown
	0x09, 0x27,        //   Unknown
	0x09, 0x28,        //   Unknown
	0x09, 0x29,        //   Unknown
	0x09, 0x2a,        //   Unknown
	0x09, 0x2b,        //   Unknown
	0x75, 0x08,        //   REPORT_SIZE (8) NEW
	0x95, 0x0c,        //   REPORT_COUNT (12)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x0a, 0x21, 0x26,  //   Unknown
	0x95, 0x08,        //   REPORT_COUNT (8)
	0xb1, 0x02,        //   FEATURE (Data,Var,Abs)
	0xc0,              // END_COLLECTION
	
	//MegaJoy descriptor 2
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,        // USAGE (Gamepad)
	0xa1, 0x01,        // COLLECTION (Application)
	0x85, GAMEPAD_1_REPORT_ID,   //  REPORT ID (Gamepad 0)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
	0x45, 0x01,        //   PHYSICAL_MAXIMUM (1)
	0x75, 0x01,        //   REPORT_SIZE (1)
	0x95, 8 * BUTTON_ARRAY_LENGTH,        //   REPORT_COUNT (144)
	0x05, 0x09,        //   USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 8 * BUTTON_ARRAY_LENGTH,        //   USAGE_MAXIMUM (Button 144)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
	0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
	0x46, 0x3b, 0x01,  //   PHYSICAL_MAXIMUM (315)
	0x75, 0x04,        //   REPORT_SIZE (4)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x65, 0x14,        //   UNIT (Eng Rot:Angular Pos)
	0x09, 0x39,        //   USAGE (Hat switch)
	0x81, 0x42,        //   INPUT (Data,Var,Abs,Null)
	0x65, 0x00,        //   UNIT (None)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
	0x16, 0x00, 0x00,  //   LOGICAL_MINIMUM 0
	0x26, 0xff, 0x03,  //   LOGICAL_MAXIMUM (1024)
	0x36, 0x00, 0x00,  //   PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x03,  //   PHYSICAL_MAXIMUM (1024)
	0x09, 0x30,        //   USAGE (X)
	0x09, 0x31,        //   USAGE (Y)
	0x09, 0x32,        //   USAGE (Z)
	0x09, 0x33,		   //   USAGE (Rx)
	0x09, 0x34,		   //   USAGE (Ry)
	0x09, 0x35,        //   USAGE (Rz)
	0x75, 0x10,        //   REPORT_SIZE (16)
	0x95, 0x06,        //   REPORT_COUNT (6)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x06, 0x00, 0xff,  //   USAGE_PAGE (Vendor Specific)
	0x09, 0x20,        //   Unknown
	0x09, 0x21,        //   Unknown
	0x09, 0x22,        //   Unknown
	0x09, 0x23,        //   Unknown
	0x09, 0x24,        //   Unknown
	0x09, 0x25,        //   Unknown
	0x09, 0x26,        //   Unknown
	0x09, 0x27,        //   Unknown
	0x09, 0x28,        //   Unknown
	0x09, 0x29,        //   Unknown
	0x09, 0x2a,        //   Unknown
	0x09, 0x2b,        //   Unknown
	0x75, 0x08,        //   REPORT_SIZE (8) NEW
	0x95, 0x0c,        //   REPORT_COUNT (12)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x0a, 0x21, 0x26,  //   Unknown
	0x95, 0x08,        //   REPORT_COUNT (8)
	0xb1, 0x02,        //   FEATURE (Data,Var,Abs)
	0xc0,              // END_COLLECTION

};


#define CONFIG1_DESC_SIZE		(9+9+9+7)
#define GAMEPAD_HID_DESC_OFFSET	(9+9)
static const uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),			// wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	1,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0x80,					// bmAttributes
	50,					// bMaxPower
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	GAMEPAD_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x00,					// bInterfaceSubClass (0x00 = No Boot)
	0x00,					// bInterfaceProtocol (0x00 = No Protocol)
	0,					// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	0x11, 0x01,				// bcdHID
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	sizeof(gamepad_hid_report_desc),	// wDescriptorLength
	0,
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	GAMEPAD_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	GAMEPAD_SIZE, 0,			// wMaxPacketSize
	10					// bInterval
};

// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.
struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};
static const struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};
static const struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
static const struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x2100, GAMEPAD_INTERFACE, config1_descriptor+GAMEPAD_HID_DESC_OFFSET, 9},
	{0x2200, GAMEPAD_INTERFACE, gamepad_hid_report_desc, sizeof(gamepad_hid_report_desc)},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)}
};
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))


/**************************************************************************
 *
 *  Variables - these are the only non-stack RAM usage
 *
 **************************************************************************/

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration = 0;

static const gamepad_state_t PROGMEM gamepad_0_idle_state = {
	.id = GAMEPAD_0_REPORT_ID,
	.buttonArray = {0,0,0,0},
	.direction = 0x08,
	.l_x_axis = 0x80, .l_y_axis = 0x80, .r_x_axis = 0x80, .r_y_axis = 0x80,
	.up_axis = 0x00, .right_axis = 0x00, .down_axis = 0x00, .left_axis = 0x00,
	.circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00, .triangle_axis = 0x00,
	.l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00,
	.x_3_axis = 0x00, .y_3_axis = 0x00
};

static const gamepad_state_t PROGMEM gamepad_1_idle_state = {
	.id = GAMEPAD_1_REPORT_ID,
	.buttonArray = {0,0,0,0},
	.direction = 0x08,
	.l_x_axis = 0x80, .l_y_axis = 0x80, .r_x_axis = 0x80, .r_y_axis = 0x80,
	.up_axis = 0x00, .right_axis = 0x00, .down_axis = 0x00, .left_axis = 0x00,
	.circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00, .triangle_axis = 0x00,
	.l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00,
	.x_3_axis = 0x00, .y_3_axis = 0x00
};

/*
 * Series of bytes that appear in control packets right after the HID
 * descriptor is sent to the host. They where discovered by tracing output
 * from a Madcatz SF4 Joystick. Sending these bytes makes the PS button work.
 */
static const uint8_t PROGMEM magic_init_bytes[] = {
	0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00
};

static uint8_t gamepad_idle_config = 0;

// protocol setting from the host.  We use exactly the same report
// either way, so this variable only stores the setting since we
// are required to be able to report which setting is in use.
static uint8_t gamepad_protocol = 1;

/**************************************************************************
 *
 *  Public Functions - these are the API intended for the user
 *
 **************************************************************************/

// initialize USB
//  Returns 0 if initialized,
//          1 if it timed out waiting for connection
uint8_t usb_init(void) {
	HW_CONFIG();
	USB_FREEZE();				// enable USB
	PLL_CONFIG();				// config PLL
	// wait a certain amount of time for PLL lock
	unsigned long timeoutCounter = 0;
	while (!(PLLCSR & (1<<PLOCK))){
		_delay_ms(1);
		timeoutCounter++;
		if (timeoutCounter >= USB_TIMEOUT)
			return 1;
	}		
	USB_CONFIG();				// start USB clock
	UDCON = 0;				// enable attach resistor
	usb_configuration = 0;
	UDIEN = (1<<EORSTE)|(1<<SOFE);
	sei();
	return 0;
}

// return 0 if the USB is not configured, or the configuration
// number selected by the HOST
uint8_t usb_configured(void) {
	return usb_configuration;
}

gamepad_state_t usbControllerState;

inline void usb_gamepad_0_reset_state(void) {
	memcpy_P(&usbControllerState, &gamepad_0_idle_state, sizeof(gamepad_state_t));
}

inline void usb_gamepad_1_reset_state(void) {
	memcpy_P(&usbControllerState, &gamepad_1_idle_state, sizeof(gamepad_state_t));
}

// sendPS3Data takes in a PhysicalButtonList data struct,
//  maps the data to the format for our gamepad packet,
//  sends that data out via USB,  and outputs
//  a return code (zero if no problems, one if problems
int8_t sendControllerDataViaUSB(dataForMegaController_t btnList, uint8_t playerID){
	usbControllerState.id = playerID;
	
	memcpy(usbControllerState.buttonArray, btnList.buttonArray, BUTTON_ARRAY_LENGTH);
/*	usbControllerState.triangle_btn = btnList.triangleOn;//Button 4
	usbControllerState.square_btn = btnList.squareOn;	
	usbControllerState.cross_btn = btnList.crossOn;
	usbControllerState.circle_btn = btnList.circleOn;

	usbControllerState.l1_btn = btnList.l1On;
	usbControllerState.r1_btn = btnList.r1On;
	usbControllerState.l2_btn = btnList.l2On;
	usbControllerState.r2_btn = btnList.r2On;
		
	
	if (usbControllerState.triangle_btn == 1)
		usbControllerState.triangle_axis = 0xFF;
	else
		usbControllerState.triangle_axis = 0;
		
	if (usbControllerState.square_btn == 1)
		usbControllerState.square_axis = 0xFF;
	else
		usbControllerState.square_axis = 0;

	if (usbControllerState.cross_btn == 1)
		usbControllerState.cross_axis = 0xFF;
	else
		usbControllerState.cross_axis = 0;

	if (usbControllerState.circle_btn == 1)
		usbControllerState.circle_axis = 0xFF;
	else
		usbControllerState.circle_axis = 0;

	if (usbControllerState.l1_btn == 1)
		usbControllerState.l1_axis = 0xFF;
	else
		usbControllerState.l1_axis = 0;
		
	if (usbControllerState.l2_btn == 1)
		usbControllerState.l2_axis = 0xFF;
	else
		usbControllerState.l2_axis = 0;
		
	if (usbControllerState.r1_btn == 1)
		usbControllerState.r1_axis = 0xFF;
	else
		usbControllerState.r1_axis = 0;
			
	if (usbControllerState.r2_btn == 1)
		usbControllerState.r2_axis = 0xFF;
	else
		usbControllerState.r2_axis = 0;
		
	usbControllerState.select_btn = btnList.selectOn;
	usbControllerState.start_btn = btnList.startOn;
	usbControllerState.l3_btn = btnList.l3On;
	usbControllerState.r3_btn = btnList.r3On;
	
	//gamepad_state.r2_btn_alt = btnList.r2On;
	//gamepad_state.l2_btn_alt = btnList.l2On;
	usbControllerState.ps_btn = btnList.homeOn;
*/ // End of the buttons

	// digital direction, use the dir_* constants(enum)
	// 8 = center, 0 = up, 1 = up/right, 2 = right, 3 = right/down
	// 4 = down, 5 = down/left, 6 = left, 7 = left/up

	usbControllerState.direction = 8;
	if (btnList.dpadUpOn == 1){
		if (btnList.dpadLeftOn == 1){
			usbControllerState.direction = 7;
		} 
		else if (btnList.dpadRightOn == 1){
			usbControllerState.direction = 1;
		}
		else
			usbControllerState.direction = 0;
		
	}
	else if (btnList.dpadDownOn == 1){
				if (btnList.dpadLeftOn == 1){
			usbControllerState.direction = 5;
		} 
		else if (btnList.dpadRightOn == 1){
			usbControllerState.direction = 3;
		}
		else
			usbControllerState.direction = 4;		
	}
	else if (btnList.dpadLeftOn == 1){
		usbControllerState.direction = 6;
	}
	else if (btnList.dpadRightOn == 1){
		usbControllerState.direction = 2;
	}
	
	// Take care of the d-pad analog pressures separately,
	//  since the 'convert to hat switch' code is confusing
	if (btnList.dpadUpOn == 1)
		usbControllerState.up_axis = 0xFF;	
	else
		usbControllerState.up_axis = 0;
	if (btnList.dpadRightOn == 1)
		usbControllerState.right_axis = 0xFF;
	else
		usbControllerState.right_axis = 0;
	if (btnList.dpadDownOn == 1)
		usbControllerState.down_axis = 0xFF;
	else
		usbControllerState.down_axis = 0;
	if (btnList.dpadLeftOn == 1)
		usbControllerState.left_axis = 0xFF;
	else
		usbControllerState.left_axis = 0;


	// left and right analog sticks, 0x00 left/up, 0x80 middle, 0xff right/down
	// Sanity check the inputs so we don't try and go out of bounds
	int16_t stickMin = 0;
	int16_t stickMax = 1023;
	if (btnList.leftStickX < stickMin)
		btnList.leftStickX = stickMin;
	if (btnList.leftStickX > stickMax)
		btnList.leftStickX = stickMax;
	if (btnList.leftStickY < stickMin)
		btnList.leftStickY = stickMin;
	if (btnList.leftStickY > stickMax)
		btnList.leftStickY = stickMax;

	if (btnList.rightStickX < stickMin)
		btnList.rightStickX = stickMin;
	if (btnList.rightStickX > stickMax)
		btnList.rightStickX = stickMax;
	if (btnList.rightStickY < stickMin)
		btnList.rightStickY = stickMin;
	if (btnList.rightStickY > stickMax)
		btnList.rightStickY = stickMax;

	if (btnList.stick3X < stickMin)
		btnList.stick3X = stickMin;
	if (btnList.stick3X > stickMax)
		btnList.stick3X = stickMax;
	if (btnList.stick3Y < stickMin)
		btnList.stick3Y = stickMin;
	if (btnList.stick3Y > stickMax)
		btnList.stick3Y = stickMax;

	
	usbControllerState.l_x_axis = btnList.leftStickX;
	usbControllerState.l_y_axis = btnList.leftStickY;
	usbControllerState.r_x_axis = btnList.rightStickX;
	usbControllerState.r_y_axis = btnList.rightStickY;
	usbControllerState.x_3_axis = btnList.stick3X;
	usbControllerState.y_3_axis = btnList.stick3Y;
	
	// Send the data out via USB
	return usb_gamepad_send();
}

int8_t usb_gamepad_send(void) {
	uint8_t intr_state, timeout, i;

	if (!usb_configuration) return -1;
	intr_state = SREG;
	cli();
	UENUM = GAMEPAD_ENDPOINT;
	timeout = UDFNUML + 50;
	while (1) {
		// are we ready to transmit?
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		// has the USB gone offline?
		if (!usb_configuration) return -1;
		// have we waited too long?
		if (UDFNUML == timeout) return -1;
		// get ready to try checking again
		intr_state = SREG;
		cli();
		UENUM = GAMEPAD_ENDPOINT;
	}

	for (i=0; i<sizeof(gamepad_state_t); i++) {
		UEDATX = ((uint8_t*)&usbControllerState)[i];
	}

	UEINTX = 0x3A;
	SREG = intr_state;
	return 0;
}

/**************************************************************************
 *
 *  Private Functions - not intended for general user consumption....
 *
 **************************************************************************/

ISR(USB_GEN_vect)
{
	uint8_t intbits;

	intbits = UDINT;
	UDINT = 0;
	if (intbits & (1<<EORSTI)) {
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1<<RXSTPE);
		usb_configuration = 0;
	}
}

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1<<RXOUTI);
}

// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR(USB_COM_vect)
{
	uint8_t intbits;
	const uint8_t *list;
	const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;

	UENUM = 0;
	intbits = UEINTX;
	if (intbits & (1<<RXSTPI)) {
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);
		UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
		if (bRequest == GET_DESCRIPTOR) {
			list = (const uint8_t *)descriptor_list;
			for (i=0; ; i++) {
				if (i >= NUM_DESC_LIST) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
					return;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue) {
					list += sizeof(struct descriptor_list_struct);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex) {
					list += sizeof(struct descriptor_list_struct)-2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length) len = desc_length;
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--) {
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			return;
		}
		if (bRequest == SET_ADDRESS) {
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			return;
		}
		if (bRequest == SET_CONFIGURATION && bmRequestType == 0) {
			usb_configuration = wValue;
			usb_send_in();
			cfg = endpoint_config_table;
			for (i=1; i<5; i++) {
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en) {
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
			UERST = 0x1E;
			UERST = 0;
			return;
		}
		if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {
			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			return;
		}

		if (bRequest == GET_STATUS) {
			usb_wait_in_ready();
			i = 0;
			#ifdef SUPPORT_ENDPOINT_HALT
			if (bmRequestType == 0x82) {
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			#endif
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
		#ifdef SUPPORT_ENDPOINT_HALT
		if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
		  && bmRequestType == 0x02 && wValue == 0) {
			i = wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT) {
				usb_send_in();
				UENUM = i;
				if (bRequest == SET_FEATURE) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);
				} else {
					UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}
		}
		#endif
		if (wIndex == GAMEPAD_INTERFACE) {
			if (bmRequestType == 0xA1) {
				if (bRequest == HID_GET_REPORT) {
					usb_wait_in_ready();

					for (i=0; i<sizeof(magic_init_bytes); i++) {
						UEDATX = pgm_read_byte(&magic_init_bytes[i]);
					}

					usb_send_in();
					return;
				}
				if (bRequest == HID_GET_IDLE) {
					usb_wait_in_ready();
					UEDATX = gamepad_idle_config;
					usb_send_in();
					return;
				}
				if (bRequest == HID_GET_PROTOCOL) {
					usb_wait_in_ready();
					UEDATX = gamepad_protocol;
					usb_send_in();
					return;
				}
			}
			if (bmRequestType == 0x21) {
				if (bRequest == HID_SET_REPORT) {
					usb_wait_receive_out();
					usb_ack_out();
					usb_send_in();
					return;
				}
				if (bRequest == HID_SET_IDLE) {
					gamepad_idle_config = (wValue >> 8);
					usb_send_in();
					return;
				}
				if (bRequest == HID_SET_PROTOCOL) {
					gamepad_protocol = wValue;
					usb_send_in();
					return;
				}
			}
		}
	}
	UECONX = (1<<STALLRQ) | (1<<EPEN);	// stall
}



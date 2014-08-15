/*
 * pindefs.h
 * 
 * Created: 12/13/2011 1:44:40 PM
 *  Author: Alan Chatham
 * 
 *   This file contains all the pin definitions for the controller
 */ 


#ifndef PINDEFS_H_
#define PINDEFS_H_

	// Change this define to update the code for new board revisions that use different pins
	#define December2011Board
	
	// Definitions for the December 2011 board
	#ifdef December2011Board
		
		#define R3			0
		#define R3_PIN		PINA
		#define START		1
		#define START_PIN	PINA
		#define R2			2
		#define R2_PIN		PINA
		#define R1			3
		#define R1_PIN		PINA
		#define TRIANGLE	4
		#define TRIANGLE_PIN PINA
		#define CIRCLE		5
		#define CIRCLE_PIN	PINA
		#define SQUARE		6
		#define SQUARE_PIN	PINA
		#define CROSS		7
		#define CROSS_PIN	PINA
		#define L1			2
		#define L1_PIN		PINE
		#define L2			7
		#define L2_PIN		PINC
		#define SELECT		6
		#define SELECT_PIN	PINC
		#define L3			5
		#define L3_PIN		PINC
		
		#define JOY_PLUS	3
		#define JOY_PLUS_PORT  PORTC 
		#define JOY_MINUS   4
		#define JOY_MINUS_PORT PORTC
		#define COMMON_1	0
		#define COMMON_1_PORT  PORTE
		#define COMMON_2	7
		#define COMMON_2_PORT  PORTD 
		
		
		#define DPADDOWN	2
		#define DPADDOWN_PIN  PINC
		#define DPADRIGHT	1
		#define DPADRIGHT_PIN PINC
		#define DPADLEFT	0
		#define DPADLEFT_PIN  PINC
		#define DPADUP		1
		#define DPADUP_PIN    PINE

		#define MENU		6
		#define MENU_PIN	PIND
		#define LED			5
		#define LED_PORT	PORTD
		#define PROGRAM		4
		#define PROGRAM_PIN PIND
	
		#define LEFTSTICKX		5
		#define LEFTSTICKX_PIN  PINF
		#define LEFTSTICKY		4
		#define LEFTSTICKY_PIN  PINF
		#define RIGHTSTICKX		7
		#define RIGHTSTICKX_PIN PINF
		#define RIGHTSTICKY		6
		#define RIGHTSTICKY_PIN PINF
	
		#define JOY_DDR			DDRF
		#define JOY_PORT		PORTF
		#define JOY3X			3 //1
		#define JOY3X_PIN		PINF
		#define JOY3Y			2 //0
		#define JOY3Y_PIN		PINF
		#define JOY4X			1 //3
		#define JOY4X_PIN		PINF
		#define JOY4Y			0 //2
		#define JOY4Y_PIN		PINF
		#define JOY3_BUTTON		7 //6
		#define JOY3_BUTTON_PIN	PINE
		#define JOY4_BUTTON		6 //7
		#define JOY4_BUTTON_PIN	PINE

		#define HANDSFREE_1		3
		#define HANDSFREE_1_PIN PIND
		#define HANDSFREE_2		2
		#define HANDSFREE_2_PIN PIND
		#define HANDSFREE_3		5
		#define HANDSFREE_3_PIN PINE
		#define HANDSFREE_4		4
		#define HANDSFREE_4_PIN PINE

		#define BUTTON_BELOW_L_STICK 7
		#define BUTTON_BELOW_L_STICK_PIN PINB
		#define BUTTON_BELOW_R_STICK 6
		#define BUTTON_BELOW_R_STICK_PIN PINB
		#define SIP_SWITCH 5
		#define SIP_SWITCH_PIN PINB
		#define PUFF_SWITCH 3
		#define PUFF_SWITCH_PIN PINE
		
		#define WII_SCL			0
		#define WII_SCL_PORT	PORTD
		#define WII_SDA			1
		#define WII_SDA_PORT	PORTD
		#define PS2_ACK			4
		#define PS2_ACK_PORT	PORTB
		#define PS2_MISO		2
		#define PS2_MISO_PORT	PORTB
		#define PS2_MOSI		3
		#define PS2_MOST_PORT	PORTB
		#define PS2_CLK			1
		#define PS2_CLK_PORT	PORTB
		#define PS2_SS			0
		#define PS2_SS_PORT		PORTB
		
	#endif // End of the December 2011 board


#endif /* PINDEFS_H_ */
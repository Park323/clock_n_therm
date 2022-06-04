#include <stm32f10x.h>
#include "prj.h"


void enable_dot_matrix(){
	/*
	row :: pin :: port
		1			9			PC0
		2			14		PC1
		3			8			PC2
		4			12		PC3
		5			1			PC4
		6			7			PC5
		7			2			PC6
		8			5			PC7
	col :: pin :: port
		1			13		PB8
	  2			3			PB9
	  3			4			PB10
	  4			10		PB11
	  5			6			PB12
	  6			11		PB13
	  7			15		PB14
	  8			16		PB15 */
	
	// enable port A-C(2,3,4) & AFIO(0)
	RCC->APB2ENR |= 0x0000001D;
	// set ports for output mode
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;
}


void enable_keypad(){
	/*
	col ::	pin :: port
		1			K0			PA8
		2			K1			PA9
		3			K2			PA10
		4 		K3			PA11
	row ::	pin :: port
		1			K4			PC8
		2			K5			PC9
		3			K6			PC10
		4			K7			PC11
	send HIGH to col then push the button,
	corresponding row port will get LOW signal.*/
	
	// enable port A and C
	RCC->APB2ENR |= 0x00000014;
	// set port C push-pull output mode
	GPIOC->CRH |= 0x00003333;
	GPIOC->CRH &= 0xFFFF3333;
	// set port A pull-up/down input mode
	GPIOA->CRH |= 0x00008888;
	GPIOA->CRH &= 0xFFFF8888;
	// set pull-up mode (default HIGH)
	GPIOA->ODR |= 0x0F00;
}





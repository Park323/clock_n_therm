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
		1			13		PB15
	  2			3			PB14
	  3			4			PB14
	  4			10		PB12
	  5			6			PB11
	  6			11		PB10
	  7			15		PB9
	  8			16		PB8  */
	
	// enable port A-C(2,3,4) & AFIO(0)
	RCC->APB2ENR |= 0x0000001D;
	// set ports for output mode
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;
	
	enable_TIM1(); //use timer for dot matrix output
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
	
	enable_TIM3(); //use timer for keypad input
}


void enable_TIM1(){
	/* enable TIM1 */
	/* TIM1 is used for dot matrix output */
	RCC->APB2ENR |= 1 << 11; //enable TIM1
	
	TIM1->CR1 = 0x00;
	TIM1->CR2 = 0x00;
	TIM1->PSC = 0x01;
	TIM1->ARR = 0x2000;
	
	TIM1->DIER |= 1; // enable update interrupt
	NVIC->ISER[0] |= 1 << 25; // TIM1 update interrupt
	
	//activate clock
	TIM1->CR1 |= 0x0001;
}


void enable_TIM3(){
	/* enable TIM3 */
	/* TIM3 is used for checking keypad input */
	RCC->APB1ENR |= 1 << 1; //enable TIM3
	
	TIM3->CR1 = 0x00;
	TIM3->CR2 = 0x00;
	TIM3->PSC = 0x10;	//match timer with TIM1
	TIM3->ARR = 0x2000;
	
	TIM3->DIER |= 1;
	NVIC->ISER[0] |= 1 <<29;
	
	//activate clock
	TIM3->CR1 |= 0x0001;
}


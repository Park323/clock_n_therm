#include <stm32f10x.h>
#include "prj.h"

/*
Dot matrix shows hour or temperature.
*/

int i_d,j_d;
u8 k=0;
u32 t=0;
u32 display_row = 1;

// Data information to display. 
u8 font8x8[16][8]={
	{0x3c, 0x42, 0x46, 0x4a, 0x52, 0x62, 0x3c, 0x00}, // 0
	{0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x7c, 0x00}, // 1
	{0x3c, 0x42, 0x02, 0x0c, 0x30, 0x42, 0x7e, 0x00}, // 2
	{0x3c, 0x42, 0x02, 0x1c, 0x02, 0x42, 0x3c, 0x00}, // 3
	{0x08, 0x18, 0x28, 0x48, 0xfe, 0x08, 0x1c, 0x00}, // 4
	{0x7e, 0x40, 0x7c, 0x02, 0x02, 0x42, 0x3c, 0x00}, // 5
	{0x1c, 0x20, 0x40, 0x7c, 0x42, 0x42, 0x3c, 0x00}, // 6
	{0x7e, 0x42, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00}, // 7
	{0x3c, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x3c, 0x00}, // 8
	{0x3c, 0x42, 0x42, 0x3e, 0x02, 0x04, 0x38, 0x00}, // 9
	{0x18, 0x24, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x00}, // A
	{0x7c, 0x22, 0x22, 0x3c, 0x22, 0x22, 0x7c, 0x00}, // B
	{0x1c, 0x22, 0x40, 0x40, 0x40, 0x22, 0x1c, 0x00}, // C
	{0x78, 0x24, 0x22, 0x22, 0x22, 0x24, 0x78, 0x00}, // D
	{0x7e, 0x22, 0x28, 0x38, 0x28, 0x22, 0x7e, 0x00}, // E
	{0x7e, 0x22, 0x28, 0x38, 0x28, 0x20, 0x70, 0x00} // F
};
uint64_t rawdata[8] = {
	0x3c103c3c087e, 
	0x423042421840, 
	0x46500202287c, 
	0x4a100c1c4802, 
	0x52103002fe02, 
	0x621042420842, 
	0x3c7c7e3c1c3c, 
	0x000000000000 
};

// Array size can be changed. Example array.
u16 display[8]; // array of memory acceessed by DMA.  

void activate_display(){
	/*set DMA*/
	/* DMA1 channel2 is connected with TIM2 */
	RCC->AHBENR |= 1;
	DMA1_Channel2->CCR = 0x000000B0; //MINC mode & No-PINC mode & Circular mode & memory2peripheral
	DMA1_Channel2->CNDTR = 8;				//Need to change according to size of 'display' array
	DMA1_Channel2->CPAR = (u32) MATRIX_COL;
	DMA1_Channel2->CMAR = (u32)	display;
	DMA1_Channel2->CCR |= 1;
}


void enable_dot_matrix(){
	/* reference : Program 11.1
	display_row :: pin :: port
		1			9			PB8
		2			14		PB9
		3			8			PB10
		4			12		PB11
		5			1			PB12
		6			7			PB13
		7			2			PB14
		8			5			PB15
	col :: pin :: port
		1			13		PC7
	  2			3			PC6
	  3			4			PC5
	  4			10		PC4
	  5			6			PC3
	  6			11		PC2
	  7			15		PC1
	  8			16		PC0  */
	
	// enable port B,C(3,4) & AFIO(0)
	RCC->APB2ENR |= 0x00000019;
	// set ports for output mode
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;
	
	enable_TIM2(); //use timer for dot matrix output
}


void enable_TIM2(){
	/* enable TIM2 */
	/* TIM2 is used for printing dot matrix */
	/* TIM2 uses DMA1_channel2 */
	RCC->APB2ENR |= 1 << 11; //enable TIM1											//TIM1? not TIM2?
	
	TIM2->CR1 = 0x00;
	TIM2->CR1 |= 1<<2; //only overflow generate DMA requests
	TIM2->CR2 = 0x00;
	TIM2->PSC = 0x01;
	TIM2->ARR = 0x2000;														//Adjustment required.
	
	TIM2->DIER |= 1; // enable update interrupt
	TIM2->DIER |= 1 << 8; //enable DMA requests
	NVIC->ISER[0] |= 1 << 28; // TIM2 global interrupt
	
	//activate clock
	TIM2->CR1 |= 0x0001;
}


void TIM2_IRQHandler (void){
	if ((TIM2->SR & 0x0001) != 0){
		if (t == 1000) {													//'Display array' is scrolled 1 bit per 1000 interrupt.
			t=0;
			j_d = sizeof(rawdata);
			if(k<j_d) {
				for(i_d=0; i_d<8; i_d++) {
					display[i_d] = rawdata[i_d] >> j_d-k;		//for start from right edge
				}
			}
			else {
				for(i_d=0; i_d<8; i_d++) {
					display[i_d] = rawdata[i_d] << k-j_d;		//scroll to left
				}
			}
		k++;
		if(k==(j_d*2)) {
			k=0;																		//scroll init
		}
	}
				GPIOB->ODR = (~display_row)<<8;
					display_row = display_row<<1;
					if (display_row == 0x100) {display_row =1; }
			t++;
		TIM2->SR &= ~(1<<0); // clear UIF
	}
}

#include <stm32f10x.h>
#include "prj.h"

/*
Dot matrix shows hour or temperature.
*/

extern u8 clk_tmp;

int i_d;
u8 base=0;
u32 update_counter=0;
u8 display_length = 8;
u32 display_row = 1;
u8 scroll_mode=0;

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
u8 symbol8x8[3][8]={
	{0x18, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00}, //degree sign
	{0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, //':' sign
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //' 'sign
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
u8 seq_length = sizeof(rawdata);

uint64_t vertical_data[24];// Data information to display. 

// Array size can be changed. Example array.
u8 display[8]; // array of memory acceessed by DMA.  


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


void display_hhmmss(u8 hh, u8 mm, u8 ss){
	/* ":" should be added */
	for(u8 i=0; i<8; i++) {
		rawdata[i] = 0;
		rawdata[i] += ((uint64_t)font8x8[hh/10][i]<<56)+((uint64_t)font8x8[hh%10][i]<<48); //hh
		rawdata[i] += ((uint64_t)symbol8x8[1][i]<<40); //':'
		rawdata[i] += ((uint64_t)font8x8[mm/10][i]<<32)+((uint64_t)font8x8[mm%10][i]<<24); //mm
		rawdata[i] += ((uint64_t)symbol8x8[1][i]<<16); //':'
		rawdata[i] += ((uint64_t)font8x8[ss/10][i]<<8)+((uint64_t)font8x8[ss%10][i]<<0); //ss
	}
}

void display_mnC (u8 m, u8 n, u32 temp_mode) {
		for(u8 j=0; j<8; j++) {
			rawdata[j] += (uint64_t)(font8x8[m][j]<<24);	//m
			rawdata[j] += (uint64_t)(font8x8[n][j]<<16);		//n
			rawdata[j] += (uint64_t)(symbol8x8[0][j]<<8);		//'
			switch (temp_mode) {
				case 0 :
					rawdata[j] += ((uint64_t)font8x8[0xC][j]);	//C
					break;
				case 1 :
					rawdata[j] += ((uint64_t)font8x8[0xF][j]);	//F
				break;
			}
		}
}


void switch_scrolling(u8 index){
	scroll_mode = index;
}


void horizon2vertical(void) {
	switch (clk_tmp) {
		case 0 :
			for(i_d=0; i_d<24; i_d++) {
				if(i_d<8) 			vertical_data[i_d] = rawdata[i_d] >> 12*4;
				else if(i_d>15) vertical_data[i_d] = rawdata[i_d-16] & 0x0000FFFF;
				else						vertical_data[i_d] = (rawdata[i_d-8] >> (6*4)) & 0x0000FFFF;
			}
			break;
		case 1 :
			for(i_d=0; i_d<24; i_d++) {
				if(i_d<8) 			vertical_data[i_d] = rawdata[i_d] >> (8*4);
				else if(i_d>15) vertical_data[i_d] = rawdata[i_d-16] & 0x0000FFFF;
				else 						vertical_data[i_d] = (rawdata[i_d-8] >> (4*4)) & 0x0000FFFF;
			}
			break;
	}
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
	RCC->APB1ENR |= 1; //enable TIM2
	
	TIM2->CR1 = 0x00;
	TIM2->CR1 |= 1<<2; //only overflow generate DMA requests
	TIM2->CR2 = 0x00;
	TIM2->PSC = 0x05;
	TIM2->ARR = 0x2000;
	
	TIM2->DIER |= 1; // enable update interrupt
	TIM2->DIER |= 1 << 8; //enable DMA requests
	NVIC->ISER[0] |= 1 << 28; // TIM2 global interrupt
	
	//activate clock
	TIM2->CR1 |= 0x0001;
}


void TIM2_IRQHandler (void){
	if ((TIM2->SR & 0x0001) != 0){
		if (update_counter++ == 100) {													//'Display array' is scrolled 1 bit per 100 interrupt.
			update_counter = 0;
			switch (scroll_mode){
				case 0 :
					//horizontal_left mode
					for(i_d=0; i_d<8; i_d++){
						if(seq_length > base) display[i_d] = (u8)(rawdata[i_d] >> (seq_length - base));		//for start from right edge
						else 									display[i_d] = (u8)(rawdata[i_d] << (base - seq_length));
					}
					if(++base==(seq_length + display_length*2)) base=0; //scroll init
					break;
				case 1 :
					//horizontal_right mode
					for(i_d=0; i_d<8; i_d++){
						if(display_length > base) display[i_d] = (u8)(rawdata[i_d] << (display_length - base));		//for start from right edge
						else 											display[i_d] = (u8)(rawdata[i_d] >> (base - display_length));
					}
					if(++base==(seq_length + display_length*2)) base=0; //scroll init
					break;
				case 2 :
					//vertical up mode
					horizon2vertical();
					for(i_d=0; i_d<8; i_d++) {
						if (i_d+base-7<0 || i_d+base-7>23){
							display[i_d] = 0;
						}
						else {
						display[i_d] = vertical_data[i_d+base-7];
						}
					}
					base++;
					if (base==30) base=0;
					break;
				case 3 :
					//vertical down mode
					horizon2vertical();
					for(i_d=0; i_d<8; i_d++) {
						if (23+i_d-base<0 || 23+i_d-base>23){
							display[i_d] = 0;
						}
						else {
						display[i_d] = vertical_data[23+i_d-base];
						}
					}
					base++;
					if (base==30) base=0;
					break;
			}
		}
		GPIOB->ODR = (~display_row)<<8;
		display_row = display_row<<1;
		if (display_row == 0x100) display_row =1;
		TIM2->SR &= ~(1<<0); // clear UIF
	}
}

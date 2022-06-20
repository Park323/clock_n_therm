#include <stm32f10x.h>
#include "prj.h"

/*
Dot matrix shows hour or temperature.
*/

extern u8 HMS;
extern u8 scroll_mode, CLKEN, CLK_CONFIG;
extern u8 hour, min, sec;
extern u8 temp_conv_10, temp_conv_1, temp_mode;

int i_d;
u8 base=0;
u32 update_counter=0;
u8 display_length = 16;
u32 display_row = 1;
u8 show=1;
int p=0;
u16 temp;

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
	0x3c10183c3c18087e, 
	0x4230184242181840, 
	0x465000020200287c, 
	0x4a10000c1c004802, 
	0x521000300200fe02, 
	0x6210184242180842, 
	0x3c7c187e3c181c3c, 
	0x0000000000000000 
};
u8 seq_length = sizeof(rawdata);

uint64_t vertical_data[24];// Data information to display. 

// Array size can be changed. Example array.
u16 display[8]; // array of memory acceessed by DMA.  


void activate_display(){
	/*set DMA*/
	/* DMA1 channel2 is connected with TIM2 */
	RCC->AHBENR |= 1;
	DMA1_Channel2->CCR = 0x000005B0; //MINC mode & No-PINC mode & Circular mode & memory2peripheral
	DMA1_Channel2->CNDTR = 8;				//Need to change according to size of 'display' array
	DMA1_Channel2->CPAR = (u32) MATRIX_COL;
	DMA1_Channel2->CMAR = (u32)	display;
	DMA1_Channel2->CCR |= 1;
}

void display_hhmmss(u8 hh, u8 mm, u8 ss){
	for(u8 i=0; i<8; i++) {
		rawdata[i] = 0;
		if (hh!=100){
			rawdata[i] += ((uint64_t)font8x8[hh/10][i]<<56)+((uint64_t)font8x8[hh%10][i]<<48); //hh
			rawdata[i] += ((uint64_t)symbol8x8[1][i]<<40); //':'
			rawdata[i] += ((uint64_t)font8x8[mm/10][i]<<32)+((uint64_t)font8x8[mm%10][i]<<24); //mm
			rawdata[i] += ((uint64_t)symbol8x8[1][i]<<16); //':'
			rawdata[i] += ((uint64_t)font8x8[ss/10][i]<<8)+((uint64_t)font8x8[ss%10][i]<<0); //ss
		}
	}
}

void display_mnC (u8 m, u8 n, u32 temp_mode) {
		for(u8 j=0; j<8; j++) {
			rawdata[j] = 0;
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

void display_nn(u8 nn) {
	for(u8 i=0; i<8; i++) {
		display[i] = (font8x8[(nn/10)][i]<<8) + font8x8[(nn%10)][i];
	}
}


void horizon2vertical(void) {
	switch (CLKEN) {
		case 1 :
			for(i_d=0; i_d<24; i_d++) {
				if(i_d<8) 			vertical_data[i_d] = rawdata[i_d] >> 12*4;
				else if(i_d>15) vertical_data[i_d] = rawdata[i_d-16] & 0x0000FFFF;
				else						vertical_data[i_d] = (rawdata[i_d-8] >> (6*4)) & 0x0000FFFF;
			}
			break;
		case 0 :
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
	 row :: pin :: port
		1			9			PB8
		2			14		PB9
		3			8			PB10
		4			12		PB11
		5			1			PB12
		6			7			PB13
		7			2			PB14
		8			5			PB15
#1_col :: pin :: port
		1			13		PC7
	  2			3			PC6
	  3			4			PC5
	  4			10		PC4
	  5			6			PC3
	  6			11		PC2
	  7			15		PC1
	  8			16		PC0 
#2_col :: pin :: port
		9			13		PA15
	  10		3			PA14
	  11		4			PA13
	  12		10		PA12
	  13		6			PA11
	  14		11		PA10
	  15		15		PA9
	  16		16		PA8 
*/
	
	// enable port B,C(3,4) & AFIO(0)
	RCC->APB2ENR |= 0x0000001D;
	// set ports for output mode
	AFIO->MAPR |= 0x04000000;
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;
	GPIOA->CRH = 0x33333333;
	
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
			if (CLK_CONFIG == 1) {
				switch (HMS) {
					case 0:
						display_nn(hour);
						break;
					case 1:
						display_nn(min);
						break;
					case 2:
						display_nn(sec);
						break;
				}
			}
			else {
				switch (scroll_mode){
					case 0 :
						//horizontal_left mode
						for(i_d=0; i_d<8; i_d++){
							if(seq_length > base) display[i_d] = (u16)(rawdata[i_d] >> (seq_length - base));		//for start from right edge
							else 									display[i_d] = (u16)(rawdata[i_d] << (base - seq_length));
						}
						if(++base>=(seq_length + display_length*2)) base=0; //scroll init
						break;
					case 1 :
						//horizontal_right mode
						for(i_d=0; i_d<8; i_d++){
							if(display_length > base) display[i_d] = (u16)(rawdata[i_d] << (display_length - base));		//for start from right edge
							else 											display[i_d] = (u16)(rawdata[i_d] >> (base - display_length));
						}
						if(++base>=(seq_length + display_length*2)) base=0; //scroll init
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
						if (base>=30) base=0;
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
						if (base>=30) base=0;
						break;
				}
			}
		}
		temp =(display[p]>>8);
		GPIOA->BSRR = 0xFF000000;
		GPIOA->BSRR = temp<<8;
		p++;
		if (p==8) p=0;
		GPIOB->ODR = (~display_row)<<8;
		display_row = display_row<<1;
		if (display_row == 0x100) display_row =1;
		TIM2->SR &= ~(1<<0); // clear UIF
	}
}


void enable_TIM1(void) {
	/* enable TIM1 */
	/* TIM1 is used update rawdata */
	RCC->APB2ENR |= 0x00000800; //enable TIM1
	
	TIM1->CR1 = 0x00;
	TIM1->CR2 = 0x00;
	TIM1->PSC = 0x05;
	TIM1->ARR = 0x1000;
	
	TIM1->DIER |= 0x0001; // enable update interrupt
	NVIC->ISER[0] |= 0x02000000; // TIM1 update interrupt enable
	
	//activate clock
	TIM1->CR1 |= 0x0001;
}

void TIM1_UP_IRQHandler (void) {
	if ((TIM1->SR & 0001) != 0) {	//UIF check
		if (CLKEN == 1) display_hhmmss(hour, min, sec);
		else display_mnC(temp_conv_10, temp_conv_1, temp_mode);
		TIM1->SR &= ~(1<<0); //clear UIF
	}
}
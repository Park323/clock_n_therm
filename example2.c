#include <stm32f10x.h>
#define DOT_MATRIX_COL	0x4001100C // GPIOC_ODR

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

u32 display_row = 1;
u32 scroll_mode = 2;
u32 k, t = 0;
int i_d, j_d = 0 ;
// Array size can be changed
u16 display[8]; // array of memory acceessed by DMA.
uint64_t rawdata[8] = {0x3c103c3c087e, 0x423042421840, 0x46500202287c, 0x4a100c1c4802, 0x52103002fe02, 0x621042420842, 0x3c7c7e3c1c3c, 0x000000000000 };
uint64_t vertical_data[24];// Data information to display. 
void horizon2vertical(void);

int main (void) {

	horizon2vertical();
	
	RCC->APB2ENR |= 0x0000001D;
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;
	
	RCC->APB1ENR |= 0x00000001;
	TIM2->CR1 = 0x04;
	TIM2->CR2 = 0x00;
	TIM2->PSC = 0x07FF;
	TIM2->ARR = 0x000F;
	TIM2->DIER = 0x0101;
	NVIC->ISER[0]= (1 << 28);
	
	RCC->AHBENR |= 0x00000001;
	DMA1_Channel2->CCR = 0x000005B0;
	DMA1_Channel2->CNDTR = 8; //Need to change according to size of 'display' array
	DMA1_Channel2->CPAR = DOT_MATRIX_COL;
	DMA1_Channel2->CMAR = (u32)display;
	
	DMA1_Channel2->CCR |= 0x00000001;
	
	TIM2->CR1 |= 0x0001;
	
	while (1) {;}
		
} // end main

void TIM2_IRQHandler (void){
	if ((TIM2->SR & 0x0001) != 0){
		//horizontal_left mode
		if(scroll_mode == 0) {
			if (t == 100) {													//'Display array' is scrolled 1 bit per 1000 interrupt.
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
		}
		
		//horizontal_right mode
		if(scroll_mode == 1) {
			if (t == 100) {													//'Display array' is scrolled 1 bit per 1000 interrupt.
				t=0;
				j_d = sizeof(rawdata);
				if(k<j_d) {
					for(i_d=0; i_d<8; i_d++) {
						display[i_d] = rawdata[i_d] << j_d-k;		//for start from left edge
					}
				}
				else {
					for(i_d=0; i_d<8; i_d++) {
						display[i_d] = rawdata[i_d] >> k-j_d;		//scroll to right
					}
				}
			k++;
			if(k==(j_d*2)) {
				k=0;
			}
		}
		}
		
		if(scroll_mode == 2) {
			if (t == 100) {													//'Display array' is scrolled 1 bit per 1000 interrupt.
				t=0;
				for(i_d=0; i_d<8; i_d++) {
					if (i_d+j_d-7<0 || i_d+j_d-7>23){
						display[i_d] = 0;
					}
					else {
					display[i_d] = vertical_data[i_d+j_d-7];
					}
				}
				j_d++;
				if (j_d==30) {
					j_d=0;
				}
			}
		}
		
		if(scroll_mode == 3) {
			if (t == 100) {													
				t=0;
				for(i_d=0; i_d<8; i_d++) {
					if (23+i_d-j_d<0 || 23+i_d-j_d>23){
						display[i_d] = 0;
					}
					else {
					display[i_d] = vertical_data[23+i_d-j_d];
					}
				}
				j_d++;
				if (j_d==30) {
					j_d=0;
				}
			}
		}
		GPIOB->ODR = (~display_row)<<8;
		display_row = display_row<<1;
		if (display_row == 0x100) {display_row =1; }
			t++;
		TIM2->SR &= ~(1<<0); // clear UIF
	}
}

void horizon2vertical(void) {
	for(i_d=0; i_d<24; i_d++) {
		if(i_d<8) {
			vertical_data[i_d] = rawdata[i_d] >> (8*4);
		}
		else if(i_d>15) {
			vertical_data[i_d] = rawdata[i_d-16] & 0x0000FFFF;
		}
		else {
			vertical_data[i_d] = (rawdata[i_d-8] >> (4*4)) & 0x0000FFFF;
		}
	}
}
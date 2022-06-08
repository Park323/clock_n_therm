#include <stm32f10x.h>
#include "prj.h"

extern u8** font8x8;
extern u8 rawdata[];

u16 cur_TMP;
u32 i, temp_mode;

u8 temp_conv, temp_conv_10, temp_conv_1;

void enable_TMP(){
	/*
	ADC uses PA4 AFIO
	ADC uses DMA1 Channel 1
	*/
	// enable port A and AFIO
	RCC->APB2ENR |= 0x5; 
	// set PA4 analog input mode
	GPIOA->CRL &= 0x44404444;
	
	//set DMA1 clock
	RCC->AHBENR |= 1;
	DMA1_Channel1->CCR = 0x00003520; // Priority very high - MSIZE & PSIZE 8 bits - Circular mode & peripheral2memory
	DMA1_Channel1->CNDTR = 1;
	DMA1_Channel1->CPAR = (u32) ADC1_DR_BASE;
	DMA1_Channel1->CMAR = (u32) &cur_TMP;
	
	//set ADC1
	RCC->APB2ENR |= 1 << 9;
	ADC1->CR1 = 0x00000000;
	ADC1->CR2 = 0x001E0102;
	ADC1->SMPR2 = 0x00007000;
	ADC1->SQR1 = 0x00000000;
	ADC1->SQR2 = 0x00000000;
	ADC1->SQR3 = 0x00000004;
	
	//enable
	DMA1_Channel1->CCR |= 1;
	ADC1->CR2 |= 1;
	ADC1->CR2 |= 1 << 22; //SWSTART
}

void tmp2data() {						//use TIM4. TIM4 causes update interrupt. cur_TMP is converted to rawdata.
	/* enable TIM4 */
	/* TIM4 is used for update 'rawdata' with temparature */
	RCC->APB1ENR |= 1 << 2; //enable TIM4
	
	TIM4->CR1 = 0x00;
	TIM4->CR2 = 0x00;
	TIM4->PSC = 0x10;	
	TIM4->ARR = 0x2000;				//adjustment required.
	
	TIM4->DIER |= 1;					//update interrupt enable.
	NVIC->ISER[0] |= 1 <<30;	//TIM4 global interrupt enable.
	
	//activate clock
	TIM4->CR1 |= 0x0001;
}

void tmp2data_off() {
	TIM4->CR1 &= ~(1<<0);			//TIM4 disabled.
}


void TIM4_IRQHandler (void){
	if ((TIM4->SR & 0x0001) != 0){
		temp_conv = cur_TMP;//temp_conv = Celsius temparature. round down after devide.
		if(temp_mode == 1) temp_conv = temp_conv*1.8+32;	//farenhiet conversion
		temp_conv_10 = temp_conv/10;		//tens of temp.
		temp_conv_1 = temp_conv%10;		//units of temp.
		display_mnC (temp_conv_10, temp_conv_1, temp_mode);
		TIM4->SR &= ~(1<<0);		//clear UIF
	}
}

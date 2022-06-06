#include <stm32f10x.h>
#include "prj.h"

u16 cur_TMP;

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

void tmp2data() {						//use TIM4. TIM4 causes update interrupt. ADC_DR is converted to rawdata.
	/* enable TIM4 */
	/* TIM4 is used for update 'rawdata' with temparature */
	RCC->APB1ENR |= 1 << 3; //enable TIM4
	
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
		temp_cel = cur_TMP;			//Celsius temparature. round down after devide.
		temp_cel_10 = temp_cel/10		//tens of celsius temp.
		temp_cel_1 = temp_cel-(10*temp_cel_10);		//units of celsius temp.
		
		for(i=0, i<8, i++) {
			rawdata[i] = (font8x8[temp_cel_10][i]*0x10000)+(font8x8[temp_cel_1][i]*0x100)+font8x8[0xC][i]			//nnC type
			rawdata[i] = rawdata[i]*0x1000000 + rawdata[i]				//nnCnnC type
		}
	TIM4->SR &= ~(1<<0);		//clear UIF
}
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
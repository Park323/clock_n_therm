#include <stm32f10x.h>
#include "prj.h"

u16 cur_TMP;

void clk2data() {						//use TIM5. TIM5 causes update interrupt. clock is converted to rawdata.
	/* enable TIM5 */
	/* TIM5 is used for update 'rawdata' with clock */
	RCC->APB1ENR |= 1 << 3; //enable TIM5
	
	TIM5->CR1 = 0x00;
	TIM5->CR2 = 0x00;
	TIM5->PSC = 0x10;	
	TIM5->ARR = 0x2000;				//adjustment required.
	
	TIM5->DIER |= 1;					//update interrupt enable.
	NVIC->ISER[1] |= 1 <<18;	//TIM5 glabal interrupt *May have error!!!(interrupt no.)*
	
	//activate clock
	TIM5->CR1 |= 0x0001;
}

void clk2data_off() {
	TIM5->CR1 &= ~(1<<0);			//TIM5 disabled.
}

void TIM5_IRQHandler (void){
	if ((TIM5->SR & 0x0001) != 0){
	TIM5->SR &= ~(1<<0);		//clear UIF
	}
}
/*
*/
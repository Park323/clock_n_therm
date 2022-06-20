#include <stm32f10x.h>
#include "prj.h"

/* PA0 : USART2_CTS
PA1 : USART2_RTS
PA2 : USART2_TX
PA3 : USART2_RX */

u8 HMS;
u8 scroll_mode, CLK_CONFIG;
u8 CLKEN = 1;
u8 hour=12, min=0, sec=0;
u8 temp_conv_10, temp_conv_1, temp_mode;

u8 num_words = 8;
u8 word_idx = 0;


void enable_Rx(void){
	RCC->APB2ENR |= 0x00000004; //GPIOA clock enable
	RCC->APB1ENR |= 0x00020000;	//USART2 clock enable
	
	GPIOA->CRL &= ~(0xFFu << 8); //PA2, PA3 mode reset
	GPIOA->CRL |= (0x04B << 8);	//PA2 : AF output pushpull, PA3 : input float
	
	USART2->BRR = 0x753;	//baudrate = 19200
	
	NVIC->ISER[1] |= (1<<6); //USART2 global interrupt enable
	USART2->CR1 |= 0x00000020; // RXNEIE bit set
	
	USART2->CR1 |= 0x00000004; //RE bit
	USART2->CR1 |= 0x00002000; //UE set
}


void USART2_IRQHandler (void) {
	if(USART2->SR & 20){
		switch (word_idx){
			case 0 :
				scroll_mode = USART2->DR;
				break;
			case 1 :
				CLKEN = USART2->DR;
				break;
			case 2 :
				hour = USART2->DR; // hour_d? it's good.
				break;
			case 3 :
				min = USART2->DR;
				break;
			case 4 :
				sec = USART2->DR;
				break;
			case 5 :
				temp_conv_10 = USART2->DR;
				break;
			case 6 :
				temp_conv_1 = USART2->DR;
				break;
			case 7 :
				temp_mode = USART2->DR;
				break;
			case 8 :
				CLK_CONFIG = USART2->DR;
				break;
			case 9 :
				HMS = USART2->DR;
				break;
		}
		if (++word_idx == 10) word_idx = 0; 
	}
}
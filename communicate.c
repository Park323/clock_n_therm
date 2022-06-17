#include <stm32f10x.h>
#include "prj.h"

/* PA0 : USART2_CTS
PA1 : USART2_RTS
PA2 : USART2_TX
PA3 : USART2_RX */

extern u8 scroll_mode;
extern u8 CLKEN;
extern u8 hour_d, min, sec;
extern u8 temp_conv_10, temp_conv_1, temp_mode;

u8 num_words = 8;
u8 word_idx = 0;

void enable_Tx(void){
	RCC->APB2ENR |= 0x00000004; //GPIOA clock enable
	GPIOA->CRL &= ~(0xFFu << 8); //PA2 mode reset
	GPIOA->CRL |= (0x04B << 8);	//PA2 : AF output pushpull
	
	USART3->BRR = 0x753;	//baudrate = 19200
	
	NVIC->ISER[1] |= (1<<7); //USART3 global interrupt enable
	
	USART3->CR1 |= 0x0088; //TXEIE, TE bit
	USART3->CR2 |= 0x0000;
	USART3->CR3 |= 0x0000;
	USART3->CR1 |= 0x2000; //UE bit
}

void enable_Rx(void){
	RCC->APB2ENR |= 0x00000004; //GPIOA clock enable
	RCC->APB1ENR |= 0x00020000;	//USART2 clock enable
	
	GPIOA->CRL &= ~(0xFFu << 8); //PA2, PA3 mode reset
	GPIOA->CRL |= (0x04B << 8);	//PA2 : AF output pushpull, PA3 : input float
	
	USART3->BRR = 0x753;	//baudrate = 19200
	
	NVIC->ISER[1] |= (1<<7); //USART3 global interrupt enable
	USART1->CR1 |= 0x00000020; // RXNEIE bit set
	
	USART3->CR1 |= 0x00000004; //RE bit
	USART3->CR1 |= 0x00002000; //UE set
}


void USART3_IRQHandler (void) {
	if(USART3->SR & 80){
		switch (word_idx){
			case 0 :
				USART1->DR = scroll_mode;
				break;
			case 1 :
				USART1->DR = CLKEN;
				break;
			case 2 :
				USART1->DR = hour_d;
				break;
			case 3 :
				USART1->DR = min;
				break;
			case 4 :
				USART1->DR = sec;
				break;
			case 5 :
				USART1->DR = temp_conv_10;
				break;
			case 6 :
				USART1->DR = temp_conv_1;
				break;
			case 7 :
				USART1->DR = temp_mode;
				break;
		}
		if (++word_idx == 8) word_idx = 0; 
	}
}
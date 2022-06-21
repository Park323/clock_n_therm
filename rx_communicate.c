#include <stm32f10x.h>
#include "prj.h"

/* PA0 : USART2_CTS
PA1 : USART2_RTS
PA2 : USART2_TX
PA3 : USART2_RX */

u8 HMS;
u8 scroll_mode;
u8 CLK_CONFIG =1;
u8 CLKEN = 1;
u8 hour=0, min=0, sec=0;
u8 temp_conv_10, temp_conv_1, temp_mode;

u8 num_words = 8;
u8 word_idx = 0;
u8 transmit_status;


void enable_Rx(void){
	RCC->APB2ENR |= 0x00004004; //GPIOA clock enable
	GPIOA->CRH |= 0x000B0400;	//PA10 input, PA12(RTS) output
	GPIOA->CRL |= 0x00300000;
	
	USART1->BRR = 0xEA6;	//baudrate = 19200
	
	NVIC->ISER[1] |= (1<<5); //USART2 global interrupt enable
	USART1->CR1 |= 0x00000020; // RXNEIE bit set
	
	USART1->CR1 |= 0x00000004; //RE bit
	USART1->CR1 |= 0x00002000; //UE set
}


void USART1_IRQHandler (void) {
	if(USART1->SR & 0x20){
		if (USART1->DR == 0xEE) {
			word_idx = 0;
			transmit_status = 1;
		}
		if (transmit_status == 1) {
			switch (word_idx){
			case 1 :
				scroll_mode = USART1->DR;
				break;
			case 2 :
				CLKEN = USART1->DR;
				break;
			case 3 :
				hour = USART1->DR; // hour_d? it's good.
				break;
			case 4 :
				min = USART1->DR;
				break;
			case 5 :
				sec = USART1->DR;
				break;
			case 6 :
				temp_conv_10 = USART1->DR;
				break;
			case 7 :
				temp_conv_1 = USART1->DR;
				break;
			case 8 :
				temp_mode = USART1->DR;
				break;
			case 9 :
				CLK_CONFIG = USART1->DR;
				break;
			case 10 :
				HMS = USART1->DR;
				break;
			}
		}
		else {
//			GPIOA->ODR = 0x00000010;
		}
	word_idx++;
	}
}
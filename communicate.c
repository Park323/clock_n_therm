#include <stm32f10x.h>
#include "prj_tx.h"

/* APB1 : 36MHz */
/* PB10 USART3_TX */

extern u8 HMS;
extern u8 scroll_mode;
extern u8 CLKEN, CLK_CONFIG;
extern u8 hour_d, min, sec;
extern u8 temp_conv_10, temp_conv_1, temp_mode;

u8 num_words = 8;
u8 word_idx = 0;

void enable_Tx(void){
	RCC->APB2ENR |= 0x00000008; //GPIOB clock enable
	GPIOB->CRH &= ~(0xFFu << 4*2); //PB10 mode reset
	GPIOB->CRH |= (0x04B << 4*2);	//PB10 : AF output pushpull
	
	USART3->BRR = 0x753;	//baudrate = 19200
	
	NVIC->ISER[1] |= (1<<7); //USART3 global interrupt enable
	
	USART3->CR1 |= 0x0088; //TXEIE, TE bit
	USART3->CR2 |= 0x0000;
	USART3->CR3 |= 0x0000;
	USART3->CR1 |= 0x2000; //UE bit
}


void USART3_IRQHandler (void) {
	if(USART3->SR & 80){
		switch (word_idx){
			case 0 :
				USART3->DR = scroll_mode;
				break;
			case 1 :
				USART3->DR = CLKEN;
				break;
			case 2 :
				USART3->DR = hour_d;
				break;
			case 3 :
				USART3->DR = min;
				break;
			case 4 :
				USART3->DR = sec;
				break;
			case 5 :
				USART3->DR = temp_conv_10;
				break;
			case 6 :
				USART3->DR = temp_conv_1;
				break;
			case 7 :
				USART3->DR = temp_mode;
				break;
			case 8 :
				USART3->DR = CLK_CONFIG;
				break;
			case 9 :
				USART3->DR = HMS;
				break;
		}
		if (++word_idx == 10) word_idx = 0; 
	}
}
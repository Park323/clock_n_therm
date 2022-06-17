#include <stm32f10x.h>
#include "prj.h"

/* PA0 : USART2_CTS
PA1 : USART2_RTS
PA2 : USART2_TX
PA3 : USART2_RX */

u8 scroll_mode;
u8 CLKEN = 1;
u8 hour=12, min=0, sec=0;
u8 temp_conv_10, temp_conv_1, temp_mode;


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
}
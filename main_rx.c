#include <stm32f10x.h>
#include "prj.h"

/*
USART Receiver
connect with dot matrix

receive these values from Tx module
- scroll mode
- hour, min, sec
- temperature, temp_mode

then display the information on dot matrix
*/

volatile u32 delay = 0;

void sleep();
extern u8 transmit_status;

int main(){
	enable_dot_matrix();
	enable_Rx();
	activate_display();
	enable_TIM1();
	enable_TIM3() ;
	
   RCC->APB2ENR |= 0x00000005;
   GPIOA->CRL = 0x00300000;
		GPIOA->CRH &= 0xFFF3FFFF; //PA12 mode reset
   GPIOA->CRH |= 0x00030000;
   static uint32_t led = 1<<5;

   while(1){
      if (led > 0x80)
         led = led>>16;
      else
         led = led<<16;
      GPIOA->BSRR = led;
      sleep();
			if(transmit_status == 1) {
			 GPIOA->ODR |= 0x20;
				GPIOA->BSRR = 0x00001000;
			 break;
		 }
   }

	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

void enable_TIM3(void){
	/* enable TIM3 */
	/* TIM2 is used for printing key matrix */
	RCC->APB1ENR |= 0x2; //enable TIM3
	
	TIM3->CR1 = 0x00;
	TIM3->CR1 |= 1<<2; //only overflow generate DMA requests
	TIM3->CR2 = 0x00;
	// 5 = 36MHz / (7200)*(1000)
	// 1 = 36MHz / (7200)*(5000)
	// 0.1 = 36MHz / (7200)*(50000)
	TIM3->PSC = 0x1c1f;
	TIM3->ARR = 0x587;
	
	TIM3->DIER |= 1; // enable update interrupt
	NVIC->ISER[0] |= 1 << 29; // TIM2 global interrupt
}

void TIM3_IRQHandler (void){
	if ((TIM3->SR & 0x0001) != 0){
		delay++;
		TIM3->SR &= ~(1<<0); // clear UIF
	}
}

void sleep(void){
	//activate clock
	TIM3->CR1 |= 0x0001;
	
	// wait for 0.6 seconds
	while(delay<3){	}
	
	//deactivate clock
	TIM3->CR1 &= ~0x0001;
	delay = 0;
}
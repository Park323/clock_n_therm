/*
USART Transmitter
connect with keypad and TMP sensor
enable clock signal

send these values to Rx module
- scroll mode
- hour, min, sec
- temperature, temp_mode
*/

#include <stm32f10x.h>
#include "prj_tx.h"

static uint32_t led = 1<<5;

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	enable_TIM2();
	enable_keypad();
	
	enable_clk();
	set_TIM1();
	
	enable_TMP();
	tmp2data();
	
	enable_Tx();
	
	READ_FLASH();
	
	GPIOB->CRH &= 0xFF8FFFFF; //PB13 mode reset
	GPIOB->CRH |= 0x00800000;	//PB13 : input
	
	//LED
	RCC->APB2ENR |= 0x00000005;
	GPIOA->CRL &= 0xFF3FFFFF;
	GPIOA->CRL |= 0x00300000;
	
	while(1){
		if (led > 0x80)
			led = led>>16;
		else
			led = led<<16;
		GPIOA->BSRR = led;
		sleep();
		if ((GPIOB->IDR & 0x2000)!=0) break;
	}
	GPIOA->BSRR = 1<<5;
	
	/* Loop waiting for interrupt */
	while (1) {
		for (u8 i=0; i<4; i++)
			scan_button(i);
	}
	//while(1){ __WFI(); }
}

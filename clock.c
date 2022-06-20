#include <stm32f10x.h>
#include "prj_tx.h"
#define RTC_BASE 0x40002800

/*
Our clock use Real-time clock(RTC)
It's independent timer.
*/

u8 CLKEN = 1;
u8 H24 = 1; // 1 if 24 mode else 0
u8 HMS = 0; // H : 0, M : 1, S : 2
u8 CLK_CONFIG = 0;

u8 clk_count = 0;
u8 no_input = 0;
u8 flash = 0;

u8 hour_b=12, min_b=0, sec_b=0;
u8 hour_d=12, hour=12, min=0, sec=0;


void switch_clk(void){
	if (CLKEN != 0) CLKEN = 0;
	else CLKEN = 1;
}

void switch_h24(void){
	if (H24 != 0) H24 = 0;
	else H24 = 1;
}


void enter_RTC_config(void){
	// poll RTOFF
	while((RTC->CRL & 0x0020)==0){}
	// enter config mode
	RTC->CRL |= 1 << 4;
}

void exit_RTC_config(void){
	// exit config mode
	RTC->CRL &= ~(1 << 4);
	// poll RTOFF
	//while((RTC->CRL & 0x0020)==0){}
}


void initialize_RTC(void){
	flash = 1;
	
	enter_RTC_config();
	
	// sync 0.25 second
	RTC->CRH |= 1; //second interrupt enable
	RTC->PRLL = 0x1FFF; // reload value (TR_CLK = RTCCLK / (PRL + 1))
	
	exit_RTC_config();
}


// Clock Configuaration
void enter_clk_config(void){
	CLK_CONFIG = 1;
	RCC->BDCR &= ~(u16)1;	// LSE OFF
	
	reset_input_count();
	TIM1->CR1 |= 0x01; // CONFIG TIMER ON
	enter_RTC_config();
}

void exit_clk_config(void){
	exit_RTC_config();
	TIM1->CR1 &= ~(0x01); // CONFIG TIMER OFF
	RCC->BDCR |= 1;	// LSE ON
	CLK_CONFIG = 0;
	HMS = 0;
}

void backup_clk(void){
	hour_b = hour;
	min_b = min_b;
	sec_b = sec;
}

void restore_clk(void){
	hour = hour_b;
	min = min_b;
	sec = sec_b;
}


void switch_config_unit(u8 direction){
	/* 
	direction
	0 : step next
	1 : step back
	order : Hour/Minute/Second
	*/
	reset_input_count();
	if (direction == 0){
		if (++HMS == 3) HMS=0;
	}
	else {
		if (HMS == 0) HMS = 2;
		else --HMS;
	}
}


void enable_clk(){
	/*initialize real time clock*/
	RCC->APB1ENR |= 3 << 27; //enable power and backup
	PWR->CR |= 1 << 8; //enable access to RTC
	// RTC clock source : LSE(2_01) 
	// LSE : 32.768kHz
	RCC->BDCR |= 1 << 8;
	RCC->BDCR |= 1;	// LSE ON
	
	initialize_RTC();
	
	//RTC and interrupt enable
	RCC->BDCR |= 1 << 15;
	NVIC->ISER[0] |= 1 << 3; //RTC global interrupt
}


void updown_clock(u8 command){
	/* 
	command
	0 : increase
	1 : decrease
	*/
	reset_input_count();
	if (command==0){
		switch(HMS){
			case 0 :
				++hour;
				if (hour==24) hour = 0;
				break;
			case 1 :
				++min;
				if (min==60) min = 0;
				break;
			case 2 :
				++sec;
				if (sec==60) sec = 0;
				break;
		}
	}
	else {
		switch(HMS){
			case 0 :
				if (hour==0) hour = 24;
				--hour;
				break;
			case 1 :
				if (min==0) min = 60;
				--min;
				break;
			case 2 :
				if (sec==0) sec = 60;
				--sec;
				break;
		}
	}
}


void reset_input_count(void){
	no_input = 0;
}


void RTC_IRQHandler(void){
	if ((RTC->CRL & 1)!=0){
		/* time counts for each 4th clk count*/
		/* clock stops during CONFIG MODE */
		if (clk_count==4){
			clk_count = 0;
			if (!flash){
				if (++sec==60){
					sec = 0;
					if (++min==60){
						min = 0;
						if (++hour==24){
							hour = 0;
						}
					}
				}
			}
		}
		
		/* send hour info. to display */
		if (clk_count++==0 && flash)
			hour_d = 100; // do not show anything on display.
		else if (H24 != 0) 
			hour_d = hour;
		else 
			hour_d = hour%12;
		
		RTC->CRL &= ~1;
	}
}


void set_TIM1(void){
	/* enable TIM1 */
	/* TIM1 is used for configuring clock */
	/* APB2 : 72MHz */
	RCC->APB2ENR |= 1<<11; //enable TIM1
	
	TIM1->CR1 = 0x00;
	//TIM1->CR1 |= 1<<2; //only overflow generate DMA requests
	TIM1->CR2 = 0x00;
	// 5 = 72MHz / (14400)*(1000)
	TIM1->PSC = 0x383F;
	TIM1->ARR = 0x03e7;
	
	TIM1->DIER |= 1; // enable update interrupt
	NVIC->ISER[0] |= 1 << 25; // TIM1 update interrupt
}


void TIM1_UP_IRQHandler (void){
	if ((TIM1->SR & 0x0001) != 0){
		/* send hour info. to display */
		hour_d = hour;
		
		TIM1->SR &= ~(1<<0); // clear UIF
		
		/* check for no input period */
		++no_input;
		//if (no_input==5){
		if (no_input==30){
			restore_clk();
			exit_clk_config();
		}
	}
}

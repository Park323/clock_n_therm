#include <stm32f10x.h>
#include "prj.h"
#define RTC_BASE 0x40002800

/*
Our clock use Real-time clock(RTC)
It's independent timer.
*/

u8 CLKEN = 1;
u8 H24 = 1; // 1 if 24 mode else 0
u8 config_mode = 0;
u8 show = 1;

u8 hour=12, min=0, sec=0;


void switch_clk(void){
	if (CLKEN != 0) CLKEN = 0;
	else CLKEN = 1;
}

void switch_h24(void){
	if (H24 != 0) H24 = 0;
	else H24 = 1;
}

void enter_clk_config(void){
	// poll RTOFF
	while((RTC->CRL & 0x0020)==0){}
	// enter config mode
	RTC->CRL |= 1 << 4;
}


void exit_clk_config(void){
	// exit config mode
	RTC->CRL &= ~(1 << 4);
	// poll RTOFF
	while((RTC->CRL & 0x0020)==0){}
}


void updown_clock(u8 command){
	/* command */
	/* U : upcount */
	/* D : downcount */
	/* H : hour */
	/* M : min */
	/* S : second */
	/* N : None */
	/* bit	7		6		5		4		3		2		1		0  */
	/* 			N		N		DH	UH	DM	UM	DS	US */
	switch(command){
		case 1<<0 :
			++sec;
			if (sec==60) sec = 0;
			break;
		case 1<<1 :
			if (sec==0) sec = 60;
			--sec;
			break;
		case 1<<2 :
			++min;
			if (min==60) min = 0;
			break;
		case 1<<3 :
			if (min==0) min = 60;
			--min;
			break;
		case 1<<4 :
			++hour;
			if (hour==24) hour = 0;
			break;
		case 1<<5 :
			if (hour==0) hour = 24;
			--hour;
			break;
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
	
	switch_clk_config();
	
	//RTC and interrupt enable
	RCC->BDCR |= 1 << 15;
	NVIC->ISER[0] |= 1 << 3; //RTC global interrupt
}


void switch_clk_config(void){
	if (config_mode == 0){
		config_mode = 1;
		// sync 0.5 second (for flicker)
		enter_clk_config();
		RTC->CRH |= 1; //second interrupt enable
		RTC->PRLL = 0x12FF; // reload value (TR_CLK = RTCCLK / (PRL + 1))
		exit_clk_config();
	}
	else{
		config_mode = 0;
		// sync 1 second
		enter_clk_config();
		RTC->CRH |= 1; //second interrupt enable
		RTC->PRLL = 0x7FFF; // reload value (TR_CLK = RTCCLK / (PRL + 1))
		exit_clk_config();
	}
}


void RTC_IRQHandler(void){
	if ((RTC->CRL & 1)!=0){
		if (config_mode == 0){
			show=1;
			/* clock works */
			++sec;
			if (sec==60){
				sec = 0;
				++min;
				if (min==60){
					min = 0;
					++hour;
					if (hour==24){
						hour = 0;
					}
				}
			}
		}
		else{
			/* set hour, min, sec */
			if (show++ == 5) show = 0;
		}
		if (CLKEN != 0){
			if (H24 != 0){
				display_hhmmss(hour, min, sec, show);
			}
			else{
				display_hhmmss(hour%12, min, sec, show);
			}
		}
		else {
			display_hhmmss(0,0,0,0);
		}
		RTC->CRL &= ~1;
	}
}

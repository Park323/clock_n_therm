#include <stm32f10x.h>
#include "prj.h"
#define RTC_BASE 0x40002800

/*
Our clock use Real-time clock(RTC)
It's independent timer.
*/

u8 CLKEN = 1;
u8 H24 = 1; // 1 if 24 mode else 0
int HMS = 0; // H : 0, M : 1, S : 2
u8 CLK_CONFIG = 0;
u8 check = 0;

u8 flash_b = 0, hour_b=12, min_b=0, sec_b=0;
u8 flash = 0, hour_d=12, hour=12, min=0, sec=0;

u8 flash_count = 1;


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
	check = RTC->CRL & 0x0020;
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


void set_flash(void){
	flash = 1;
	
	enter_RTC_config();
	
	// sync 0.5 second (for flicker)
	RTC->CRH |= 1; //second interrupt enable
	RTC->PRLL = 0x12FF; // reload value (TR_CLK = RTCCLK / (PRL + 1))
	
	exit_RTC_config();
}

void set_stable(void){
	flash = 0;
	
	enter_RTC_config();
	
	// sync 1 second
	RTC->CRH |= 1; //second interrupt enable
	RTC->PRLL = 0x7FFF; // reload value (TR_CLK = RTCCLK / (PRL + 1))
	
	exit_RTC_config();
}


// Clock Configuaration
void enter_clk_config(void){
	CLK_CONFIG = 1;
	RCC->BDCR &= ~1;	// LSE OFF
	enter_RTC_config();
}

void exit_clk_config(void){
	exit_RTC_config();
	RCC->BDCR |= 1;	// LSE ON
	CLK_CONFIG = 0;
}

void backup_clk(void){
	CLK_CONFIG = 0;
	HMS = 0;
	flash = flash_b;
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
	if (direction == 0){
		if (++HMS == 3) HMS=0;
	}
	else {
		if (--HMS == -1) HMS=2;
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
	
	set_flash();
	
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
	hour_b = hour;
	min_b = min;
	sec_b = sec;
	
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



void RTC_IRQHandler(void){
	if ((RTC->CRL & 1)!=0){
		if (!flash){
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
		else if (++flash_count == 5) flash_count = 0;

		if (H24 != 0) hour_d = hour;
		else hour_d = hour%12;

		RTC->CRL &= ~1;
	}
}

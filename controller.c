#include <stm32f10x.h>
#include "prj.h"

/*
Keypad controls the display and operate mode
Make it do not wait for input, we may use EXTI... refer to chapter 8 - discussion 4
*/

extern u8 CLKEN;
extern u8 CLK_CONFIG;
extern u8 H24;
extern u32 temp_mode;
extern u8 no_input;

u8 debug = 0;
u8 key_index, pressed;
u32 tmp;
u32 key_row, key_col;


void enable_keypad(){
	/*
	col ::	pin :: port
		1			K0			PA8
		2			K1			PA9
		3			K2			PA10
		4 		K3			PA11
	row ::	pin :: port
		1			K4			PC8
		2			K5			PC9
		3			K6			PC10
		4			K7			PC11
	send HIGH to rows(PC) then push the button,
	corresponding col port(PA) will get LOW signal.*/
	
	// enable port A and C
	RCC->APB2ENR |= 0x00000014;
	// set port C push-pull output mode
	GPIOC->CRH |= 0x00003333;
	GPIOC->CRH &= 0xFFFF3333;
	// set port A pull-up/down input mode
	GPIOA->CRH |= 0x00008888;
	GPIOA->CRH &= 0xFFFF8888;
	// set pull-up mode (default HIGH)
	GPIOA->ODR |= 0x0F00;
	
	// external interrupt
	EXTI->FTSR = 0x0F00; //EXTI[11:8]
	EXTI->IMR = 0x0F00;
	AFIO->EXTICR[2] = 0x0000; //port A
	NVIC->ISER[0] = 1<<23; //EXTI_9_5
	NVIC->ISER[1] = 1<<8; //EXTI_15_10
}


void scan_button(u8 col_num){
	u8 i;
	for(i=0; i<4; i++){
		if (col_num >> i == 1) break;
	}
	key_index=i;
	for (key_row=0x01;key_row<0x10;key_row=key_row<<1){
		GPIOC->BSRR = (~(key_row << 8) & 0x0F00) | (key_row << 24);
		for (tmp=0; tmp<1000; tmp++) {}
		key_col = GPIOA->IDR;
		key_col = (key_col >> 8) & 0x0F;
		
		// key detected 
		if ((key_col & col_num ) == 0){
			pressed = key_index;
			switch(key_index){
				/*
					S0 : CLK/TMP mode 
				*/
				case 0:
					switch_clk();
					//if (CLKEN!=0) tmp2data_off();
					//else tmp2data();
					break;
				/* 
					S6 : set button
					enable clock setting
					confirm setting
				*/
				case 6:
					if (CLKEN){
						if (CLK_CONFIG)
							exit_clk_config();
						else 
							enter_clk_config();
					}
					break;
				/*
					S1 : back button
					exit config mode
					back to original setting
				*/
				case 1:
					if (CLKEN && CLK_CONFIG){
						backup_clk();
						exit_clk_config();
					}
					break;
				/*
					S2 : UP
					increase +
				*/
				case 2:
					if (CLKEN && CLK_CONFIG)
						updown_clock(0);
					break;
				/*
					S10 : DOWN
					decrease -
				*/
				case 10:
					if (CLKEN && CLK_CONFIG)
						updown_clock(1);
					break;
				/*
					S5 : LEFT
				*/
				case 5:
					if (CLKEN && CLK_CONFIG) 
						switch_config_unit(1);
					break;
				/*
					S7 : RIGHT
				*/
				case 7:
					if (CLKEN && CLK_CONFIG) 
						switch_config_unit(0);
					break;
				//switch 12/24 or C/F
				case 8:
					if (CLKEN)
						switch_h24();
					//else{
					//	if (temp_mode != 0) temp_mode = 0;
					//	else temp_mode = 1;
					//}
					break;
				case 12:
					switch_scrolling(0);
					break;
				case 13:
					switch_scrolling(1);
					break;
				case 14:
					switch_scrolling(2);
					break;
				case 15:
					switch_scrolling(3);
					break;
				case 4:
					//Reserved
					break;
			}
			break;
		}
		key_index = key_index + 4;
	}
}


void EXTI9_5_IRQHandler(void){
	if ((EXTI->PR & 0x0100)!=0)	{
		scan_button(1<<0);
		EXTI->PR |= 1<<8;
	}
	if ((EXTI->PR & 0x0200)!=0)	{
		scan_button(1<<1);
		EXTI->PR |= 1<<9;
	}
}

void EXTI15_10_IRQHandler(void){
	if ((EXTI->PR & 0x0400)!=0)	{
		scan_button(1<<2);
		EXTI->PR |= 1<<10;
	}
	if ((EXTI->PR & 0x0800)!=0)	{
		scan_button(1<<3);
		EXTI->PR |= 1<<11;
	}
}

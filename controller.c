#include <stm32f10x.h>
#include "prj_tx.h"

/*
Keypad controls the display and operate mode
Make it do not wait for input, we may use EXTI... refer to chapter 8 - discussion 4
*/

extern u8 CLKEN;
extern u8 CLK_CONFIG;
extern u8 H24;
extern u32 temp_mode;
extern u8 no_input;
extern u8 flash;

volatile u8 debug = 0;
u8 key_index, pressed;
u8 scroll_mode = 0;
u32 key_row, key_col;
volatile u32 delay = 0;


void enable_EXTI(void){
	// external interrupt
	EXTI->FTSR = 0x0F00; //EXTI[11:8]
	EXTI->IMR = 0x0F00;
	AFIO->EXTICR[2] = 0x0000; //port A
	NVIC->ISER[0] = 1<<23; //EXTI_9_5
	NVIC->ISER[1] = 1<<8; //EXTI_15_10
}

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
	GPIOC->ODR |= 0x0F00;
	// set port A pull-up/down input mode
	GPIOA->CRH |= 0x00008888;
	GPIOA->CRH &= 0xFFFF8888;
	// set pull-up mode (default HIGH)
	GPIOA->ODR |= 0x0F00;
	//enable_EXTI();
}


void sleep(void){
	//activate clock
	TIM2->CR1 |= 0x0001;
	
	// wait for 0.6 seconds
	while(delay<3){	}
	
	//deactivate clock
	TIM2->CR1 &= ~0x0001;
	delay = 0;
}

void scan_button(u8 col_num){
	key_index=col_num;
	for (key_row=0x01;key_row<0x10;key_row=key_row<<1){
		GPIOC->BSRR = (~(key_row << 8) & 0x0F00) | (key_row << 24);
		for (u16 d=0; d<1000; d++) {}
		key_col = GPIOA->IDR;
		key_col = (key_col >> 8) & 0x0F;
		
		// key detected 
		if ((key_col & (1<<col_num) ) == 0){
			pressed = key_index;
			switch(key_index){
				/*
					S0 : CLK/TMP mode 
				*/
				case 0:
					switch_clk();
					break;
				/* 
					S6 : set button
					enable clock setting
					confirm setting
				*/
				case 6:
					if (CLKEN){
						if (CLK_CONFIG!=0){
							exit_clk_config();
							flash = 0;
						}
						else {
							backup_clk();
							enter_clk_config();
						}
					}
					break;
				/*
					S1 : back button
					exit config mode
					back to original setting
				*/
				case 1:
					if (CLKEN && CLK_CONFIG){
						restore_clk();
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
					else {
						if (temp_mode != 0) temp_mode = 0;
						else temp_mode = 1;
					}
					break;
				case 12:
					scroll_mode = 0;
					break;
				case 13:
					scroll_mode = 1;
					break;
				case 14:
					scroll_mode = 2;
					break;
				case 15:
					scroll_mode = 3;
					break;
				case 4:
					//Reserved
					break;
			}
			sleep();
			debug++;
			break;
		}
		key_index = key_index + 4;
	}
	GPIOC->ODR |= 0x0F00;
}


void EXTI9_5_IRQHandler(void){
	if ((EXTI->PR & 1<<8)!=0)	{
		scan_button(0);
		EXTI->PR |= 1<<8;
	}
	if ((EXTI->PR & 1<<9)!=0)	{
		scan_button(1);
		EXTI->PR |= 1<<9;
	}
}

void EXTI15_10_IRQHandler(void){
	if ((EXTI->PR & 1<<10) != 0){
		scan_button(2);
		EXTI->PR |= 1<<10;
	}
	if ((EXTI->PR & 1<<11) != 0){
		scan_button(3);
		EXTI->PR |= 1<<11;
	}
}

void enable_TIM2(void){
	/* enable TIM2 */
	/* TIM2 is used for printing key matrix */
	RCC->APB1ENR |= 1; //enable TIM2
	
	TIM2->CR1 = 0x00;
	TIM2->CR1 |= 1<<2; //only overflow generate DMA requests
	TIM2->CR2 = 0x00;
	// 5 = 36MHz / (7200)*(1000)
	// 1 = 36MHz / (7200)*(5000)
	// 0.1 = 36MHz / (7200)*(50000)
	TIM2->PSC = 0x1c1f;
	TIM2->ARR = 0x1387;
	
	TIM2->DIER |= 1; // enable update interrupt
	NVIC->ISER[0] |= 1 << 28; // TIM2 global interrupt
}

void TIM2_IRQHandler (void){
	if ((TIM2->SR & 0x0001) != 0){
		delay++;
		TIM2->SR &= ~(1<<0); // clear UIF
	}
}
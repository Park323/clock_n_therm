#include <stm32f10x.h>
#include "prj.h"

/*
Keypad controls the display and operate mode
Make it do not wait for input, we may use EXTI... refer to chapter 8 - discussion 4
*/

extern clk_tmp;
extern h24_mode;
extern scroll_mode;
extern temp_mode;

u8 key_index;
u32 row, col, i_c, j_c;
u32 key_row, key_col, col_scan;


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
	send HIGH to col then push the button,
	corresponding row port will get LOW signal.*/
	
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
	
	enable_TIM3(); //use timer for keypad input
}


void enable_TIM3(){
	/* enable TIM3 */
	/* TIM3 is used for checking keypad input */
	RCC->APB1ENR |= 1 << 1; //enable TIM3
	
	TIM3->CR1 = 0x00;
	TIM3->CR2 = 0x00;
	TIM3->PSC = 0x10;	//match timer with TIM1
	TIM3->ARR = 0x2000;
	
	TIM3->DIER |= 1;
	NVIC->ISER[0] |= 1 <<29;
	
	//activate clock
	TIM3->CR1 |= 0x0001;
}


void TIM3_IRQHandler (void){
	// check Update interrupt pending
	if ((TIM3->SR & 0x0001) != 0){	
		key_index=0;
		for (key_row=0x01;key_row<0x10;key_row=key_row<<1){
			GPIOC->BSRR = (~(key_row << 8) & 0x0F00) | (key_row << 24);
			for (j_c=0; j_c<1000; j_c++) {}
			key_col = GPIOA->IDR;
			key_col = (key_col >> 8) & 0x0F;
			col_scan = 0x01;
			for (j_c=0; j_c<4; j_c++){
				if ((key_col & col_scan ) == 0){
					/* if key detected */
					switch(key_index){
						case 0:
							/* switch clock & temperature mode */
							if (clk_tmp!=0){
								clk_tmp = 0;
								tmp2data();
							}
							else{
								tmp2data_off();
								clk_tmp = 1;
							}
							break;
						case 8:
							/* switch 12/24 or C/F */
							if (clk_tmp!=0){
								if (h24_mode != 0) h24_mode = 0;
								else h24_mode = 1;
							}
							else{
								if (temp_mode != 0) temp_mode = 0;
								else temp_mode = 1;
							}
							break;
						case 1:
							/* Hour ++ */
							updown_clock(1<<4);
							break;
						case 3:
							/* Hour -- */
							updown_clock(1<<5);
							break;
						case 5:
							/* Minute ++ */
							updown_clock(1<<2);
							break;
						case 7:
							/* Minute -- */
							updown_clock(1<<3);
							break;
						case 9:
							/* Second ++ */
							updown_clock(1<<0);
							break;
						case 11:
							/* Second -- */
							updown_clock(1<<1);
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
						case 2:
							/* Reserved */
							break;
						case 4:
							/* Reserved */
							break;
						case 6:
							/* Reserved */
							break;
						case 10:
							/* Reserved */
							break;
					}
				}
				col_scan = col_scan << 1;
				key_index = key_index + 1;
			}
		}
	}
}

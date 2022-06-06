#include <stm32f10x.h>
#include "prj.h"

/*
Keypad controls the display and operate mode
Make it do not wait for input, we may use EXTI... refer to chapter 8 - discussion 4
*/

u8 key_index;
u32 row, col, i, j;
u32 k=0;
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
			for (j=0; j<1000; j++) {}
			key_col = GPIOA->IDR;
			key_col = (key_col >> 8) & 0x0F;
			col_scan = 0x01;
			for (j=0; j<4; j++){
				if ((key_col & col_scan ) == 0){
					cur_data = key_index;
				}
				if(cur_data == 0) {
					if (k==0) {
						tmp2data ();
						clk2data_off();
						k=1;
					}
					if (k==1) {
						clk2data ();
						tmp2data_off();
						k=0;
					}
				}
				col_scan = col_scan << 1;
				key_index = key_index + 1;
			}
		}
	}
}

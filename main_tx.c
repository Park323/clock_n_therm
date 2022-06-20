#include <stm32f10x.h>
#include "prj_tx.h"

/*
USART Transmitter
connect with keypad and TMP sensor
enable clock signal

send these values to Rx module
- scroll mode
- hour, min, sec
- temperature, temp_mode
*/

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
	
	/* Loop waiting for interrupt */
	while (1) {
		for (u8 i=0; i<4; i++)
			scan_button(i);
	}
	//while(1){ __WFI(); }
}

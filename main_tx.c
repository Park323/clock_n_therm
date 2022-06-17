#include <stm32f10x.h>
#include "prj.h"

/*
USART Transmitter
connect with keypad and TMP sensor
enable clock signal

send these values to Rx module
- button number
- hour, min, sec
- temperature
*/

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	enable_keypad();
	enable_clk();
	//enable_TMP();
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

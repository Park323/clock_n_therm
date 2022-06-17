#include <stm32f10x.h>
#include "prj.h"

/*
USART Receiver
connect with dot matrix

receive these values from Tx module
- scroll mode
- hour, min, sec
- temperature, temp_mode

then display the information on dot matrix
*/

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	enable_dot_matrix();
	
	activate_display();
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

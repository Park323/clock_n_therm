#include <stm32f10x.h>
#include "prj.h"

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	//Activte Dot matrix & Keypad, we follow connection of Chapter9 - step9
	enable_dot_matrix();
	enable_key_matrix();
	
	SET_TIM();
	SET_DMA();
	SET_EXTI();
	SET_USART();
	
	/* Enable IO or Interrupt or ..etc */
	// TIM1->CR1 |= 0x0001;
	
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

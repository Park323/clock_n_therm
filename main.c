#include <stm32f10x.h>
#include "prj.h"

int main(){
	//Mr.kim fighting!!
	SET_GPIO();
	SET_TIM();
	SET_DMA();
	SET_EXTI();
	SET_USART();
	
	/* Enable IO or Interrupt or ..etc */
	// TIM1->CR1 |= 0x0001;
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}
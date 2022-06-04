#include <stm32f10x.h>
#include "prj.h"

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	//Activte Dot matrix & Keypad, we follow connection of Chapter9 - step9
	enable_dot_matrix();
	enable_keypad();
	enable_TMP();
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

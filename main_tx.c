#include <stm32f10x.h>
#include "prj.h"

u8 current_data = 0;
u32 matrix_col = 0x40000000; // This address should be revised.

int main(){
	//Mr.kim fighting!!
	//Mr.park fighting!!
	//yeah~
	
	//Activte Dot matrix & Keypad, we follow connection of Chapter9 - step9
	enable_dot_matrix();
	enable_keypad();
	enable_TMP();
	
	print_font();
	
	/* Loop waiting for interrupt */
	while(1){ __WFI(); }
}

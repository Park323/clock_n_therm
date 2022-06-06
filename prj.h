#include <stm32f10x.h>
#ifndef DEF_HEADER    // include guard
#define DEF_HEADER    // prevents the file from being included twice.
#define MATRIX_COL		0x4001100C
#define ADC1_DR_BASE 	0x4001244C


/* Functions */
// initialization
void enable_dot_matrix(void);
void enable_keypad(void);
void enable_TMP(void);
void enable_clk(void);

void enable_TIM1(void);
void enable_TIM2(void);
void enable_TIM3(void);

// show
void activate_display(void);
void display_hhmmss(u8 hh, u8 mm, u8 ss);

// control
void tmp2data(void);
void tmp2data_off(void);
void switch_clk_config(void);
void updown_clock(u8 command);

#endif

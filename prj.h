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
void display_hhmmss(u8 hh, u8 mm, u8 ss, u8 show);
void display_mnC(u8 m, u8 n, u32 temp_mode);
void horizon2vertical(void);
void switch_scrolling(u8 index);

// clock control
void switch_clk(void);
void switch_h24(void);
void switch_config_unit(u8 dir);
void enter_clk_config(void);
void exit_clk_config(void);
void backup_clk(void);
void updown_clock(u8 command);

// control
void scan_button(u8 col_num);
void tmp2data(void);
void tmp2data_off(void);

#endif

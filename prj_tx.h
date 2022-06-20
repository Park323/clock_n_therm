#include <stm32f10x.h>
#ifndef DEF_HEADER    // include guard
#define DEF_HEADER    // prevents the file from being included twice.
#define MATRIX_COL		0x4001100C
#define ADC1_DR_BASE 	0x4001244C


/* Functions */
// initialization
void enable_keypad(void);
void enable_TMP(void);
void enable_clk(void);

void set_TIM1(void);
void enable_TIM2(void);
void enable_TIM3(void);

void enable_Tx(void);

// show
void activate_display(void);
void horizon2vertical(void);

// clock control
void switch_clk(void);
void switch_h24(void);
void switch_config_unit(u8 dir);
void enter_clk_config(void);
void exit_clk_config(void);
void backup_clk(void);
void restore_clk(void);
void updown_clock(u8 command);
void reset_input_count(void);

// control
void scan_button(u8 col_num);
void tmp2data(void);
void tmp2data_off(void);

#endif

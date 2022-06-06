#ifndef DEF_HEADER    // include guard
#define DEF_HEADER    // prevents the file from being included twice.
#define MATRIX_COL		0x4001100C
#define ADC1_DR_BASE 	0x4001244C

u8 cur_data;
u16 cur_TMP;
u8 font8x8[16][8];
uint64_t rawdata[8] = {0x3c103c3c087e, 0x423042421840, 0x46500202287c, 0x4a100c1c4802, 0x52103002fe02, 0x621042420842, 0x3c7c7e3c1c3c, 0x000000000000 }; // Data information to display. 

void enable_dot_matrix(void);

void enable_keypad(void);

void enable_TMP(void);

void enable_TIM1(void);

void enable_TIM2(void);

void enable_TIM3(void);

void print_font();

void tmp2data();

void tmp2data_off();

void clk2data ();

void clk2data_off ();

#endif
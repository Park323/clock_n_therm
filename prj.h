#ifndef DEF_HEADER    // include guard
#define DEF_HEADER    // prevents the file from being included twice.
#define MATRIX_COL		0x4001100C
#define ADC1_DR_BASE 	0x4001244C

u8 cur_data;
u16 cur_TMP;

void enable_dot_matrix(void);

void enable_keypad(void);

void enable_TMP(void);

void enable_TIM1(void);

void enable_TIM2(void);

void enable_TIM3(void);

void print_font();

#endif
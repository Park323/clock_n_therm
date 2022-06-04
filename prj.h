#ifndef DEF_HEADER    // include guard
#define DEF_HEADER    // prevents the file from being included twice.

void enable_dot_matrix(void);

void enable_keypad(void);

void enable_TIM1(void);

void enable_TIM2(void);

void enable_TIM3(void);

void print_font(u32 font, u32 matrix_col);

#endif
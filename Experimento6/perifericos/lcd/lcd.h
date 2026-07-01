#ifndef LCD_H
#define LCD_H

#include <stdbool.h>

bool lcd_init(void);

void lcd_clear(void);

void lcd_setCursor(int row, int col);

void lcd_print(const char *text);

void lcd_printChar(char c);

void lcd_printInt(int value);

void lcd_close(void);

#endif
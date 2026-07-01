#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>

bool keypad_init(void);

char keypad_getKey(void);

void keypad_close(void);

#endif
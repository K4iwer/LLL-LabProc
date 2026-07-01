#include "keypad.h"

#include <wiringPi.h>

#define NUM_ROWS 4
#define NUM_COLS 4

/* Numeração WiringPi */
static const int rows[NUM_ROWS] =
{
    0, 2, 3, 4
};

static const int cols[NUM_COLS] =
{
    5, 6, 21, 22
};

static const char keys[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

bool keypad_init(void)
{
    if (wiringPiSetup() == -1)
        return false;

    for (int i = 0; i < NUM_ROWS; i++)
    {
        pinMode(rows[i], OUTPUT);
        digitalWrite(rows[i], LOW);
    }

    for (int i = 0; i < NUM_COLS; i++)
    {
        pinMode(cols[i], INPUT);
        pullUpDnControl(cols[i], PUD_DOWN);
    }

    return true;
}

char keypad_getKey(void)
{
    while (1)
    {
        for (int r = 0; r < NUM_ROWS; r++)
        {
            for (int i = 0; i < NUM_ROWS; i++)
                digitalWrite(rows[i], LOW);

            digitalWrite(rows[r], HIGH);

            delay(1);

            for (int c = 0; c < NUM_COLS; c++)
            {
                if (digitalRead(cols[c]))
                {
                    while (digitalRead(cols[c]));

                    delay(20);

                    return keys[r][c];
                }
            }
        }
    }
}

void keypad_close(void)
{
}
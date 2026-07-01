#include "keypad.h"

#include <gpiod.h>
#include <unistd.h>

#define CHIPNAME "/dev/gpiochip0"

#define NUM_ROWS 4
#define NUM_COLS 4

/* GPIOs de exemplo.
   Altere para os pinos que você ligar. */

static const unsigned int rows[NUM_ROWS] =
{
    17,27,22,23
};

static const unsigned int cols[NUM_COLS] =
{
    24,25,5,6
};

static const char keypad[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

static struct gpiod_chip *chip;

static struct gpiod_line *rowLines[NUM_ROWS];
static struct gpiod_line *colLines[NUM_COLS];

bool keypad_init(void)
{
    chip = gpiod_chip_open(CHIPNAME);

    if(chip == NULL)
        return false;

    for(int i=0;i<NUM_ROWS;i++)
    {
        rowLines[i] = gpiod_chip_get_line(chip, rows[i]);

        if(gpiod_line_request_output(rowLines[i],"keypad",0) < 0)
            return false;
    }

    for(int i=0;i<NUM_COLS;i++)
    {
        colLines[i] = gpiod_chip_get_line(chip, cols[i]);

        if(gpiod_line_request_input(colLines[i],"keypad") < 0)
            return false;
    }

    return true;
}

char keypad_getKey(void)
{
    while(1)
    {
        for(int r=0;r<NUM_ROWS;r++)
        {
            /* Desliga todas as linhas */

            for(int i=0;i<NUM_ROWS;i++)
                gpiod_line_set_value(rowLines[i],0);

            /* Liga somente uma */

            gpiod_line_set_value(rowLines[r],1);

            usleep(1000);

            for(int c=0;c<NUM_COLS;c++)
            {
                if(gpiod_line_get_value(colLines[c]))
                {
                    while(gpiod_line_get_value(colLines[c]));

                    usleep(20000);

                    return keypad[r][c];
                }
            }
        }
    }
}

void keypad_close(void)
{
    for(int i=0;i<NUM_ROWS;i++)
        gpiod_line_release(rowLines[i]);

    for(int i=0;i<NUM_COLS;i++)
        gpiod_line_release(colLines[i]);

    gpiod_chip_close(chip);
}
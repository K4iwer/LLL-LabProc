#include <stdio.h>

#include "keypad.h"

int main(void)
{
    if (!keypad_init())
    {
        printf("Erro ao iniciar teclado.\n");
        return 1;
    }

    printf("Pressione teclas.\n");

    while (1)
    {
        char tecla = keypad_getKey();

        printf("Tecla: %c\n", tecla);
    }

    keypad_close();

    return 0;
}
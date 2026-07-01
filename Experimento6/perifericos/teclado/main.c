#include <stdio.h>
#include "keypad.h"

int main(void)
{
    if (!keypad_init())
    {
        printf("Erro ao inicializar o teclado!\n");
        return 1;
    }

    printf("Teclado inicializado.\n");
    printf("Pressione teclas (Ctrl+C para sair).\n");

    while (1)
    {
        char tecla = keypad_getKey();

        printf("Tecla pressionada: %c\n", tecla);
        fflush(stdout);
    }

    keypad_close();

    return 0;
}
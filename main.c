#include <stdio.h>
#include "menu.h"

int main()
{
    ncurses_init();
    menu_do_routine();
	ncurses_destroy();

    return 0;
}

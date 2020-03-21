#include <stdio.h>
#include "menu.h"

int main()
{
    ncurses_init();
    menu_do();
	endwin();
    return 0;
}

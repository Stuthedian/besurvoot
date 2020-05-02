#include <stdio.h>
#include "menu.h"
#include "baht.h"

int main()
{
  baht_init();
  baht_catch_sigsegv();
  baht_catch_sigabort();

  ncurses_init();
  menu_do_routine();
  ncurses_destroy();

  return 0;
}

#include <stdio.h>
#include "menu.h"
#include "baht.h"

int main()
{
  baht_init();
  baht_catch_signal(SIGABRT);
  baht_catch_signal(SIGSEGV);

  ncurses_init();
  menu_do_routine();
  ncurses_destroy();

  return 0;
}

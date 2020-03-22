#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include "menu.h"

Menu_t main_menu;

void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
    resizeterm(size.ws_row, size.ws_col);
    menu_destroy(&main_menu);
    refresh();
    menu_init(&main_menu);
}

void ncurses_init()
{
  initscr() CHECK_IS_NULL;
  cbreak() CHECK_ERR;
  noecho() CHECK_ERR;

  signal(SIGWINCH, sig_winch) CHECK(==, SIG_ERR);

  curs_set(FALSE) CHECK_ERR;

  start_color() CHECK_ERR;
  init_pair(1, COLOR_YELLOW, COLOR_BLUE) CHECK_ERR;
  init_pair(2, COLOR_YELLOW, COLOR_CYAN) CHECK_ERR;
  init_pair(3, COLOR_WHITE, COLOR_BLACK) CHECK_ERR;
  bkgd(COLOR_PAIR(2)) CHECK_ERR;

  nonl();//Always 'OK'
  intrflush(stdscr, FALSE) CHECK_ERR;
  keypad(stdscr, TRUE) CHECK_ERR;

  nodelay(stdscr, 1) CHECK_ERR;

  refresh() CHECK_ERR;
}

void ncurses_destroy()
{
    endwin() CHECK(==, ERR);
}

void menu_init(Menu_t *menu)
{
  const int menu_box_offset = BOX_OFFSET;
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
  menu->env.terminal_width = size.ws_col;
  menu->env.terminal_height = size.ws_row;
  menu->env.max_items_on_screen = menu->env.terminal_height - menu_box_offset;

  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  menu->menu_wnd = newwin(menu->env.terminal_height, menu->env.terminal_width, menu_ncurses_y,
                          menu_ncurses_x);
  menu->items = malloc(menu->env.max_items_on_screen * sizeof(WINDOW));

  int menu_item_width = menu->env.terminal_width - menu_box_offset;

  strcpy(menu->text[0], "show firmware");
  strcpy(menu->text[1], "reload system");
  strcpy(menu->text[2], "show running config");
  strcpy(menu->text[3], "show msdp vrf test peers");
  strcpy(menu->text[4], "Exit");

  menu->screen_idx = 0;
  menu->current_idx = 0;
  menu->top_of_text_array = 0;
  for (int i = 0; i < menu->env.max_items_on_screen; i++) {
      menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
              i + menu_box_offset / 2,
              0 + menu_box_offset / 2);
      if(i < NUM_MENU_ITEMS)
          wprintw(menu->items[i], menu->text[i]);
  }

  wbkgd(menu->menu_wnd, COLOR_PAIR(1));
  wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE);

  box(menu->menu_wnd, '|', '-');
  wrefresh(menu->menu_wnd);
}

void menu_destroy(Menu_t *menu)
{
  for (int i = 0; i < menu->env.max_items_on_screen; i++) {
    delwin(menu->items[i]);
  }
  free(menu->items);

  delwin(menu->menu_wnd);
}

void menu_go_down(Menu_t *menu)
{
    if (menu->current_idx + 1 < NUM_MENU_ITEMS) {
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1));
        wrefresh(menu->items[menu->screen_idx]);
        menu->screen_idx++;
        menu->current_idx++;
        if(menu->screen_idx >= menu->env.max_items_on_screen)
        {
            menu->top_of_text_array++;
            for (int i = 0, j = menu->top_of_text_array; i < menu->env.max_items_on_screen && i < NUM_MENU_ITEMS; i++, j++) {
                wclear(menu->items[i]);
                wprintw(menu->items[i], menu->text[j]);
                wrefresh(menu->items[i]);
            }
            menu->screen_idx--;
        }
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE);
        wrefresh(menu->items[menu->screen_idx]);
    }
}

void menu_go_up(Menu_t *menu)
{
  if (menu->current_idx - 1 >= 0) {
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1));
    wrefresh(menu->items[menu->screen_idx]);
        menu->screen_idx--;
        menu->current_idx--;
        if(menu->screen_idx < 0)
        {
            menu->top_of_text_array--;
            for (int i = 0, j = menu->top_of_text_array; i < menu->env.max_items_on_screen && i < NUM_MENU_ITEMS; i++, j++) {
                wclear(menu->items[i]);
                wprintw(menu->items[i], menu->text[j]);
                wrefresh(menu->items[i]);
            }
            menu->screen_idx++;
        }
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE);
    wrefresh(menu->items[menu->screen_idx]);
  }
}

void menu_move(Menu_t *menu)
{
  while (1) 
  {
      int ch = getch();

      switch (ch) 
      {
          case 'j': case KEY_DOWN:
              menu_go_down(menu); break;
          case 'k': case KEY_UP:
              menu_go_up(menu); break;
          case '\r':
              menu_act_on_item(menu); break;
          case 'q': return;
          case ERR: default: break;
      }
  }
}

void menu_act_on_item(Menu_t *menu)
{
  char* result_command = NULL;
  char* prefix = "tmux send-keys -t ! \"";
  char* command = menu->text[menu->current_idx];
  char* suffix = "\" Enter";

  result_command = malloc(strlen(prefix) + strlen(command) + strlen(suffix) + 1);
  result_command[0] = '\0';

  strcat(result_command, prefix);
  strcat(result_command, command);
  strcat(result_command, suffix);

  system(result_command);

  free(result_command);
}

void menu_do_routine()
{
  //Menu_t main_menu;

  menu_init(&main_menu);
  menu_move(&main_menu);
  menu_destroy(&main_menu);
}


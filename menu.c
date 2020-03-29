#define _POSIX_C_SOURCE 1
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#undef _POSIX_C_SOURCE
#include "menu.h"

Menu_t main_menu;

void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size) CHECK_IS_NEGATIVE_ONE;
    resizeterm(size.ws_row, size.ws_col) CHECK_ERR;
    menu_destroy(&main_menu);
    refresh() CHECK_ERR;
    //pane enlarged
    if(main_menu.env.terminal_height < size.ws_row)
    {
  const int menu_box_offset = BOX_OFFSET;
  int terminal_height_change = size.ws_row - main_menu.env.terminal_height;
  main_menu.env.terminal_width = size.ws_col;
  main_menu.env.terminal_height = size.ws_row;
  int last =main_menu.env.max_items_on_screen; 
  main_menu.env.max_items_on_screen = main_menu.env.terminal_height - menu_box_offset;

  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  main_menu.menu_wnd = newwin(main_menu.env.terminal_height, main_menu.env.terminal_width, menu_ncurses_y,
                          menu_ncurses_x);
  main_menu.menu_wnd CHECK_IS_NULL;
  main_menu.items = malloc(main_menu.env.max_items_on_screen * sizeof(WINDOW));
  main_menu.items CHECK_IS_NULL;

  int menu_item_width = main_menu.env.terminal_width - menu_box_offset;

  int to_show = NUM_MENU_ITEMS - 1 - main_menu.top_of_text_array + 1;
  if(main_menu.env.max_items_on_screen > to_show)
  //if(terminal_height_change + main_menu.top_of_text_array > NUM_MENU_ITEMS -1)
  {
  main_menu.top_of_text_array -= terminal_height_change;
  if(main_menu.top_of_text_array < 0)
      main_menu.top_of_text_array = 0;
  else
      main_menu.screen_idx += terminal_height_change;
  //if(main_menu.screen_idx >= main_menu.env.max_items_on_screen)
      //main_menu.top_of_text_array = main_menu.env.max_items_on_screen;
  }
  for (int i = 0, j = main_menu.top_of_text_array; i < main_menu.env.max_items_on_screen && j < NUM_MENU_ITEMS; i++, j++) {
      main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
              i + menu_box_offset / 2,
              0 + menu_box_offset / 2);
      main_menu.items[i] CHECK_IS_NULL;
      wprintw(main_menu.items[i], main_menu.text[j]) CHECK_ERR;
  }

  wbkgd(main_menu.menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
  wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

  box(main_menu.menu_wnd, '|', '-') CHECK_ERR;
  wrefresh(main_menu.menu_wnd) CHECK_ERR;
    }
    //pane shrank
    else
    {
  const int menu_box_offset = BOX_OFFSET;
  main_menu.env.terminal_width = size.ws_col;
  int terminal_height_change = main_menu.env.terminal_height - size.ws_row;
  main_menu.env.terminal_height = size.ws_row;
  main_menu.env.max_items_on_screen = main_menu.env.terminal_height - menu_box_offset;

  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  main_menu.menu_wnd = newwin(main_menu.env.terminal_height, main_menu.env.terminal_width, menu_ncurses_y,
                          menu_ncurses_x);
  main_menu.menu_wnd CHECK_IS_NULL;
  main_menu.items = malloc(main_menu.env.max_items_on_screen * sizeof(WINDOW));
  main_menu.items CHECK_IS_NULL;

  int menu_item_width = main_menu.env.terminal_width - menu_box_offset;

  if(main_menu.screen_idx >= main_menu.env.max_items_on_screen)
  {
      main_menu.screen_idx -= terminal_height_change;
      main_menu.top_of_text_array += terminal_height_change;
  }
  for (int i = 0, j = main_menu.top_of_text_array; i < main_menu.env.max_items_on_screen && j < NUM_MENU_ITEMS; i++, j++) {
      main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
              i + menu_box_offset / 2,
              0 + menu_box_offset / 2);
      main_menu.items[i] CHECK_IS_NULL;
      wprintw(main_menu.items[i], main_menu.text[j]) CHECK_ERR;
  }

  wbkgd(main_menu.menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
  wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

  box(main_menu.menu_wnd, '|', '-') CHECK_ERR;
  wrefresh(main_menu.menu_wnd) CHECK_ERR;
    }
}

void ncurses_init()
{
  const struct sigaction sa_handler =   
  {
      .sa_handler = sig_winch
  };

  initscr() CHECK_IS_NULL;
  cbreak() CHECK_ERR;
  noecho() CHECK_ERR;

  sigaction(SIGWINCH, &sa_handler, NULL) CHECK_IS_NEGATIVE_ONE;

  curs_set(FALSE) CHECK_ERR;

  start_color() CHECK_ERR;
  init_pair(1, COLOR_YELLOW, COLOR_BLUE) CHECK_ERR;
  init_pair(2, COLOR_YELLOW, COLOR_CYAN) CHECK_ERR;
  init_pair(3, COLOR_WHITE, COLOR_BLACK) CHECK_ERR;
  bkgd(COLOR_PAIR(2)) CHECK_ERR;

  nonl() CHECK_ERR;
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
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size) CHECK_IS_NEGATIVE_ONE;
  menu->env.terminal_width = size.ws_col;
  menu->env.terminal_height = size.ws_row;
  menu->env.max_items_on_screen = menu->env.terminal_height - menu_box_offset;

  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  menu->menu_wnd = newwin(menu->env.terminal_height, menu->env.terminal_width, menu_ncurses_y,
                          menu_ncurses_x);
  menu->menu_wnd CHECK_IS_NULL;
  menu->items = malloc(menu->env.max_items_on_screen * sizeof(WINDOW));
  menu->items CHECK_IS_NULL;

  int menu_item_width = menu->env.terminal_width - menu_box_offset;

  strcpy(menu->text[0], "show firmware");
  strcpy(menu->text[1], "reload system");
  strcpy(menu->text[2], "show running config");
  strcpy(menu->text[3], "show msdp vrf test peers");
  strcpy(menu->text[4], "Exit");

  menu->screen_idx = 0;
  menu->current_idx = 0;
  menu->top_of_text_array = 0;
  for (int i = 0; i < menu->env.max_items_on_screen && i < NUM_MENU_ITEMS; i++) {
      menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
              i + menu_box_offset / 2,
              0 + menu_box_offset / 2);
      menu->items[i] CHECK_IS_NULL;
      wprintw(menu->items[i], menu->text[i]) CHECK_ERR;
  }

  wbkgd(menu->menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
  wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

  box(menu->menu_wnd, '|', '-') CHECK_ERR;
  wrefresh(menu->menu_wnd) CHECK_ERR;
}

void menu_destroy(Menu_t *menu)
{
  for (int i = 0, j = menu->top_of_text_array; i < menu->env.max_items_on_screen && j < NUM_MENU_ITEMS; i++, j++) {
    delwin(menu->items[i]) CHECK_ERR;
  }
  free(menu->items);

  delwin(menu->menu_wnd) CHECK_ERR;
}

void menu_go_down(Menu_t *menu)
{
    if (menu->current_idx + 1 < NUM_MENU_ITEMS) {
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1)) CHECK_ERR;
        wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
        menu->screen_idx++;
        menu->current_idx++;
        if(menu->screen_idx >= menu->env.max_items_on_screen)
        {
            menu->top_of_text_array++;
            for (int i = 0, j = menu->top_of_text_array; i < menu->env.max_items_on_screen && j < NUM_MENU_ITEMS; i++, j++) {
                wclear(menu->items[i]) CHECK_ERR;
                wprintw(menu->items[i], menu->text[j]) CHECK_ERR;
                wrefresh(menu->items[i]) CHECK_ERR;
            }
            menu->screen_idx--;
        }
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
        wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
    }
}

void menu_go_up(Menu_t *menu)
{
  if (menu->current_idx - 1 >= 0) {
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1)) CHECK_ERR;
    wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
        menu->screen_idx--;
        menu->current_idx--;
        if(menu->screen_idx < 0)
        {
            menu->top_of_text_array--;
            for (int i = 0, j = menu->top_of_text_array; i < menu->env.max_items_on_screen && j < NUM_MENU_ITEMS; i++, j++) {
                wclear(menu->items[i]) CHECK_ERR;
                wprintw(menu->items[i], menu->text[j]) CHECK_ERR;
                wrefresh(menu->items[i]) CHECK_ERR;
            }
            menu->screen_idx++;
        }
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
    wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
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
  result_command CHECK_IS_NULL;
  result_command[0] = '\0';

  strcat(result_command, prefix);
  strcat(result_command, command);
  strcat(result_command, suffix);

  system(result_command) CHECK_IS_NEGATIVE_ONE;

  free(result_command);
}

void menu_do_routine()
{
  menu_init(&main_menu);
  menu_move(&main_menu);
  menu_destroy(&main_menu);
}

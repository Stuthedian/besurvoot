#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>

#define TERMINAL_WIDTH 80
#define BOX_OFFSET 2
#define NUM_MENU_ITEMS 5

typedef struct Env
{
    int terminal_width;
    int terminal_height;
    int max_items_on_screen;
} Env_t;

typedef struct Menu
{
  WINDOW* menu_wnd;
  WINDOW** items;
  char text[NUM_MENU_ITEMS][TERMINAL_WIDTH];
  int top_of_text_array;
  int current_idx;
  int screen_idx;

  Env_t env;
} Menu_t;


void sig_winch(int signo);

void ncurses_init();
void ncurses_destroy();
void menu_init(Menu_t *menu);
void menu_destroy(Menu_t *menu);

void menu_go_up(Menu_t *menu);
void menu_go_down(Menu_t *menu);
void menu_move(Menu_t *menu);

void menu_act_on_item(Menu_t *menu);

void menu_do_routine();

#endif // _MENU_H_

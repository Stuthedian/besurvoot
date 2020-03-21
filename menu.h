#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24
#define NUM_MENU_ITEMS 5
#define STATUS_PLAY 0
#define STATUS_EXIT 1

typedef struct Menu_item
{
  WINDOW *item;
  char text[TERMINAL_WIDTH];
} Menu_item_t;

typedef struct Menu
{
  WINDOW *menu_wnd;
  Menu_item_t menu_items[NUM_MENU_ITEMS];
  int current_idx;
} Menu_t;


void sig_winch(int signo);
void check_terminal_size();

void ncurses_init();
void menu_init(Menu_t *menu);
void menu_destroy(Menu_t *menu);

void menu_go_up(Menu_t *menu);
void menu_go_down(Menu_t *menu);
int menu_move(Menu_t *menu);

int menu_act_on_item(Menu_t *menu);

int menu_do();

#endif // _MENU_H_

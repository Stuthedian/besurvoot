#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24
#define NUM_MENU_ITEMS 2
#define STATUS_PLAY 0
#define STATUS_EXIT 1

struct Menu {
  WINDOW *menu_wnd;
  WINDOW *menu_items[NUM_MENU_ITEMS];
  int current_idx;
};

void sig_winch(int signo);
void check_terminal_size();

void ncurses_init();
void menu_init(struct Menu *menu);
void menu_destroy(struct Menu *menu);

void menu_go_up(struct Menu *menu);
void menu_go_down(struct Menu *menu);
int menu_move(struct Menu *menu);

int menu_act_on_item(struct Menu *menu);

int menu_do();

void draw_waiting_for_connection();
void draw_waiting_for_player();
#endif // _MENU_H_

#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>
#include "check.h"
#include "linked_list.h"

#define CHECK_ERR CHECK(==, ERR);

#define BOX_OFFSET 2

typedef struct Env
{
    int max_items_on_screen;
} Env_t;

typedef enum User_action
{
    UA_QUIT = 'q',
    UA_DOWN = 'j',
    UA_UP = 'k',
    UA_ENTER = '\r',
    UA_NEW_ITEM = 'O',
} User_action_e;

typedef struct Menu
{
  WINDOW* menu_wnd;
  WINDOW* input_wnd;
  WINDOW** items;
  Linked_list_t text_list;
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

void menu_add_item(Menu_t *menu);
void menu_act_on_item(Menu_t *menu);

void menu_do_routine();

#endif // _MENU_H_

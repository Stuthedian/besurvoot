#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>
#include "check.h"
#include "linked_list.h"

#define CHECK_ERR CHECK(==, ERR);

#define BOX_OFFSET 2

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

  Linked_list_t text_list;
  int text_list_idx; // range 0 .. list.count - 1
  int top_of_text_list; // range 0 .. list.count - 1

  WINDOW** items;
  int num_items_on_screen; // min(max_items_on_screen, list.count)
  int max_items_on_screen; // term_height - BOX_OFFSET
  int screen_idx; // range 0 .. max_items_on_screen - 1
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

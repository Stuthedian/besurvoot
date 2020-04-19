#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>
#include "check.h"
#include "linked_list.h"

#define CHECK_ERR CHECK(==, ERR);

#define BOX_OFFSET 2
#define BESURVOOT_FILENAME ".besurvoot_commands"

typedef enum User_action
{
  UA_QUIT = 'q',
  UA_DOWN = 'j',
  UA_UP = 'k',
  UA_ENTER = '\r',
  UA_ADD_ITEM = 'O',
  UA_DEL_ITEM = 'D',
} User_action_e;

typedef struct Menu
{
  int height;
  WINDOW* menu_wnd;
  WINDOW* input_wnd;

  Linked_list_t text_list;
  int text_list_idx; // range 0 .. list.count - 1
  //also: text_list_idx == top_of_text_list + screen_idx
  int top_of_text_list; // minimum value = 0

  WINDOW** items;
  int num_items_on_screen; // min(max_items_on_screen, list.count)
  int max_items_on_screen; // term_height - BOX_OFFSET, min == 1
  int screen_idx; // maximum value = max_items_on_screen - 1
} Menu_t;


int get_height();
void fill_list_from_file(Linked_list_t* list);
void fill_file_from_list(const Linked_list_t* list);
char* remove_newline(char* string);

void ncurses_init();
void ncurses_destroy();
void menu_init(Menu_t* menu);
void menu_destroy(Menu_t* menu);

void menu_go_up(Menu_t* menu);
void menu_go_down(Menu_t* menu);
void menu_move(Menu_t* menu);

void menu_add_item(Menu_t* menu);
void menu_act_on_item(Menu_t* menu);
bool menu_should_resize(const int menu_height);
void menu_resize();

void menu_do_routine();

#endif // _MENU_H_

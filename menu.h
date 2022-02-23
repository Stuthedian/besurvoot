#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>
#include "linked_list.h"


#define NUM_OF_DIGITS 3
#define MENU_BOX_OFFSET 2
#define BESURVOOT_FILENAME ".besurvoot_commands"

typedef enum User_action
{
  UA_QUIT = 'q',
  UA_DOWN = 'j',
  UA_UP = 'k',
  UA_ENTER = '\r',
  UA_ADD_ITEM = 'O',
  UA_DEL_ITEM = 'D',
  UA_MOVE_TO_ITEM = 'G',
  UA_MOVE_TO_ITEM2 = 'g',
  UA_DO_REGIME = 'Z',
  UA_SEARCH_FORWARD = '/',
  UA_SEARCH_BACKWARD = '?',
} User_action_e;

typedef struct Menu
{
  int is_active;
  int in_do_regime;

  char row_num_str[NUM_OF_DIGITS];
  int row_num;

  int width;
  int height;
  WINDOW* menu_wnd;
  WINDOW* input_wnd;

  Linked_List_t text_list;
  int text_list_idx;
  int top_of_text_list;

  WINDOW** items;
  int num_items_on_screen; // min(max_items_on_screen, list.count)
  int max_items_on_screen; // term_height - BOX_OFFSET, min == 0
  int screen_idx;
} Menu_t;


int get_height();
void update_row_num(char* row_num_str, int ch);
void fill_list_from_file(Linked_List_t* list);
void fill_file_from_list(const Linked_List_t* list);
char* remove_newline(char* string);

void ncurses_init();
void ncurses_destroy();
void menu_init(Menu_t* menu);
void menu_destroy(Menu_t* menu);

void menu_go_up(Menu_t* menu, int repeat_count);
void menu_go_down(Menu_t* menu, int repeat_count);
void menu_wait_for_user_input(Menu_t* menu);

void menu_move_to_item(Menu_t* menu);
void menu_recolor(Menu_t* menu);
void menu_add_item(Menu_t* menu);
void menu_del_item(Menu_t* menu);
void menu_act_on_item(Menu_t* menu);
bool menu_should_resize(const int menu_height, const int menu_width);
void menu_resize(Menu_t* menu);
void menu_search_for_item(Menu_t* menu, bool search_forward);

void menu_do_routine();

#endif // _MENU_H_

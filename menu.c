#define _POSIX_C_SOURCE  1
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "menu.h"

Menu_t main_menu;

int get_height()
{
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ,
        (char*) &size) CHECK_IS_NEGATIVE_ONE;
  return size.ws_row;
}

bool menu_should_resize(const int menu_height)
{
  return (get_height() - menu_height) != 0;
}

void menu_resize()
{
  const int term_height = get_height();
  const int terminal_height_change = term_height - main_menu.height;
  const int terminal_height_change_abs = abs(terminal_height_change);
  const int menu_box_offset = BOX_OFFSET;
  const int menu_item_width = COLS - menu_box_offset;
  main_menu.height = term_height;
  main_menu.max_items_on_screen = main_menu.height - menu_box_offset;

  if(terminal_height_change_abs != 0)
  {
    resizeterm(term_height, COLS) CHECK_ERR;
    wresize(main_menu.menu_wnd, main_menu.height, COLS) CHECK_ERR;
  }

  if(main_menu.max_items_on_screen < 0)
    main_menu.max_items_on_screen = 0;

  if(main_menu.max_items_on_screen <= 0)
  {
    for(int i = 0; i < main_menu.height; i++)
      mvwhline(main_menu.menu_wnd, i, 0, 'x', COLS) CHECK_ERR;
  }
  else
  {
    wclear(main_menu.menu_wnd) CHECK_ERR;
    wbkgd(main_menu.menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
    box(main_menu.menu_wnd, '|', '-') CHECK_ERR;
  }

  int num_items_on_screen_prev = main_menu.num_items_on_screen;

  //minimum of available screen space and number of items
  main_menu.num_items_on_screen = main_menu.max_items_on_screen >
                                  main_menu.text_list.count ? main_menu.text_list.count :
                                  main_menu.max_items_on_screen;

  int num_items_on_screen_diff = main_menu.num_items_on_screen -
                                 num_items_on_screen_prev;

  if(num_items_on_screen_diff > 0)// allocate new windows
  {
    //realloc before allocating new windows as we need some place to store pointers
    main_menu.items = realloc(main_menu.items,
                              main_menu.num_items_on_screen * sizeof(WINDOW*));
    main_menu.items CHECK_IS_NULL;

    for(int i = num_items_on_screen_prev;
        i < main_menu.num_items_on_screen;
        i++)
    {
      main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
                                  i + menu_box_offset / 2,
                                  0 + menu_box_offset / 2);

      if(main_menu.items[i] == NULL)
      {
        if(menu_should_resize(main_menu.height))
        {
          menu_resize();
          return;
        }
        else main_menu.items[i] CHECK_IS_NULL;
      }
    }
  }
  else if(num_items_on_screen_diff < 0)// deallocate unused windows
  {
    //realloc after deallocation as we need not to loose pointers to windows
    for(int i = num_items_on_screen_prev - 1;
        i >= main_menu.num_items_on_screen;
        i--)
    {
      /* if item is NULL it means that we fail to derwin it in previous
         call to menu_resize() because we were unaware that pane height
         was changed */
      delwin(main_menu.items[i]);//CHECK_ERR;
    }

    main_menu.items = realloc(main_menu.items,
                              main_menu.num_items_on_screen * sizeof(WINDOW*));

    if(main_menu.num_items_on_screen != 0)
      main_menu.items CHECK_IS_NULL;
  }


  //pane enlarged
  if(terminal_height_change > 0 && main_menu.num_items_on_screen != 0)
  {
    int end_of = num_items_on_screen_prev +
                 main_menu.top_of_text_list;
    int num_below = main_menu.text_list.count - 1 - end_of;

    if(num_below < 0)
      num_below = 0;

    if(terminal_height_change_abs > num_below)
    {

      int top_of_text_list_prev = main_menu.top_of_text_list;
      main_menu.top_of_text_list -= terminal_height_change_abs - num_below;

      if(main_menu.top_of_text_list < 0) //do not overjump
        main_menu.top_of_text_list = 0;

      main_menu.screen_idx += top_of_text_list_prev -
                              main_menu.top_of_text_list;
    }
  }
  //pane shrank
  else if(terminal_height_change < 0
          && main_menu.num_items_on_screen != 0)
  {
    if(main_menu.screen_idx >= main_menu.max_items_on_screen)
    {
      int screen_idx_prev = main_menu.screen_idx;
      main_menu.screen_idx = main_menu.max_items_on_screen - 1;
      main_menu.top_of_text_list += screen_idx_prev - main_menu.screen_idx;
    }
  }

  for(int i = 0, j = main_menu.top_of_text_list;
      i < main_menu.num_items_on_screen; i++, j++)
  {
    delwin(main_menu.items[i]) CHECK_ERR;
    main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
                                i + menu_box_offset / 2,
                                0 + menu_box_offset / 2);

    if(main_menu.items[i] == NULL)
    {
      if(menu_should_resize(main_menu.height))
      {
        menu_resize();
        return;
      }
      else main_menu.items[i] CHECK_IS_NULL;
    }

    wbkgd(main_menu.items[i],
          COLOR_PAIR(1) | (i == main_menu.screen_idx ? A_REVERSE : A_NORMAL))
    CHECK_ERR;
    wclear(main_menu.items[i]) CHECK_ERR;
    wprintw(main_menu.items[i], list_find(main_menu.text_list,
                                          j)) CHECK_ERR;
  }

  touchwin(main_menu.menu_wnd) CHECK_ERR;
  wrefresh(main_menu.menu_wnd) CHECK_ERR;
}

void ncurses_init()
{
  initscr() CHECK_IS_NULL;
  cbreak() CHECK_ERR;
  noecho() CHECK_ERR;

  curs_set(FALSE) CHECK_ERR;

  start_color() CHECK_ERR;
  init_pair(1, COLOR_YELLOW, COLOR_BLUE) CHECK_ERR;
  init_pair(3, COLOR_GREEN, COLOR_BLACK) CHECK_ERR;

  nonl() CHECK_ERR;
  intrflush(stdscr, FALSE) CHECK_ERR;
  keypad(stdscr, TRUE) CHECK_ERR;

  halfdelay(5) CHECK_ERR;

  refresh() CHECK_ERR;
}

void ncurses_destroy()
{
  endwin() CHECK_ERR;
}

char* remove_newline(char* string)
{
  int i = strlen(string);

  if(string[i - 1] == '\n')
    string[i - 1] = '\0';

  return string;
}

void fill_list_from_file(Linked_list_t* list)
{
  char temp_str[STRING_WIDTH];
  FILE* file = fopen(BESURVOOT_FILENAME, "a+");
  file CHECK_IS_NULL;

  while(TRUE)
  {
    if(fgets(temp_str, STRING_WIDTH, file) == NULL)
      break;

    list_add(list, remove_newline(temp_str));
  }
}

void menu_init(Menu_t* menu)
{
  const int menu_box_offset = BOX_OFFSET;
  const int menu_ncurses_y = 0;
  const int menu_ncurses_x = 0;
  const int menu_item_width = COLS - menu_box_offset;
  menu->height = LINES;
  menu->max_items_on_screen = menu->height - menu_box_offset;
  menu->screen_idx = 0;
  menu->text_list_idx = 0;
  menu->top_of_text_list = 0;

  /*list_add(&menu->text_list,  "show firmware");
  list_add(&menu->text_list,  "reload system");
  list_add(&menu->text_list,  "show running config");
  list_add(&menu->text_list,  "show msdp vrf test peers");
  list_add(&menu->text_list,  "Exit");*/
  menu->text_list.count = 0;
  fill_list_from_file(&menu->text_list);

  if(menu->text_list.count == 0)
  {
    menu->screen_idx = -1;
    menu->text_list_idx = -1;
    menu->top_of_text_list = -1;
  }

  menu->num_items_on_screen = menu->max_items_on_screen >
                              menu->text_list.count ?
                              menu->text_list.count : menu->max_items_on_screen;

  menu->menu_wnd = newwin(menu->height, COLS, menu_ncurses_y,
                          menu_ncurses_x);
  menu->menu_wnd CHECK_IS_NULL;

  wbkgd(menu->menu_wnd, COLOR_PAIR(1)) CHECK_ERR;

  menu->items = NULL;

  if(menu->max_items_on_screen <= 0)
  {
    for(int i = 0; i < menu->height; i++)
      mvwhline(menu->menu_wnd, i, 0, 'x', COLS) CHECK_ERR;

    wrefresh(menu->menu_wnd) CHECK_ERR;
    return;
  }

  menu->items = malloc(menu->num_items_on_screen * sizeof(WINDOW*));
  menu->items CHECK_IS_NULL;

  box(menu->menu_wnd, '|', '-') CHECK_ERR;

  for(int i = 0; i < menu->num_items_on_screen; i++)
  {
    menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
                            i + menu_box_offset / 2,
                            0 + menu_box_offset / 2);
    menu->items[i] CHECK_IS_NULL;
    wprintw(menu->items[i], list_find(menu->text_list, i)) CHECK_ERR;
  }

  if(menu->num_items_on_screen != 0)
    wbkgd(menu->items[menu->screen_idx],
          COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

  touchwin(main_menu.menu_wnd) CHECK_ERR;
  wrefresh(menu->menu_wnd) CHECK_ERR;
}

void menu_destroy(Menu_t* menu)
{
  for(int i = 0; i < menu->num_items_on_screen; i++)
    delwin(menu->items[i]) CHECK_ERR;

  list_free(&menu->text_list);
  free(menu->items);
  delwin(menu->menu_wnd) CHECK_ERR;
}

void menu_go_down(Menu_t* menu)
{
  if(menu->text_list_idx + 1 < menu->text_list.count)
  {
    wbkgd(menu->items[menu->screen_idx],
          COLOR_PAIR(1) | A_NORMAL) CHECK_ERR;
    menu->screen_idx++;
    menu->text_list_idx++;

    if(menu->screen_idx >= menu->max_items_on_screen)
    {
      menu->top_of_text_list++;

      for(int i = 0, j = menu->top_of_text_list;
          i < menu->num_items_on_screen;
          i++, j++)
      {
        wclear(menu->items[i]) CHECK_ERR;
        wprintw(menu->items[i], list_find(menu->text_list, j)) CHECK_ERR;
      }

      menu->screen_idx--;
    }

    wbkgd(menu->items[menu->screen_idx],
          COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
    touchwin(menu->menu_wnd) CHECK_ERR;
    wrefresh(menu->menu_wnd) CHECK_ERR;
  }
}

void menu_go_up(Menu_t* menu)
{
  if(menu->text_list_idx - 1 >= 0)
  {
    wbkgd(menu->items[menu->screen_idx],
          COLOR_PAIR(1) | A_NORMAL) CHECK_ERR;
    menu->screen_idx--;
    menu->text_list_idx--;

    if(menu->screen_idx < 0)
    {
      menu->top_of_text_list--;

      for(int i = 0, j = menu->top_of_text_list;
          i < menu->num_items_on_screen;
          i++, j++)
      {
        wclear(menu->items[i]) CHECK_ERR;
        wprintw(menu->items[i], list_find(menu->text_list, j)) CHECK_ERR;
      }

      menu->screen_idx++;
    }

    wbkgd(menu->items[menu->screen_idx],
          COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
    touchwin(menu->menu_wnd) CHECK_ERR;
    wrefresh(menu->menu_wnd) CHECK_ERR;
  }
}

void menu_move(Menu_t* menu)
{
  while(1)
  {
    int ch = wgetch(menu->menu_wnd);

    if(menu_should_resize(menu->height))
      menu_resize();

    switch(ch)
    {
    case UA_DOWN:
    case KEY_DOWN:
      menu_go_down(menu);
      break;

    case UA_UP:
    case KEY_UP:
      menu_go_up(menu);
      break;

    case UA_ENTER:
      menu_act_on_item(menu);
      break;

    case UA_ADD_ITEM:
      menu_add_item(menu);
      break;

    /*case UA_DEL_ITEM:
        menu_del_item(menu); break;*/
    case UA_QUIT:
      return;

    case ERR:
    default:
      break;
    }
  }
}

/*void menu_del_item(Menu_t *menu)
{
    sig_winch(0);
}*/
void menu_add_item(Menu_t* menu)
{
  char* user_input = NULL;
  const int menu_item_width = COLS - BOX_OFFSET;
  user_input = malloc(menu_item_width * sizeof(char) + 1);
  user_input CHECK_IS_NULL;

  menu->input_wnd = derwin(menu->menu_wnd, 1, menu_item_width,
                           menu->height - 1,
                           0 + BOX_OFFSET / 2);
  menu->input_wnd CHECK_IS_NULL;
  wbkgd(menu->input_wnd, COLOR_PAIR(3)) CHECK_ERR;
  werase(menu->input_wnd);

  echo() CHECK_ERR;
  wgetnstr(menu->input_wnd, user_input, menu_item_width) CHECK_ERR;
  noecho() CHECK_ERR;

  delwin(menu->input_wnd) CHECK_ERR;
  list_add(&menu->text_list, user_input);
  free(user_input);

  menu->screen_idx = 0;
  menu->text_list_idx = 0;
  menu->top_of_text_list = 0;

  menu_resize();

  while(menu->text_list_idx != menu->text_list.count - 1)
    menu_go_down(menu);
}

void menu_act_on_item(Menu_t* menu)
{
  char* result_command = NULL;
  char* prefix = "tmux send-keys -t ! \"";
  char* suffix = "\" Enter";
  char* command = list_find(menu->text_list, menu->text_list_idx);

  if(command == NULL)
    return;

  result_command = malloc(strlen(prefix) + strlen(command) + strlen(
                            suffix) + 1);
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

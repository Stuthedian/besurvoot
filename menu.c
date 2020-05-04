#define BAHT_NCURSES
#define BAHT_IMPLEMENTATION
#define _POSIX_C_SOURCE 2
#include "baht.h"

#include <ctype.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "menu.h"


#define assert_conditions(menu) do\
{\
  assert(menu->max_items_on_screen >= 0);\
  assert(menu->num_items_on_screen >= 0);\
  assert(menu->text_list_idx >= 0\
         && menu->text_list_idx < menu->text_list.count);\
  assert(menu->top_of_text_list >= 0\
         && menu->top_of_text_list < menu->text_list.count);\
  assert(menu->screen_idx >= 0\
         && menu->screen_idx < menu->num_items_on_screen);\
  assert(menu->text_list_idx == menu->top_of_text_list +\
         menu->screen_idx);\
} while(0)

void get_size(int* height, int* width)
{
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ,
        (char*) &size) BAHT_IS_NEG_1_ERRNO;
  *height = size.ws_row;
  *width = size.ws_col;
}

bool menu_should_resize(const int menu_height, const int menu_width)
{
  int term_height, term_width;
  get_size(&term_height, &term_width);
  return term_height != menu_height || term_width != menu_width;
}

void menu_enlarge(Menu_t* menu, const int num_items_on_screen_prev)
{
  int num_of_hidden_items = menu->text_list.count -
                            num_items_on_screen_prev;

  if(num_of_hidden_items > 0)
  {
    int num_above_screen = menu->top_of_text_list;
    int num_below_screen = menu->text_list.count - (menu->top_of_text_list
                           + num_items_on_screen_prev);
    assert(num_below_screen >= 0);
    assert(num_below_screen + num_above_screen == num_of_hidden_items);
    int num_above_screen_can_show = menu->num_items_on_screen -
                                    num_items_on_screen_prev -
                                    num_below_screen;

    if(num_above_screen_can_show > 0)
    {
      menu->screen_idx += num_above_screen_can_show;
      menu->top_of_text_list -= num_above_screen_can_show;
    }
  }
}

void menu_shrink(Menu_t* menu, const int num_items_on_screen_prev)
{
  int num_items_on_screen_diff = num_items_on_screen_prev - menu->num_items_on_screen;

  if(menu->screen_idx >= menu->max_items_on_screen)
  {
    menu->screen_idx -= num_items_on_screen_diff;
    menu->top_of_text_list += num_items_on_screen_diff;
  }
}

void menu_allocate_items(Menu_t* menu)
{
  const int menu_box_offset = MENU_BOX_OFFSET;
  const int menu_item_width = COLS - menu_box_offset;

  for(int i = 0; i < menu->num_items_on_screen; i++)
  {
    menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
                            i + menu_box_offset / 2,
                            0 + menu_box_offset / 2);
    menu->items[i] BAHT_IS_NULL;
  }
}

void menu_repaint_items(Menu_t* menu)
{
  for(int i = 0, j = menu->top_of_text_list;
      i < menu->num_items_on_screen; i++, j++)
  {
    wbkgd(menu->items[i],
          (menu->is_active ? COLOR_PAIR(1) : COLOR_PAIR(2))
          | (i == menu->screen_idx ? A_REVERSE : A_NORMAL))
    BAHT_IS_ERR;
    wclear(menu->items[i]) BAHT_IS_ERR;
    wprintw(menu->items[i], "[%d] %s", j+1,
            list_get_value(&menu->text_list, j));// BAHT_IS_ERR;
  }


  touchwin(menu->menu_wnd) BAHT_IS_ERR;
  wrefresh(menu->menu_wnd) BAHT_IS_ERR;
}

void menu_resize(Menu_t* menu)
{
  int term_height, term_width;
  const int menu_box_offset = MENU_BOX_OFFSET;

  get_size(&term_height, &term_width);
  resizeterm(term_height, term_width) BAHT_IS_ERR;
  wresize(menu->menu_wnd, term_height, term_width) BAHT_IS_ERR;

  menu->height = term_height;
  menu->width = term_width;

  if((term_height == 1 || term_height == 2) || (term_width == 1
      || term_width == 2))
  {
    for(int i = 0; i < term_height; i++)
      mvwhline(menu->menu_wnd, i, 0, 'x', term_width) BAHT_IS_ERR;

    return;
  }
  else
  {
    wclear(menu->menu_wnd) BAHT_IS_ERR;
    wbkgd(menu->menu_wnd,
        (menu->is_active ? COLOR_PAIR(1) : COLOR_PAIR(2))) BAHT_IS_ERR;
    box(menu->menu_wnd, '|', '-') BAHT_IS_ERR;
  }

  menu->max_items_on_screen = menu->height - menu_box_offset;
  assert(menu->max_items_on_screen >= 0);
  menu->max_items_on_screen = menu->max_items_on_screen;


  const int num_items_on_screen_prev = menu->num_items_on_screen;

  //minimum of available screen space and number of items
  menu->num_items_on_screen = menu->max_items_on_screen >
                              menu->text_list.count ? menu->text_list.count :
                              menu->max_items_on_screen;
  assert(menu->num_items_on_screen >= 0);

  const int num_items_on_screen_diff = menu->num_items_on_screen -
                                       num_items_on_screen_prev;

  for(int i = 0; i < num_items_on_screen_prev; i++)
    delwin(menu->items[i]) BAHT_IS_ERR;

  menu->items = realloc(menu->items,
                        menu->num_items_on_screen * sizeof(WINDOW*));

  if(menu->text_list.count != 0)
  {
    if(num_items_on_screen_diff > 0)
      menu_enlarge(menu, num_items_on_screen_prev);
    else if(num_items_on_screen_diff < 0)
      menu_shrink(menu, num_items_on_screen_prev);

    assert_conditions(menu);

    if(menu_should_resize(menu->height, menu->width))
    {
      menu_resize(menu);
      return;
    }
    else
    {
      menu_allocate_items(menu);
      menu_repaint_items(menu);
    }
  }
}

void ncurses_init()
{
  initscr() BAHT_IS_NULL_ERRNO;
  cbreak() BAHT_IS_ERR;
  noecho() BAHT_IS_ERR;

  curs_set(FALSE) BAHT_IS_ERR;

  start_color() BAHT_IS_ERR;
  init_pair(1, COLOR_YELLOW, COLOR_BLUE) BAHT_IS_ERR;
  init_pair(2, COLOR_WHITE, COLOR_BLACK) BAHT_IS_ERR;
  init_pair(3, COLOR_GREEN, COLOR_BLACK) BAHT_IS_ERR;

  nonl() BAHT_IS_ERR;
  intrflush(stdscr, FALSE) BAHT_IS_ERR;
  keypad(stdscr, TRUE) BAHT_IS_ERR;

  halfdelay(5) BAHT_IS_ERR;

  refresh() BAHT_IS_ERR;
}

void ncurses_destroy()
{
  endwin() BAHT_IS_ERR;
}

char* remove_newline(char* string)
{
  int i = strlen(string);

  if(string[i - 1] == '\n')
    string[i - 1] = '\0';

  return string;
}

void fill_list_from_file(Linked_List_t* list)
{
  char temp_str[STRING_WIDTH];
  FILE* file = fopen(BESURVOOT_FILENAME, "a+");
  file BAHT_IS_NULL_ERRNO;

  while(TRUE)
  {
    if(fgets(temp_str, STRING_WIDTH, file) == NULL)
      break;

    list_add(list, remove_newline(temp_str));
  }

  fclose(file) BAHT_IS_EOF_ERRNO;
}

void fill_file_from_list(const Linked_List_t* list)
{
  FILE* file = fopen(BESURVOOT_FILENAME, "w");
  file BAHT_IS_NULL_ERRNO;

  for(int i = 0; i < list->count; i++)
  {
    fputs(list_get_value(list, i), file) BAHT_IS_EOF_ERRNO;
    fputc('\n', file) BAHT_IS_EOF_ERRNO;
  }

  fclose(file) BAHT_IS_EOF_ERRNO;
}

void menu_init(Menu_t* menu)
{
  const int menu_ncurses_y = 0;
  const int menu_ncurses_x = 0;
  menu->is_active = 1;
  menu->width = COLS;
  menu->height = LINES;
  menu->max_items_on_screen = menu->height - MENU_BOX_OFFSET;
  menu->max_items_on_screen = menu->max_items_on_screen < 0 ? 0 :
                              menu->max_items_on_screen;
  menu->row_num = 0;
  memset(menu->row_num_str, '\0', NUM_OF_DIGITS);

  menu->screen_idx = 0;
  menu->text_list_idx = 0;
  menu->top_of_text_list = 0;

  list_init(&menu->text_list);
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

  menu->menu_wnd = newwin(menu->height, menu->width, menu_ncurses_y,
                          menu_ncurses_x);
  menu->menu_wnd BAHT_IS_NULL_ERRNO;

  wbkgd(menu->menu_wnd,
        (menu->is_active ? COLOR_PAIR(1) : COLOR_PAIR(2))) BAHT_IS_ERR;

  menu->items = NULL;

  if(menu->max_items_on_screen == 0)
  {
    for(int i = 0; i < menu->height; i++)
      mvwhline(menu->menu_wnd, i, 0, 'x', COLS) BAHT_IS_ERR;

    wrefresh(menu->menu_wnd) BAHT_IS_ERR;
    menu->height = 0;
    return;
  }

  menu->items = malloc(menu->num_items_on_screen * sizeof(WINDOW*));
  menu->items BAHT_IS_NULL_ERRNO;

  box(menu->menu_wnd, '|', '-') BAHT_IS_ERR;
  menu_allocate_items(menu);
  menu_repaint_items(menu);


  touchwin(menu->menu_wnd) BAHT_IS_ERR;
  wrefresh(menu->menu_wnd) BAHT_IS_ERR;
}

void menu_destroy(Menu_t* menu)
{
  for(int i = 0; i < menu->num_items_on_screen; i++)
    delwin(menu->items[i]) BAHT_IS_ERR;

  list_destroy(&menu->text_list);
  free(menu->items);
  delwin(menu->menu_wnd) BAHT_IS_ERR;
}

void menu_go_down(Menu_t* menu, int repeat_count)
{
  if(repeat_count <= 0)
    return;

  for(int i = 0; i < repeat_count; i++)
  {
    if(menu->text_list_idx + 1 < menu->text_list.count)
    {
      menu->screen_idx++;
      menu->text_list_idx++;

      if(menu->screen_idx >= menu->max_items_on_screen)
      {
        menu->top_of_text_list++;
        menu->screen_idx--;
      }

      assert_conditions(menu);
    }
  }

  menu_repaint_items(menu);
  touchwin(menu->menu_wnd) BAHT_IS_ERR;
  wrefresh(menu->menu_wnd) BAHT_IS_ERR;
}

void menu_go_up(Menu_t* menu, int repeat_count)
{
  if(repeat_count <= 0)
    return;

  for(int i = 0; i < repeat_count; i++)
  {
    if(menu->text_list_idx - 1 >= 0)
    {
      menu->screen_idx--;
      menu->text_list_idx--;

      if(menu->screen_idx < 0)
      {
        menu->top_of_text_list--;
        menu->screen_idx++;
      }

      assert_conditions(menu);
    }
  }

  menu_repaint_items(menu);
  touchwin(menu->menu_wnd) BAHT_IS_ERR;
  wrefresh(menu->menu_wnd) BAHT_IS_ERR;
}

void menu_recolor(Menu_t* menu)
{
  char* command = "tmux display-message -p -t $TMUX_PANE -F '#{pane_active}'";
  FILE* pipe = popen(command, "r");
  pipe BAHT_IS_NULL_ERRNO;

  int result = fgetc(pipe);

  if(result == '0' && menu->is_active)
    menu->is_active = 0;
  else if(result == '1' && !menu->is_active)
    menu->is_active = 1;

  wbkgd(menu->menu_wnd,
        (menu->is_active ? COLOR_PAIR(1) : COLOR_PAIR(2))) BAHT_IS_ERR;

  wrefresh(menu->menu_wnd) BAHT_IS_ERR;
  pclose(pipe) BAHT_IS_NEG_1_ERRNO;
}

void menu_wait_for_user_input(Menu_t* menu)
{
  bool was_move_to_item2_pressed = FALSE;

  while(1)
  {
    int ch = wgetch(menu->menu_wnd);
    menu_recolor(menu);

    if(menu_should_resize(menu->height, menu->width))
      menu_resize(menu);

    if((menu->height == 1 || menu->height == 2)
        || (menu->width == 1 || menu->width == 2))
      if(ch != UA_QUIT)
        ch = ERR;

    if(menu->text_list.count == 0 && (ch != UA_QUIT && ch != UA_ADD_ITEM))
      ch = ERR;

    switch(ch)
    {
    case UA_DOWN:
    case KEY_DOWN:
      menu_go_down(menu, 1);
      break;

    case UA_UP:
    case KEY_UP:
      menu_go_up(menu, 1);
      break;

    case UA_ENTER:
      menu_act_on_item(menu);
      break;

    case UA_ADD_ITEM:
      menu_add_item(menu);
      break;

    case UA_MOVE_TO_ITEM:
      menu_move_to_item(menu);
      break;

    case UA_MOVE_TO_ITEM2:
      if(was_move_to_item2_pressed == FALSE)
        was_move_to_item2_pressed = TRUE;
      else
      {
        was_move_to_item2_pressed = FALSE;
        menu_move_to_item(menu);
      }
      break;

    case UA_DEL_ITEM:
        menu_del_item(menu);
        break;

    case UA_QUIT:
      return;

    case ERR:
    default:
      break;
    }


    if(ch != ERR && was_move_to_item2_pressed != TRUE)
    {
      update_row_num(menu->row_num_str, ch);
      was_move_to_item2_pressed = FALSE;
    }
  }
}

void update_row_num(char* row_num_str, int ch)
{
  if(isdigit(ch))
  {
    for(int i = 0; i < NUM_OF_DIGITS; i++)
    {
      if(row_num_str[i] == '\0')
      {
        row_num_str[i] = (char)ch;
        break;
      }
    }
  }
  else
    memset(row_num_str, '\0', NUM_OF_DIGITS);
}

void menu_move_to_item(Menu_t* menu)
{
  int row_num = strtoul(menu->row_num_str, NULL, 10);

  if(row_num == 0 || row_num == menu->text_list_idx + 1)
    return;
  else
  {
    if(row_num > menu->text_list.count)
      row_num = menu->text_list.count;

    int diff = row_num - (menu->text_list_idx + 1);

    if(diff > 0)
      menu_go_down(menu, abs(diff));
    else
      menu_go_up(menu, abs(diff));
  }
}

void menu_del_item(Menu_t* menu)
{
  list_del(&menu->text_list, menu->text_list_idx);
  menu->text_list_idx--;

  if(menu->top_of_text_list == 0)
  {
    int num_below_screen = (menu->text_list.count + 1) - menu->num_items_on_screen;

    if(num_below_screen == 0)
    {
      for(int i = 0; i < menu->num_items_on_screen; i++)
        delwin(menu->items[i]) BAHT_IS_ERR;

      menu->num_items_on_screen -= 1;
      menu->items = realloc(menu->items,
          menu->num_items_on_screen * sizeof(WINDOW*));

      menu_allocate_items(menu);
    }

    if(menu->text_list_idx < 0)
      menu->text_list_idx++;
    else
      menu->screen_idx--;

    if(menu->text_list.count != 0)
      assert_conditions(menu);
    else
    {
      menu->screen_idx = -1;
      menu->text_list_idx = -1;
      menu->top_of_text_list = -1;
    }

    wclear(menu->menu_wnd) BAHT_IS_ERR;
    wbkgd(menu->menu_wnd, COLOR_PAIR(1)) BAHT_IS_ERR;
    box(menu->menu_wnd, '|', '-') BAHT_IS_ERR;
  }
  else
  {
    menu->top_of_text_list--;
    assert_conditions(menu);
  }

  menu_repaint_items(menu);
}

void menu_add_item(Menu_t* menu)
{
  char* user_input = NULL;
  const int menu_item_width = COLS - MENU_BOX_OFFSET;
  user_input = malloc(menu_item_width * sizeof(char) + 1);
  user_input BAHT_IS_NULL_ERRNO;

  menu->input_wnd = derwin(menu->menu_wnd, 1, menu_item_width,
                           menu->height - 1,
                           0 + MENU_BOX_OFFSET / 2);
  menu->input_wnd BAHT_IS_NULL;
  wbkgd(menu->input_wnd, COLOR_PAIR(3)) BAHT_IS_ERR;
  werase(menu->input_wnd);

  echo() BAHT_IS_ERR;
  curs_set(TRUE) BAHT_IS_ERR;
  wgetnstr(menu->input_wnd, user_input, menu_item_width) BAHT_IS_ERR;
  curs_set(FALSE) BAHT_IS_ERR;
  noecho() BAHT_IS_ERR;
  halfdelay(5) BAHT_IS_ERR;//restore delay because it changed by wgetnstr

  delwin(menu->input_wnd) BAHT_IS_ERR;
  list_add(&menu->text_list, user_input);
  free(user_input);

  if(menu->text_list.count == 1)
  {
    menu->screen_idx = 0;
    menu->text_list_idx = 0;
    menu->top_of_text_list = 0;
  }

  menu_resize(menu);

  menu_go_down(menu, menu->text_list.count - 1 - menu->text_list_idx);
}

void menu_act_on_item(Menu_t* menu)
{
  char* result_command = NULL;
  char* prefix = "tmux send-keys -t ";
  char* target_pane = menu->row_num_str[0] == '\0' ? "!" : menu->row_num_str;
  char* command = list_get_value(&menu->text_list, menu->text_list_idx);
  char* suffix = "Enter 2>/dev/null";

  if(command == NULL)
    return;

  //2 - space and double quote
  int result_command_size = strlen(prefix) + strlen(target_pane) + 2
    + strlen(command) + 2 + strlen(suffix) + 1;
  result_command = malloc(result_command_size);
  result_command BAHT_IS_NULL_ERRNO;
  result_command[0] = '\0';

  snprintf(result_command, result_command_size,
      "%s%s \"%s\" %s", prefix, target_pane, command, suffix);

  system(result_command) BAHT_IS_NEG_1_ERRNO;

  free(result_command);
}

void menu_do_routine()
{
  Menu_t main_menu;

  menu_init(&main_menu);
  menu_wait_for_user_input(&main_menu);
  fill_file_from_list(&main_menu.text_list);
  menu_destroy(&main_menu);
}

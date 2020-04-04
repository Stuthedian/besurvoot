#define _POSIX_C_SOURCE  1
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "menu.h"

Menu_t main_menu;

void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size) CHECK_IS_NEGATIVE_ONE;
    const int terminal_height_change = size.ws_row - main_menu.height;
    resizeterm(size.ws_row, size.ws_col) CHECK_ERR;

    const int terminal_height_change_abs = abs(terminal_height_change);
    const int menu_box_offset = BOX_OFFSET;
    const int menu_ncurses_y = 0;
    const int menu_ncurses_x = 0;
    const int menu_item_width = COLS - menu_box_offset;
    main_menu.height = size.ws_row;
    main_menu.max_items_on_screen = main_menu.height - menu_box_offset;

    if(terminal_height_change_abs != 0)
        wresize(main_menu.menu_wnd, main_menu.height, COLS) CHECK_ERR;

    int num_items_on_screen_prev = main_menu.num_items_on_screen;
    //minimum of available screen space and number of items
    main_menu.num_items_on_screen = main_menu.max_items_on_screen > main_menu.text_list.count ? main_menu.text_list.count : main_menu.max_items_on_screen;

    int num_items_on_screen_diff = main_menu.num_items_on_screen - num_items_on_screen_prev;
    int end_of_text_list_prev = main_menu.top_of_text_list + num_items_on_screen_prev;


    if(num_items_on_screen_diff > 0)// allocate new windows
    {
        //realloc before allocating new windows as we need some place to store pointers
        main_menu.items = realloc(main_menu.items, main_menu.num_items_on_screen * sizeof(WINDOW*));
        main_menu.items CHECK_IS_NULL;
        for (int i = num_items_on_screen_prev; i < main_menu.num_items_on_screen; i++)
        {
            main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
                    i + menu_box_offset / 2,
                    0 + menu_box_offset / 2);
            main_menu.items[i] CHECK_IS_NULL;
        }
    }
    else if(num_items_on_screen_diff < 0)// deallocate unused windows
    {
        //realloc after deallocation as we need not to loose pointers to windows
        for (int i = num_items_on_screen_prev - 1; i >= main_menu.num_items_on_screen; i--)
        {
            wclear(main_menu.items[i]) CHECK_ERR;
            wrefresh(main_menu.items[i]) CHECK_ERR;
            delwin(main_menu.items[i]) CHECK_ERR;
        }
        main_menu.items = realloc(main_menu.items, main_menu.num_items_on_screen * sizeof(WINDOW*));
        main_menu.items CHECK_IS_NULL;
    }
    if(terminal_height_change > 0)//pane enlarged
    {
        //num of items that are currently on screen plus items that "below" the screen
        int to_show = main_menu.text_list.count - 1 - main_menu.top_of_text_list + 1;
        if(main_menu.max_items_on_screen > to_show)
        {
            int num_below_screen = to_show - num_items_on_screen_prev; //num of items that "below" the screen
            int num_above_screen_to_show = terminal_height_change_abs - num_below_screen;
            if(num_above_screen_to_show > 0) //show items that "above" the screen
            {
                int top_of_text_list_prev = main_menu.top_of_text_list;
                main_menu.top_of_text_list -= num_above_screen_to_show;
                if(main_menu.top_of_text_list < 0) //do not overjump
                    main_menu.top_of_text_list = 0;
                wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_NORMAL) CHECK_ERR;
                main_menu.screen_idx += top_of_text_list_prev - main_menu.top_of_text_list;
                wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
            }
        }
    }
    else if(terminal_height_change < 0)//pane shrank
    {
        if(main_menu.screen_idx >= main_menu.max_items_on_screen)
        {
            int screen_idx_prev = main_menu.screen_idx;
            main_menu.screen_idx = main_menu.max_items_on_screen - 1;
            wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;
            main_menu.top_of_text_list += screen_idx_prev - main_menu.screen_idx;
        }
    }
    for (int i = 0, j = main_menu.top_of_text_list; i < main_menu.num_items_on_screen; i++, j++)
    {
        wclear(main_menu.items[i]) CHECK_ERR;
        wprintw(main_menu.items[i], list_find(main_menu.text_list, j)) CHECK_ERR;
        wrefresh(main_menu.items[i]) CHECK_ERR;
    }


    //wbkgd(main_menu.menu_wnd, COLOR_PAIR(1)) CHECK_ERR;

    //wclear(main_menu.menu_wnd) CHECK_ERR;
    //box(main_menu.menu_wnd, '|', '-') CHECK_ERR;
    wrefresh(main_menu.menu_wnd) CHECK_ERR;
    //refresh() CHECK_ERR;
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

  //sigaction(SIGWINCH, &sa_handler, NULL) CHECK_IS_NEGATIVE_ONE;

  curs_set(FALSE) CHECK_ERR;

  start_color() CHECK_ERR;
  init_pair(1, COLOR_YELLOW, COLOR_BLUE) CHECK_ERR;
  init_pair(2, COLOR_YELLOW, COLOR_CYAN) CHECK_ERR;
  init_pair(3, COLOR_GREEN, COLOR_BLACK) CHECK_ERR;
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
    const int menu_ncurses_y = 0;
    const int menu_ncurses_x = 0;
    const int menu_item_width = COLS - menu_box_offset;
    menu->height = LINES;
    menu->max_items_on_screen = menu->height - menu_box_offset;

    list_add(&menu->text_list,  "show firmware");
    list_add(&menu->text_list,  "reload system");
    list_add(&menu->text_list,  "show running config");
    list_add(&menu->text_list,  "show msdp vrf test peers");
    list_add(&menu->text_list,  "Exit");

    menu->num_items_on_screen = menu->max_items_on_screen > menu->text_list.count ? menu->text_list.count : menu->max_items_on_screen;

    menu->menu_wnd = newwin(menu->height, COLS, menu_ncurses_y, menu_ncurses_x);
    menu->menu_wnd CHECK_IS_NULL;
    menu->items = malloc(menu->num_items_on_screen * sizeof(WINDOW*));
    menu->items CHECK_IS_NULL;



    menu->screen_idx = 0;
    menu->text_list_idx = 0;
    menu->top_of_text_list = 0;

    for (int i = 0; i < menu->num_items_on_screen; i++)
    {
        menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
                i + menu_box_offset / 2,
                0 + menu_box_offset / 2);
        menu->items[i] CHECK_IS_NULL;
        wprintw(menu->items[i], list_find(menu->text_list, i)) CHECK_ERR;
    }

    wbkgd(menu->menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

    //box(menu->menu_wnd, '|', '-') CHECK_ERR;
    wrefresh(menu->menu_wnd) CHECK_ERR;
}

void menu_destroy(Menu_t *menu)
{
    for (int i = 0, j = menu->top_of_text_list; i < menu->max_items_on_screen && j < menu->text_list.count; i++, j++)
        delwin(menu->items[i]) CHECK_ERR;

    list_free(&menu->text_list);
    free(menu->items);
    delwin(menu->menu_wnd) CHECK_ERR;
}

void menu_go_down(Menu_t *menu)
{
    if (menu->text_list_idx + 1 < menu->text_list.count) {
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1)) CHECK_ERR;
        wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
        menu->screen_idx++;
        menu->text_list_idx++;
        if(menu->screen_idx >= menu->max_items_on_screen)
        {
            menu->top_of_text_list++;
            for (int i = 0, j = menu->top_of_text_list; i < menu->num_items_on_screen; i++, j++)
            {
                wclear(menu->items[i]) CHECK_ERR;
                wprintw(menu->items[i], list_find(menu->text_list, j)) CHECK_ERR;
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
  if (menu->text_list_idx - 1 >= 0) {
    wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1)) CHECK_ERR;
    wrefresh(menu->items[menu->screen_idx]) CHECK_ERR;
        menu->screen_idx--;
        menu->text_list_idx--;
        if(menu->screen_idx < 0)
        {
            menu->top_of_text_list--;
            for (int i = 0, j = menu->top_of_text_list; i < menu->num_items_on_screen; i++, j++)
            {
                wclear(menu->items[i]) CHECK_ERR;
                wprintw(menu->items[i], list_find(menu->text_list, j)) CHECK_ERR;
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
          case UA_DOWN: case KEY_DOWN:
              menu_go_down(menu); break;
          case UA_UP: case KEY_UP:
              menu_go_up(menu); break;
          case UA_ENTER:
              menu_act_on_item(menu); break;
          case UA_ADD_ITEM:
              menu_add_item(menu); break;
          case 'u':
              sig_winch(0); break;
          /*case UA_DEL_ITEM:
              menu_del_item(menu); break;*/
          case UA_QUIT: return;

          case ERR: default: break;
      }
  }
}

/*void menu_del_item(Menu_t *menu)
{
    sig_winch(0);
}*/
void menu_add_item(Menu_t *menu)
{
/*
 * clear last line of screen, i. e. border of box
 * use this line to hold user input
 * all user keys interpreted as is until enter key is pressed
 * after that:
 * redraw box border
 * add new entry to linked list
 * set text_list_idx equal to new item
 * redraw screen, screen_idx should highlight new item
 * how to redraw screen: for example use function menu_go_down
 * until text_list_idx reaches new item
 *
 * create input_wnd instead?
 * */
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

    //box(menu->menu_wnd, '|', '-') CHECK_ERR;
    wrefresh(menu->menu_wnd) CHECK_ERR;

    list_add(&menu->text_list, user_input);
    free(user_input);
    sig_winch(0);
}

void menu_act_on_item(Menu_t *menu)
{
  char* result_command = NULL;
  char* prefix = "tmux send-keys -t ! \"";
  char* command = list_find(menu->text_list, menu->text_list_idx);
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

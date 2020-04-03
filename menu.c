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
    const int terminal_height_change = size.ws_row - LINES;
    resizeterm(size.ws_row, size.ws_col) CHECK_ERR;
    //menu_destroy(&main_menu);
    //refresh() CHECK_ERR;

    const int terminal_height_change_abs = abs(terminal_height_change);
    const int menu_box_offset = BOX_OFFSET;
    const int menu_ncurses_y = 0;
    const int menu_ncurses_x = 0;
    const int menu_item_width = COLS - menu_box_offset;
    main_menu.max_items_on_screen = LINES - menu_box_offset;

    /*main_menu.menu_wnd = newwin(LINES, COLS, menu_ncurses_y, menu_ncurses_x);
    main_menu.menu_wnd CHECK_IS_NULL;
    main_menu.items = malloc(main_menu.max_items_on_screen * sizeof(WINDOW*));
    main_menu.items CHECK_IS_NULL;*/
    /*
     * store prev value
     * int temp = num_items_on_screen
     * calc how many items are need to be showed
     * minimum of available screen space and num of items
     * num_items_on_screen  = max_items_on_screen > list.count ? list.count : max_items_on_screen
     * realloc items array
     * compare (A)num_items_on_screen & (B)temp
     * int diff = A - B
     * if diff > 0 -> allocate new windows
     * else diff < 0 -> free unused windows
     *
     * */

    if(terminal_height_change > 0)//pane enlarged
    {
        int to_show = main_menu.text_list.count - 1 - main_menu.top_of_text_list + 1;
        if(main_menu.max_items_on_screen > to_show)
        {
            main_menu.top_of_text_list -= terminal_height_change_abs;
            if(main_menu.top_of_text_list < 0)
                main_menu.top_of_text_list = 0;
            else
                main_menu.screen_idx += terminal_height_change_abs;
        }
    }
    else if(terminal_height_change < 0)//pane shrank
    {
        if(main_menu.screen_idx >= main_menu.max_items_on_screen)
        {
            main_menu.screen_idx -= terminal_height_change_abs;
            main_menu.top_of_text_list += terminal_height_change_abs;
        }
    }

    for (int i = 0, j = main_menu.top_of_text_list; i < main_menu.max_items_on_screen && j < main_menu.text_list.count; i++, j++) {
        main_menu.items[i] = derwin(main_menu.menu_wnd, 1, menu_item_width,
                i + menu_box_offset / 2,
                0 + menu_box_offset / 2);
        main_menu.items[i] CHECK_IS_NULL;
        wprintw(main_menu.items[i], list_find(main_menu.text_list, j)) CHECK_ERR;
    }
    wbkgd(main_menu.menu_wnd, COLOR_PAIR(1)) CHECK_ERR;
    wbkgd(main_menu.items[main_menu.screen_idx], COLOR_PAIR(1) | A_REVERSE) CHECK_ERR;

    box(main_menu.menu_wnd, '|', '-') CHECK_ERR;
    wrefresh(main_menu.menu_wnd) CHECK_ERR;
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

  sigaction(SIGWINCH, &sa_handler, NULL) CHECK_IS_NEGATIVE_ONE;

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
    menu->max_items_on_screen = LINES - menu_box_offset;

    list_add(&menu->text_list,  "show firmware");
    list_add(&menu->text_list,  "reload system");
    list_add(&menu->text_list,  "show running config");
    list_add(&menu->text_list,  "show msdp vrf test peers");
    list_add(&menu->text_list,  "Exit");

    menu->num_items_on_screen = menu->max_items_on_screen > menu->text_list.count ? menu->text_list.count : menu->max_items_on_screen;

    menu->menu_wnd = newwin(LINES, COLS, menu_ncurses_y, menu_ncurses_x);
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

    box(menu->menu_wnd, '|', '-') CHECK_ERR;
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
            for (int i = 0, j = menu->top_of_text_list; i < menu->max_items_on_screen && j < menu->text_list.count; i++, j++) {
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
            for (int i = 0, j = menu->top_of_text_list; i < menu->max_items_on_screen && j < menu->text_list.count; i++, j++) {
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
          case UA_NEW_ITEM:
              menu_add_item(menu); break;
          case UA_QUIT: return;

          case ERR: default: break;
      }
  }
}

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
                LINES - 1,
                0 + BOX_OFFSET / 2);
    menu->input_wnd CHECK_IS_NULL;
    wbkgd(menu->input_wnd, COLOR_PAIR(3)) CHECK_ERR;
    werase(menu->input_wnd);

    echo() CHECK_ERR;
    wgetnstr(menu->input_wnd, user_input, menu_item_width) CHECK_ERR;
    noecho() CHECK_ERR;

    delwin(menu->input_wnd) CHECK_ERR;

    box(menu->menu_wnd, '|', '-') CHECK_ERR;
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

#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include "menu.h"

void sig_winch(int signo)
{

}

void check_terminal_size()
{
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);

  if (size.ws_col != TERMINAL_WIDTH || size.ws_row != TERMINAL_HEIGHT) {
    printf("Terminal has improper width/height!\n");
    printf("Recommended sizes are: width - %d  height - %d\n",
           TERMINAL_WIDTH, TERMINAL_HEIGHT);
    exit(EXIT_FAILURE);
  }
}

void ncurses_init()
{
  // check_terminal_size();

  initscr();
  signal(SIGWINCH, sig_winch);
  cbreak();
  curs_set(0);
  start_color();
  noecho();
  refresh();
  init_pair(1, COLOR_YELLOW, COLOR_BLUE);
  init_pair(2, COLOR_YELLOW, COLOR_CYAN);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
  keypad(stdscr, 1);
  nodelay(stdscr, 1);
  bkgd(COLOR_PAIR(2));
  refresh();
}

void menu_init(Menu_t *menu)
{
  const int menu_box_offset = 2;
  int menu_width = TERMINAL_WIDTH + menu_box_offset;
  int menu_height = NUM_MENU_ITEMS + menu_box_offset;
  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  menu->menu_wnd = newwin(menu_height, menu_width, menu_ncurses_y,
                          menu_ncurses_x);

  int menu_item_width = menu_width - menu_box_offset;

  for (int i = 0; i < NUM_MENU_ITEMS; i++) {
    menu->menu_items[i].item = derwin(menu->menu_wnd, 1, menu_item_width,
                                 i + menu_box_offset / 2,
                                 0 + menu_box_offset / 2);
  }

  wprintw(menu->menu_items[0].item, "show firmware");
  wprintw(menu->menu_items[1].item, "reload system");
  wprintw(menu->menu_items[2].item, "show running config");
  wprintw(menu->menu_items[3].item, "show msdp vrf test peers");
  wprintw(menu->menu_items[4].item, "Exit");

  strcpy(menu->menu_items[0].text, "show firmware");
  strcpy(menu->menu_items[1].text, "reload system");
  strcpy(menu->menu_items[2].text, "show running config");
  strcpy(menu->menu_items[3].text, "show msdp vrf test peers");
  strcpy(menu->menu_items[4].text, "Exit");

  wbkgd(menu->menu_wnd, COLOR_PAIR(1));
  menu->current_idx = 0;
  wbkgd(menu->menu_items[menu->current_idx].item, COLOR_PAIR(1) | A_BOLD);

  box(menu->menu_wnd, '|', '-');
  wrefresh(menu->menu_wnd);
}

void menu_destroy(Menu_t *menu)
{
  for (int i = 0; i < NUM_MENU_ITEMS; i++) {
    delwin(menu->menu_items[i].item);
  }

  delwin(menu->menu_wnd);
}

void menu_go_down(Menu_t *menu)
{
  if (menu->current_idx + 1 < NUM_MENU_ITEMS) {
    wbkgd(menu->menu_items[menu->current_idx].item, COLOR_PAIR(1));
    wrefresh(menu->menu_items[menu->current_idx].item);
    wbkgd(menu->menu_items[++menu->current_idx].item, COLOR_PAIR(1) | A_BOLD);
    wrefresh(menu->menu_items[menu->current_idx].item);
  }
}

void menu_go_up(Menu_t *menu)
{
  if (menu->current_idx - 1 >= 0) {
    wbkgd(menu->menu_items[menu->current_idx].item, COLOR_PAIR(1));
    wrefresh(menu->menu_items[menu->current_idx].item);
    wbkgd(menu->menu_items[--menu->current_idx].item, COLOR_PAIR(1) | A_BOLD);
    wrefresh(menu->menu_items[menu->current_idx].item);
  }
}

int menu_move(Menu_t *menu)
{
  while (1) 
  {
      int ch = getch();

      switch (ch) 
      {
          case 'q':
              return STATUS_EXIT;

          case KEY_DOWN:
              menu_go_down(menu);
              break;

          case KEY_UP:
              menu_go_up(menu);
              break;

          case '\n':
              return (menu_act_on_item(menu));

          case ERR:
          default:
              break;
      }
  }
}

int menu_act_on_item(Menu_t *menu)
{
  int status = -1;
  system("tmux send-keys -t ! \"TEST\" Enter");
  return status;
}

int menu_do()
{
  Menu_t main_menu;
  menu_init(&main_menu);
  int player_choice = menu_move(&main_menu);
  menu_destroy(&main_menu);

  return player_choice;
}


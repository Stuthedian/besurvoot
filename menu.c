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
  const int menu_box_offset = BOX_OFFSET;
  int menu_width = TERMINAL_WIDTH;
  int menu_height = TERMINAL_HEIGHT;
  int menu_ncurses_y = 0;
  int menu_ncurses_x = 0;

  menu->menu_wnd = newwin(menu_height, menu_width, menu_ncurses_y,
                          menu_ncurses_x);

  int menu_item_width = menu_width - menu_box_offset;

  strcpy(menu->text[0], "show firmware");
  strcpy(menu->text[1], "reload system");
  strcpy(menu->text[2], "show running config");
  strcpy(menu->text[3], "show msdp vrf test peers");
  strcpy(menu->text[4], "Exit");

  menu->page = 0;
  menu->screen_idx = 0;
  menu->current_idx = 0;
  menu->top_of_text_array = 0;
  for (int i = 0; i < MAX_ITEMS_ON_SCREEN; i++) {
      menu->items[i] = derwin(menu->menu_wnd, 1, menu_item_width,
              i + menu_box_offset / 2,
              0 + menu_box_offset / 2);
      if(i < NUM_MENU_ITEMS)
          wprintw(menu->items[i], menu->text[i]);
  }

  wbkgd(menu->menu_wnd, COLOR_PAIR(1));
  wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE);

  box(menu->menu_wnd, '|', '-');
  wrefresh(menu->menu_wnd);
}

void menu_destroy(Menu_t *menu)
{
  for (int i = 0; i < MAX_ITEMS_ON_SCREEN; i++) {
    delwin(menu->items[i]);
  }

  delwin(menu->menu_wnd);
}

void menu_go_down(Menu_t *menu)
{
    if (menu->current_idx + 1 < NUM_MENU_ITEMS) {
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1));
        wrefresh(menu->items[menu->screen_idx]);
        menu->screen_idx++;
        menu->current_idx++;
        if(menu->screen_idx >= MAX_ITEMS_ON_SCREEN)
        {
            menu->top_of_text_array++;
            for (int i = 0, j = menu->top_of_text_array; i < MAX_ITEMS_ON_SCREEN && i < NUM_MENU_ITEMS; i++, j++) {
                wclear(menu->items[i]);
                wprintw(menu->items[i], menu->text[j]);
                wrefresh(menu->items[i]);
            }
            menu->screen_idx--;
        }
        wbkgd(menu->items[menu->screen_idx], COLOR_PAIR(1) | A_REVERSE);
        wrefresh(menu->items[menu->screen_idx]);
    }
}

void menu_go_up(Menu_t *menu)
{
  /*if (menu->current_idx - 1 >= 0) {
    wbkgd(menu->menu_items[menu->current_idx].item, COLOR_PAIR(1));
    wrefresh(menu->menu_items[menu->current_idx].item);
    wbkgd(menu->menu_items[--menu->current_idx].item, COLOR_PAIR(1) | A_BOLD);
    wrefresh(menu->menu_items[menu->current_idx].item);
  }*/
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
              menu_act_on_item(menu);
              break;

          case ERR:
          default:
              break;
      }
  }
}

void menu_act_on_item(Menu_t *menu)
{
  char* result_command = NULL;
  char* prefix = "tmux send-keys -t ! \"";
  char* command = menu->text[menu->current_idx];
  char* suffix = "\" Enter";
  result_command = malloc(strlen(prefix) + strlen(command) + strlen(suffix) + 1);
  result_command[0] = '\0';
  strcat(result_command, prefix);
  strcat(result_command, command);
  strcat(result_command, suffix);
  system(result_command);
  free(result_command);
}

int menu_do()
{
  Menu_t main_menu;
  menu_init(&main_menu);
  int player_choice = menu_move(&main_menu);
  menu_destroy(&main_menu);

  return player_choice;
}


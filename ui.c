#include "database.h"
#include <ncurses.h>
#include <string.h>
WINDOW *create_window_box(int nlines, int ncols, int begin_y, int begin_x) {

  WINDOW *window = newwin(nlines, ncols, begin_y, begin_x);

  box(window, 0, 0);
  return window;
}

int main(void) {
  initscr();

  int y_max, x_max, head_y_max, head_x_max;

  const char *title = "TODOERMINAL";
  const char *options = "[A]dd [D]elete [E]dit [C]ompleted [Q]uit";
  const char *todos = "TODOs list";

  WINDOW *header, *left_pane, *right_pane;
  getmaxyx(stdscr, y_max, x_max);

  header = create_window_box(4, x_max - 2, 1, 1);
  getmaxyx(header, head_y_max, head_x_max);

  mvwprintw(header, 1, head_x_max / 2 - strlen(title) / 2, title);
  mvwprintw(header, 3, head_x_max / 2 - strlen(options) / 2, options);

  left_pane = create_window_box(y_max - head_y_max - 1, x_max / 2 - 2,
                                head_y_max + 1, 1);

  mvwprintw(left_pane, 0, 1, todos);

  right_pane = create_window_box(y_max - head_y_max - 1, head_x_max / 2 - 1,
                                 head_y_max + 1, x_max / 2 + 1);

  refresh();
  wrefresh(header);
  wrefresh(left_pane);
  wrefresh(right_pane);

  getch();

  delwin(header);
  delwin(left_pane);
  delwin(right_pane);

  endwin();
  return 0;
}

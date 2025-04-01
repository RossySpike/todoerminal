#include "database.h"
#include <assert.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *title = "TODOERMINAL";
const char *options = "[A]dd [D]elete [E]dit [C]ompleted [Q]uit";
const char *todos = "TODOs list";

WINDOW *create_window_box(int nlines, int ncols, int begin_y, int begin_x);

void clear_previous(WINDOW *pane);

void print_characters(WINDOW *pane, int max_x, int *current_x, int *current_y,
                      const char *str, int strlen);

void show_hovered_todo(WINDOW *pane, todo_t *t);

void show_editing_actions(WINDOW *pane, todo_t *t);

void copy_todos(list_t *src, list_t *dest);

void copy_todos_completed(list_t *src, list_t *dest);

void show_error_msg(WINDOW *dest, char *msg);

void replace_char(char *str, char find, char replace);

void get_n_characters(WINDOW *input_box, char *box_title, char *input, int n,
                      char *err_msg, int custom_check(char *, void *),
                      void *argv);

int is_yY_or_Nn(char *str, void *useless);

void get_name(WINDOW *input_box, char *name);

void get_description(WINDOW *input_box, char *description);

void get_completed(WINDOW *input_box, char *completed);

void get_todo_data(WINDOW *input_box, char *name, char *description,
                   char *completed);

void draw_header(WINDOW *header);

WINDOW *create_header(void);

WINDOW *create_left_pane(WINDOW *header);

WINDOW *create_right_pane(WINDOW *header);

void refresh_all(WINDOW *arr[], size_t len);

void init_panes(WINDOW **header, WINDOW **left_pane, WINDOW **right_pane);

void draw_left_pane(WINDOW *left_pane, list_t list_current, int selected_todo);

void get_action(WINDOW *input_pane, WINDOW *edit_pane, WINDOW *header,
                todo_t *todo, int head_x_max, int head_y_max,
                list_t list_current, int selected_todo);

WINDOW *create_window_box(int nlines, int ncols, int begin_y, int begin_x) {
  WINDOW *window = newwin(nlines, ncols, begin_y, begin_x);

  box(window, 0, 0);
  return window;
}
void clear_previous(WINDOW *pane) {
  werase(pane);
  box(pane, 0, 0);
}
void print_characters(WINDOW *pane, int max_x, int *current_x, int *current_y,
                      const char *str, int strlen) {
  for (int i = 0; i < strlen; i++) {
    if (*current_x >= max_x - 1) {
      *current_x = 1;
      (*current_y)++;
    }
    mvwprintw(pane, *current_y, *current_x, "%c", str[i]);
    (*current_x)++;
  }
  *current_x = 1;
  (*current_y)++;
}
void show_hovered_todo(WINDOW *pane, todo_t *t) {
  clear_previous(pane);
  int max_y, max_x;
  getmaxyx(pane, max_y, max_x);
  int current_y = 1;
  int current_x = 1;

  char title_tag[strlen("Title: ") + strlen(t->Title) + 1];
  snprintf(title_tag, sizeof(title_tag), "%s%s", "Title: ", t->Title);
  print_characters(pane, max_x, &current_x, &current_y, title_tag,
                   strlen(title_tag));

  char description_tag[strlen("Description: ") + strlen(t->Description) + 1];
  snprintf(description_tag, sizeof(description_tag), "%s%s",
           "Description: ", t->Description);
  print_characters(pane, max_x, &current_x, &current_y, description_tag,
                   strlen(description_tag));

  char completed_tag[strlen("Completed: ") + 2];
  t->Completed ? snprintf(completed_tag, sizeof(completed_tag), "%s%s",
                          "Completed: ", "Y")
               : snprintf(completed_tag, sizeof(completed_tag), "%s%s",
                          "Completed: ", "N");
  print_characters(pane, max_x, &current_x, &current_y, completed_tag,
                   strlen(completed_tag));

  char date_tag[strlen("Date: ") + strlen(t->Date) + 1];
  snprintf(date_tag, sizeof(date_tag), "%s%s", "Date: ", t->Date);
  print_characters(pane, max_x, &current_x, &current_y, date_tag,
                   strlen(date_tag));
}
void show_editing_actions(WINDOW *pane, todo_t *t) {

  clear_previous(pane);
  int max_y, max_x;
  getmaxyx(pane, max_y, max_x);
  int current_y = 1;
  int current_x = 1;

  char title_tag[strlen("Titl[E]: ") + strlen(t->Title) + 1];
  snprintf(title_tag, sizeof(title_tag), "%s%s", "Titl[E]: ", t->Title);
  print_characters(pane, max_x, &current_x, &current_y, title_tag,
                   strlen(title_tag));

  char description_tag[strlen("Descriptio[N]: ") + strlen(t->Description) + 1];
  snprintf(description_tag, sizeof(description_tag), "%s%s",
           "Descriptio[N]: ", t->Description);
  print_characters(pane, max_x, &current_x, &current_y, description_tag,
                   strlen(description_tag));

  char completed_tag[strlen("Complete[D]: ") + 2];
  t->Completed ? snprintf(completed_tag, sizeof(completed_tag), "%s%s",
                          "Complete[D]: ", "Y")
               : snprintf(completed_tag, sizeof(completed_tag), "%s%s",
                          "Complete[D]: ", "N");
  print_characters(pane, max_x, &current_x, &current_y, completed_tag,
                   strlen(completed_tag));

  char date_tag[strlen("Date: ") + strlen(t->Date) + 1];
  snprintf(date_tag, sizeof(date_tag), "%s%s", "Date: ", t->Date);
  print_characters(pane, max_x, &current_x, &current_y, date_tag,
                   strlen(date_tag));
  const char *quit_tag = "[Q]uit.";
  print_characters(pane, max_x, &current_x, &current_y, quit_tag,
                   strlen(quit_tag));
}
void copy_todos(list_t *src, list_t *dest) {
  if (!src || !dest)
    return; // Verificación de punteros nulos

  int min_len = src->len < dest->len ? src->len : dest->len;
  for (int i = 0; i < min_len; i++) {
    dest->array[i] = src->array[i];
  }
  if (src->len < dest->len) {
    for (int i = src->len; i < dest->len; i++) {
      list_remove(dest, src->len, NULL);
    }
  } else {
    for (int i = dest->len; i < src->len; i++) {
      list_append(dest, src->array[i]);
    }
  }
}
void copy_todos_completed(list_t *src, list_t *dest) {
  if (!src || !dest)
    return; // Verificación de punteros nulos

  int j = 0;
  for (int i = 0; i < src->len; i++) {
    todo_t *todo = (todo_t *)src->array[i];
    if (!todo->Completed)
      continue;

    if (j < dest->len) {
      dest->array[j] = todo;
    } else {
      list_append(dest, todo);
    }
    j++;
  }
  while (j < dest->len) {
    list_remove(dest, j, NULL);
  }
}
void show_error_msg(WINDOW *dest, char *msg) {
  wclear(dest);
  box(dest, 0, 0);
  mvwprintw(dest, 1, 1, "%s", msg);
  wrefresh(dest);
}
void replace_char(char *str, char find, char replace) {
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == find)
      str[i] = replace;
  }
}
void get_n_characters(WINDOW *input_box, char *box_title, char *input, int n,
                      char *err_msg, int custom_check(char *, void *),
                      void *argv) {
  assert(n > 0);
  int err_value;
  do {
    mvwprintw(input_box, 0, 1, "%s", box_title);

    err_value = n > 2 ? mvwgetnstr(input_box, 1, 1, input, n - 2)
                      : mvwgetnstr(input_box, 1, 1, input, 1);
    int check = custom_check ? custom_check(input, argv) : 0;
    if (strlen(input) > n - 1 || strlen(input) < 1 || check) {
      err_value = ERR;
      show_error_msg(input_box, err_msg);
      sleep(2);
    }
    wclear(input_box);

    box(input_box, 0, 0);
  } while (err_value == ERR);
  replace_char(input, '\n', '\0');
}
int is_yY_or_Nn(char *str, void *useless) {
  switch (str[0]) {
  case 'n':
  case 'N':
  case 'y':
  case 'Y':
    return 0;
  default:
    return 1;
  }
}
void get_name(WINDOW *input_box, char *name) {
  get_n_characters(input_box, "Name: ", name, 128,
                   "Title must be between 1 and 126 characters.", NULL, NULL);
}
void get_description(WINDOW *input_box, char *description) {
  get_n_characters(input_box, "Description: ", description, 256,
                   "Description must be between 1 and 254 characters.", NULL,
                   NULL);
}
void get_completed(WINDOW *input_box, char *completed) {
  get_n_characters(input_box, "Completed (y-Y-n-N): ", completed, 2,
                   "Description must be 1 character (y-Y-n-N).", *is_yY_or_Nn,
                   NULL);
}
void get_todo_data(WINDOW *input_box, char *name, char *description,
                   char *completed) {
  get_name(input_box, name);
  get_description(input_box, description);
  get_completed(input_box, completed);
}
void draw_header(WINDOW *header) {
  int head_x_max = getmaxx(header);

  box(header, 0, 0);
  mvwprintw(header, 1, head_x_max / 2 - strlen(title) / 2, "%s", title);
  mvwprintw(header, 3, head_x_max / 2 - strlen(options) / 2, "%s", options);
  wrefresh(header);
}
WINDOW *create_header(void) {
  int x_max = getmaxx(stdscr);
  WINDOW *header = create_window_box(4, x_max - 2, 1, 1);
  draw_header(header);
  return header;
}
WINDOW *create_left_pane(WINDOW *header) {
  int y_max, x_max, head_y_max, head_x_max;
  getmaxyx(stdscr, y_max, x_max);
  getmaxyx(header, head_y_max, head_x_max);
  return create_window_box(y_max - head_y_max - 1, x_max / 2 - 2,
                           head_y_max + 1, 1);
}
WINDOW *create_right_pane(WINDOW *header) {
  int y_max, x_max, head_y_max, head_x_max;
  getmaxyx(stdscr, y_max, x_max);
  getmaxyx(header, head_y_max, head_x_max);
  return create_window_box(y_max - head_y_max - 1, head_x_max / 2 - 1,
                           head_y_max + 1, x_max / 2 + 1);
}
void refresh_all(WINDOW *arr[], size_t len) {
  for (int i = 0; i < len; i++) {
    wrefresh(arr[i]);
  }
}
void init_panes(WINDOW **header, WINDOW **left_pane, WINDOW **right_pane) {
  *header = create_header();
  if (!*header) {
    endwin();
    printf("[-] Error creating panes.\n");
    exit(1);
  }

  *left_pane = create_left_pane(*header);
  mvwprintw(*left_pane, 0, 1, "%s", todos);
  keypad(*left_pane, true);

  if (!*left_pane) {
    endwin();
    printf("[-] Error creating panes.\n");
    exit(1);
  }
  *right_pane = create_right_pane(*header);
  if (!*right_pane) {
    endwin();
    printf("[-] Error creating panes.\n");
    exit(1);
  }
  noecho();
  cbreak();
  curs_set(0);
  WINDOW *arr[] = {*header, *left_pane, *right_pane};
  refresh_all(arr, 3);
}
void draw_left_pane(WINDOW *left_pane, list_t list_current, int selected_todo) {
  clear_previous(left_pane);
  mvwprintw(left_pane, 0, 1, "%s", todos);
  int current_y = 1;
  for (int i = 0; i < list_current.len; i++) {
    todo_t *todo = (todo_t *)list_current.array[i];
    if (i == selected_todo)
      wattron(left_pane, A_REVERSE);
    mvwprintw(left_pane, current_y, 1, "%s", todo->Title);
    wattroff(left_pane, A_REVERSE);
    current_y++;
  }
}
void get_action(WINDOW *input_pane, WINDOW *edit_pane, WINDOW *header,
                todo_t *todo, int head_x_max, int head_y_max,
                list_t list_current, int selected_todo) {
  bool exit = false;
  int input_key;
  WINDOW *input_box = create_window_box(4, head_x_max, head_y_max + 1, 1);
  while (!exit) {
    show_editing_actions(edit_pane, todo);
    wrefresh(edit_pane);
    WINDOW *input_box = create_window_box(4, head_x_max, head_y_max + 1, 1);

    input_key = wgetch(input_pane);
    switch (input_key) {
    case 'e':
    case 'E': // Title
      clear();
      echo();
      nocbreak();
      refresh();
      char name[128];
      get_name(input_box, name);
      strncpy(todo->Title, name, 128);
      break;

    case 'n': // description
    case 'N':
      clear();
      echo();
      nocbreak();
      refresh();
      char description[256];
      get_description(input_box, description);
      strncpy(todo->Description, description, 256);
      break;
    case 'd': // completed
    case 'D':
      clear();
      echo();
      nocbreak();
      refresh();
      char completed[2];
      get_completed(input_box, completed);
      todo->Completed = completed[0] == 'y' || completed[0] == 'Y';
      break;
    case 'q': // quit
    case 'Q':
      exit = !exit;
      break;
    }
    clear();
    refresh();

    // Header
    draw_header(header);

    noecho();
    cbreak();

    draw_left_pane(input_pane, list_current, selected_todo);
    if (selected_todo < list_current.len)
      show_hovered_todo(edit_pane, list_current.array[selected_todo]);
    refresh();
  }
}

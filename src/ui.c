#include "database.h"
#include <assert.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

int main(void) {
  char name[128], description[256], completed[2];
  int y_max, x_max, head_y_max, head_x_max, selected_todo;
  bool show_completed, exit;
  show_completed = exit = false;
  sqlite3 *db;
  list_t list_todos, list_current;
  int input_key;
  WINDOW *input_box;
  list_new(&list_todos);
  list_new(&list_current);
  open_conection("db/Todo.db", &db);
  get_todos(db, GET_ALL, &list_todos);
  if (list_todos.len)
    copy_todos(&list_todos, &list_current);
  selected_todo = 0;

  const char *title = "TODOERMINAL";
  const char *options = "[A]dd [D]elete [E]dit [C]ompleted [Q]uit";
  const char *todos = "TODOs list";

  initscr();

  WINDOW *header, *left_pane, *right_pane;
  getmaxyx(stdscr, y_max, x_max);

  // pane init
  header = create_window_box(4, x_max - 2, 1, 1);
  getmaxyx(header, head_y_max, head_x_max);

  mvwprintw(header, 1, head_x_max / 2 - strlen(title) / 2, "%s", title);
  mvwprintw(header, 3, head_x_max / 2 - strlen(options) / 2, "%s", options);

  left_pane = create_window_box(y_max - head_y_max - 1, x_max / 2 - 2,
                                head_y_max + 1, 1);

  mvwprintw(left_pane, 0, 1, "%s", todos);

  right_pane = create_window_box(y_max - head_y_max - 1, head_x_max / 2 - 1,
                                 head_y_max + 1, x_max / 2 + 1);

  keypad(left_pane, true);

  keypad(left_pane, true);
  if (nodelay(left_pane, true) == ERR) {
    printf("[-] There\'s a problem.\n");
  }
  noecho();
  cbreak();
  curs_set(0);
  // End pane init

  refresh();
  wrefresh(header);
  wrefresh(left_pane);
  wrefresh(right_pane);

  while (!exit) {
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

    wrefresh(right_pane);
    input_key = wgetch(left_pane);
    if (list_todos.len)
      show_hovered_todo(right_pane, list_current.array[selected_todo]);

    switch (input_key) {
    case KEY_UP:
      selected_todo--;
      if (selected_todo == -1)
        selected_todo = 0;
      break;
    case KEY_DOWN:
      selected_todo++;
      if (selected_todo == list_current.len)
        selected_todo = list_current.len - 1;
      break;
    case 'a':
    case 'A':
      clear();
      echo();
      nocbreak();
      input_box = create_window_box(4, head_x_max, head_y_max + 1, 1);
      refresh();
      get_todo_data(input_box, name, description, completed);

      // Adding todo...
      // TODO: Change this to add the todo to the db then update the list.
      add_todo(db, name, description,
               completed[0] == 'y' || completed[0] == 'Y');

      list_free(&list_todos, free_todo);
      list_free(&list_current, NULL);

      list_new(&list_todos);
      list_new(&list_current);
      get_todos(db, GET_ALL, &list_todos);
      if (show_completed)
        copy_todos_completed(&list_todos, &list_current);
      else
        copy_todos(&list_todos, &list_current);
      // ****

      // Destroy input box and clear screen.
      delwin(input_box);
      clear();
      refresh();

      // Header
      box(header, 0, 0);
      mvwprintw(header, 1, head_x_max / 2 - strlen(title) / 2, "%s", title);
      mvwprintw(header, 3, head_x_max / 2 - strlen(options) / 2, "%s", options);
      wrefresh(header);

      noecho();
      cbreak();
      break;
    case 'd':
    case 'D':
      if (!list_current.len)
        break;
      delete_todo(db, ((todo_t *)list_current.array[selected_todo])->Id);
      list_remove(&list_current, selected_todo, NULL);
      list_remove(&list_todos, selected_todo, free_todo);
      break;
    case 'e':
    case 'E':
      clear();
      echo();
      nocbreak();
      input_box = create_window_box(4, head_x_max, head_y_max + 1, 1);
      refresh();
      get_todo_data(input_box, name, description, completed);

      // Adding todo...
      // TODO: Change edited todo database and list

      todo_t *todo = (todo_t *)list_current.array[selected_todo];
      strncpy(todo->Title, name, 128);
      strncpy(todo->Description, description, 256);
      todo->Completed = completed[0] == 'y' || completed[0] == 'Y';

      // Destroy input box and clear screen.
      delwin(input_box);
      clear();
      refresh();

      // Header
      box(header, 0, 0);
      mvwprintw(header, 1, head_x_max / 2 - strlen(title) / 2, "%s", title);
      mvwprintw(header, 3, head_x_max / 2 - strlen(options) / 2, "%s", options);
      wrefresh(header);

      noecho();
      cbreak();
      break;

    case 'c':
    case 'C':
      show_completed = !show_completed;
      if (show_completed) {
        copy_todos_completed(&list_todos, &list_current);
        selected_todo = 0;
      } else {
        copy_todos(&list_todos, &list_current);
      }
      break;
    case 'q':
    case 'Q':
      exit = true;
      break;
    default:
      break;
    }
  }

  list_free(&list_todos, free_todo);
  list_free(&list_current, NULL);
  sqlite3_close(db);
  delwin(header);
  delwin(left_pane);
  delwin(right_pane);

  endwin();
  return 0;
}

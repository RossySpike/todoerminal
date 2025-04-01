#include "ui.c"

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

  initscr();

  getmaxyx(stdscr, y_max, x_max);
  WINDOW *header, *left_pane, *right_pane;
  header = left_pane = right_pane = NULL;

  // pane init
  init_panes(&header, &left_pane, &right_pane);

  getmaxyx(header, head_y_max, head_x_max);

  while (!exit) {
    draw_left_pane(left_pane, list_current, selected_todo);
    if (selected_todo < list_current.len)
      show_hovered_todo(right_pane, list_current.array[selected_todo]);

    wrefresh(right_pane);

    input_key = wgetch(left_pane);
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
      draw_header(header);

      noecho();
      cbreak();
      break;
    case 'd':
    case 'D':
      if (!list_current.len)
        break;
      clear_previous(right_pane);

      delete_todo(db, ((todo_t *)list_current.array[selected_todo])->Id);
      list_remove(&list_current, selected_todo, NULL);
      list_remove(&list_todos, selected_todo, free_todo);
      break;
    case 'e':
    case 'E':
      get_action(left_pane, right_pane, header,
                 list_current.array[selected_todo], head_x_max, head_y_max,
                 list_current, selected_todo);
      update_todo(db, list_current.array[selected_todo]);

      list_free(&list_todos, free_todo);
      list_free(&list_current, NULL);

      list_new(&list_todos);
      list_new(&list_current);
      get_todos(db, GET_ALL, &list_todos);

      if (show_completed)
        copy_todos_completed(&list_todos, &list_current);
      else
        copy_todos(&list_todos, &list_current);
      // // TODO: Change edited todo database and list
      break;

    case 'c':
    case 'C':
      show_completed = !show_completed;
      if (show_completed) {
        copy_todos_completed(&list_todos, &list_current);
        selected_todo = list_current.len - 1;
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

#include "list.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct TODO {
  int Id;
  char Title[128];
  char Description[256];
  bool Completed;
  char *Date;
} todo_t;

const char *GET_ALL = "SELECT * FROM Todos ORDER BY id DESC;";
const char *GET_UNCOMPLETE = "SELECT * FROM Todos ORDER BY Completed DESC;";
const char *GET_COMPLETE = "SELECT * FROM Todos ORDER BY Completed ASC;";
const char *GET_RECENT = "SELECT * FROM Todos ORDER BY Date ASC;";
const char *GET_LATEST = "SELECT * FROM Todos ORDER BY Date DESC;";

int handle_error(int errCode, sqlite3 **db) {
  if (errCode == SQLITE_OK) {
    return SQLITE_OK;
  }
  perror(sqlite3_errmsg(*db));
  return errCode;
}

int open_conection(const char *path, sqlite3 **db) {
  int errCode = sqlite3_open(path, db);
  return handle_error(errCode, db);
}

int callback(void *param, int argc, char **argv, char **column) {
  if (argc != 5) {
    return 1;
  }

  todo_t *current = malloc(sizeof(todo_t));
  if (!current) {
    perror("Failed to allocate memory");
    return 1;
  }

  current->Id = atoi(argv[0]);
  strncpy(current->Title, argv[1], 128);
  strncpy(current->Description, argv[2], 256);
  current->Completed = atoi(argv[3]);
  current->Date = strdup(argv[4]);

  if (!current->Date) {
    perror("Failed to allocate memory for strings");
    free(current->Date);
    free(current);
    return 1;
  }

  list_append((list_t *)param, (void *)current);
  return 0;
}

void free_todo(void *element) {
  todo_t *todo = (todo_t *)element;
  free(todo->Date);
  free(todo);
}

void check_err(char *err) {
  if (!err)
    return;
  perror(err);
  sqlite3_free(err);
}

int get_todos(sqlite3 *db, const char *stmt, list_t *l) {
  char *err = NULL;
  int errCode = sqlite3_exec(db, stmt, callback, (void *)l, &err);
  check_err(err);

  return errCode;
}

int delete_todo(sqlite3 *db, int Id) {
  char stmt[256];
  char *err = NULL;
  int errCode = sprintf(stmt, "DELETE FROM Todos WHERE Id=%d;", Id);
  sqlite3_exec(db, stmt, NULL, NULL, &err);
  check_err(err);
  return errCode;
}
int add_todo(sqlite3 *db, char Title[128], char Description[256],
             bool Completed) {
  char stmt[9 + 128 + 256 + 20 + 86]; // Hopes this is enough
  char *err = NULL;
  int errCode =
      sprintf(stmt,
              "INSERT INTO Todos (Title, Description, Completed) VALUES "
              "('%s', '%s', %d);",
              Title, Description, Completed);
  sqlite3_exec(db, stmt, NULL, NULL, &err);
  check_err(err);
  return errCode;
}
int update_todo(sqlite3 *db, todo_t *todo) {
  char stmt[9 + 128 + 256 + 86]; // Hopesopes this is enough
  char *err = NULL;
  int errCode =
      sprintf(stmt,
              "UPDATE Todos SET Title='%s', Description='%s', Completed=%d "
              "WHERE Id=%d;",
              todo->Title, todo->Description, todo->Completed, todo->Id);
  sqlite3_exec(db, stmt, NULL, NULL, &err);
  check_err(err);
  return errCode;
}

#ifndef DATABASE_H
#define DATABASE_H
#include "database.c"

typedef struct TODO todo_t;

int handle_error(int errCode, sqlite3 **db);
int open_conection(const char *path, sqlite3 **db);
int callback(void *param, int argc, char **argv, char **column);
void free_todo(void *element);
void check_err(char *err);
int get_todos(sqlite3 *db, const char *stmt, list_t *l);
int delete_todo(sqlite3 *db, int Id);
int add_todo(sqlite3 *db, char Title[128], char Description[256],
             bool Completed);
#endif // DATABASE_H

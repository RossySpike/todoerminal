#ifndef LIST_H
#define LIST_H
#include "list.c"
typedef struct DYNAMIC_LIST_STRUCT list_t;

/* array should be freed */
void list_new(list_t *new_list);

void list_grow(list_t *l);

/* Puts `element` at the end of array */
void list_append(list_t *l, void *element);

/* Puts `element` at the specified `index`. Returns -1 if theres any problem */
int list_insert(list_t *l, void *element, int index);

/* Frees element and sets it to NULL */
void list_default_callback(void *element);

/* Removes element at the `index` value. Returns -1 if theres any problem, if
 * necesary you can use the callback function to free the element */
int list_remove(list_t *l, int index, void (*callback)(void *));

/* Removes last element. Returns -1 if theres any problem, if necesary you can
 * use the callback function to free the element */
int list_pop(list_t *l, void (*callback)(void *));

/* you should free all element of the array if necesary you can use the callback
 * function in order to do so... the array itself is freed by the function(also
 * it set them NULL)
 */
void list_free(list_t *l, void (*callback)(void *));

#endif // LIST_H

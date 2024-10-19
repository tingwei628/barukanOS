#ifndef LIST_H
#define LIST_H

#include "stdbool.h"
#include "stdint.h"

typedef struct List {
	struct List *next;
} List;

typedef struct HeadList{
	List *next;
	List *tail;
} HeadList;

void append_list_tail(HeadList *list, List *item);
List* remove_list_head(HeadList *list);
bool is_list_empty(HeadList *list);

#endif
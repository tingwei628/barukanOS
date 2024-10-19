#include "list.h"
#include "stddef.h"

// TODO: use pointer to pointer to rewrite it !!

// *list is dummy pointer of HeadList
// list->next is head of HeadList
void append_list_tail(HeadList *list, List *item)
{
    item->next = NULL;
    // first item added to empty list
    if (is_list_empty(list))
    {
        // set head of list
        list->next = item;
    }
    else
    {
        // old tail next = item
        list->tail->next = item;
    }

    // set new tail of list 
    list->tail = item;
}


// *list is dummy pointer of HeadList
// list->next is head of HeadList
List* remove_list_head(HeadList *list)
{
    List *item;

    if (is_list_empty(list))
    {
        return NULL;
    }

    item = list->next;
    list->next = item->next;

    // after removing the only item    
    if (is_list_empty(list))
    {
        // set tail of HeadList
        list->tail = NULL;
    }
    
    return item;
}

// *list is dummy pointer of HeadList
// list->next is head of HeadList
bool is_list_empty(HeadList *list)
{
    return (list->next == NULL);
}
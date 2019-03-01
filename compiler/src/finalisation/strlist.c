#include "count_map.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "strlist.h"

#include <stdlib.h>

strlist_node* strlistMake(char* content) {
    DBUG_ASSERT((content != NULL), "strlistMake called with NULL content string pointer");
    strlist_node* new_node = MEMmalloc(sizeof(strlist_node));

    new_node->content = content;
    new_node->next = NULL;

    return new_node;
}

void strlistFree(strlist_node* head) {
    DBUG_ASSERT((head != NULL), "strlistFree called with NULL head node");
    strlist_node* current = head;

    while(current != NULL) {
        strlist_node* next = current->next;

        MEMfree(current);

        current = next;
    }
}

void  strlistAdd(strlist_node* head, char* content) {
    DBUG_ASSERT((head != NULL), "strlistAdd called with NULL head node");
    DBUG_ASSERT((content != NULL), "strlistAdd called with NULL content string pointer");

    strlist_node* tail = head;

    while(tail->next != NULL) {
        tail = tail->next;
    }

    strlist_node* new_tail = strlistMake(content);

    tail->next = new_tail;
}

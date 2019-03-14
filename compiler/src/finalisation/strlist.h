#ifndef __STRING_LIST_H__
#define __STRING_LIST_H__


struct __strlist_node {
    struct __strlist_node* next;
    void* content;
};

typedef struct __strlist_node strlist_node;


strlist_node* strlistMake(void* content);
void strlistFree(strlist_node* head);
void  strlistAdd(strlist_node* head, void* content);

#endif

#ifndef __STRING_LIST_H__
#define __STRING_LIST_H__


struct __strlist_node {
    struct __strlist_node* next;
    char* content;
};

typedef struct __strlist_node strlist_node;


strlist_node* strlistMake(char* content);
void strlistFree(strlist_node* head);
void  strlistAdd(strlist_node* head, char* content);

#endif

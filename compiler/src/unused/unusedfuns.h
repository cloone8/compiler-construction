#ifndef __UNUSED_FUNS_H__
#define __UNUSED_FUNS_H__
#include "types.h"

extern node *SUFfunction(node *arg_node, info *arg_info);
extern node *SUFfuncall(node* arg_node, info* arg_info);
extern node *SUFdeclaration(node *arg_node, info *arg_info);
extern node *SUFlocalfunction(node* arg_node, info* arg_info);

extern node *SUFdo(node * syntaxtree);

#endif
